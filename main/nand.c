// This file contains the process routines for the NAND chip control.
//
// modified by Yeonwoo Sung on 2019.01.20


#include <pxa255.h>
#include <serial.h>
#include <time.h>
#include <gpio.h>
#include <stdio.h>
#include <string.h>

#include <mem_map.h>
#include <config.h>

#define NAND_VAR 1
#include <nand.h>


extern TConfig Cfg; // config.c 에 정의

TNandInfo Nand1;
TNandPartition Nand1Partition[NAND_MAX_PATITION] =
    {
        // NAME                  START      	        SIZE
        {"Kernel ", 0, 64},   // 1-MByte Size:0x100000
        {"Ramdisk", 64, 192}, // 3-MByte Size:0x300000
        {"Generel", 256, NAND_PARTITON_REST_SIZE},
        {"", NAND_PARTITON_END, 0},
};

#define USED_BOOT_BLOCK_COUNT (Nand1Partition[0].BlockCount + Nand1Partition[1].BlockCount)

#define USE_PARTITION_MAX 3

#define BLK_CNT_1M 64



static void NAND_Par_Fixup(void) {
    Nand1Partition[0].BaseBlock = 0;
    Nand1Partition[0].BlockCount = Cfg.NandPar[0] * BLK_CNT_1M;

    Nand1Partition[1].BaseBlock = Cfg.NandPar[0] * BLK_CNT_1M;
    Nand1Partition[1].BlockCount = Cfg.NandPar[1] * BLK_CNT_1M;

    Nand1Partition[2].BaseBlock = (Cfg.NandPar[0] + Cfg.NandPar[1]) * BLK_CNT_1M;
    Nand1Partition[2].BlockCount = Cfg.NandPar[2] * BLK_CNT_1M;
}

void nand_dumy(void) {
    return;
}
void NAND_AddressSetupWait(void) {
    int wait = 250;

    while (wait--)
        nand_dumy();
}

BOOL NAND_CheckBadBlock(TNandInfo *NandDev, int BlockNumber) {
    unsigned long Dummy;
    unsigned long BlockAddr;
    unsigned char State;

    BlockAddr = (BlockNumber << 5);

    Dummy = NAND_ACCESS_START;
    NAND_CMD = NAND_CMD_READ_C;                   // C Area Read Command
    NAND_ADDR = 0x05;                             // Bad Check Offset      Col
    NAND_ADDR = (BlockAddr & 0xFF);               // Block Page Address L  Row0
    NAND_ADDR = ((BlockAddr >> 8) & 0xFF);        // Block Page Address M  Row1
    if (NandDev->TotalSize >= 64 * 1024 * 1024) { // Block Page Address H  Row2
        NAND_ADDR = ((BlockAddr >> 16) & 0xFF);
    }
    NAND_AddressSetupWait();

    State = NAND_DATA & 0xFF;
    Dummy = NAND_ACCESS_END;

    NandDev->BadBlock[BlockNumber] = 'O';
    if (State != 0xFF) {
        NandDev->BadBlock[BlockNumber] = 'X';
        return FALSE;
    }

    return TRUE;
}


int NAND_ScanBadBlock(TNandInfo *NandDev) {
    int ScanBlock, IndexBlock;
    int ReplaceBlock, VirIndex;

    ScanBlock = USED_BOOT_BLOCK_COUNT; //NandDev->TotalSize/NandDev->EraseSize;
    NandDev->BadBlockCount = 0;

    for (IndexBlock = 0; IndexBlock < ScanBlock; IndexBlock++) {
        if (NAND_CheckBadBlock(NandDev, IndexBlock) == FALSE) {
            NandDev->BadBlockCount++;
        }
    }

    // sort all virtual blocks
    ReplaceBlock = 0;
    VirIndex = 0;
    for (IndexBlock = 0; IndexBlock < ScanBlock; IndexBlock++) {
        if ('X' == NandDev->BadBlock[IndexBlock]) {
            ReplaceBlock++;
            continue;
        }

        if (ReplaceBlock < ScanBlock) {
            NandDev->VirBlock[VirIndex++] = ReplaceBlock++;
        } else {
            NandDev->VirBlock[VirIndex++] = ScanBlock - 1;
        }
    }

    return NandDev->BadBlockCount;
}

BOOL NAND_EraseBlock(TNandInfo *NandDev, int BlockNumber) {
    unsigned long Dummy;
    unsigned long BlockAddr;
    unsigned char State;

    // if( NandDev->BadBlock[BlockNumber] == 'X' ) return TRUE; // if it's a bad block, not erase it.

    BlockAddr = BlockNumber << 5;
    Dummy = NAND_ACCESS_START;
    NAND_CMD = NAND_CMD_ERASE_SETUP;              // C Area Read Command
    NAND_ADDR = (BlockAddr & 0xFF);               // Block Page Address L
    NAND_ADDR = ((BlockAddr >> 8) & 0xFF);        // Block Page Address M
    if (NandDev->TotalSize >= 64 * 1024 * 1024) { // Block Page Address H
        NAND_ADDR = ((BlockAddr >> 16) & 0xFF);
    }

    NAND_AddressSetupWait();

    NAND_CMD = NAND_CMD_ERASE_RUN;

    while (1) {
        NAND_CMD = NAND_CMD_READ_STATE; // command that asks to get current state
        State = NAND_DATA & 0xFF;       // get the current state
        if ((State & NAND_STATUS_BUSY) != 0)
            break; // Busy 상태이면 계속 돈다.
    }

    Dummy = NAND_ACCESS_END;

    if ((State & NAND_STATUS_ERASE_OK) != 0) {
        printf("NAND Erase Block [%d] Fail\n", BlockNumber);
        if ((State & NAND_STATUS_PROTECTED) == 0)
        {
            printf("NAND Device Lock\n");
        }
        return FALSE;
    }

    /*
	//debuging
  
    	NAND_CMD      = NAND_CMD_READ_A; 
    	NAND_ADDR     =                    0x00;      
    	NAND_ADDR     = (BlockNumber << 5) & 0xff;
    	NAND_ADDR     = (BlockNumber >> 3) & 0xff;
    	if( NandDev->TotalSize >= 64*1024*1024 ) {    // Page Address H
        	NAND_ADDR = (BlockNumber >> 11) & 0xff;
    	}
    	NAND_AddressSetupWait();

	printf( "[%d] %02x %02x\n", BlockNumber, NAND_DATA, NAND_DATA );
    */

    return TRUE;
}


void NAND_ReadPage(TNandInfo *NandDev, int PageNumber, unsigned char *pBuf) {
    unsigned long Dummy;
    unsigned long Loop;
    unsigned char *pReadBuf;
    unsigned long *PtrNandData;

    PtrNandData = (unsigned long *)&(NAND_DATA);
    pReadBuf = pBuf;

    Dummy = NAND_ACCESS_START;

    NAND_CMD = NAND_CMD_READ_A;
    NAND_ADDR = 0x00;
    NAND_ADDR = (PageNumber & 0xFF);            // Page Address L
    NAND_ADDR = ((PageNumber >> 8) & 0xFF);     // Page Address M
    if (NandDev->TotalSize >= 64 * 1024 * 1024) { // Page Address H
        NAND_ADDR = ((PageNumber >> 16) & 0xFF);
    }

    NAND_AddressSetupWait();

    // reads 512 bytes
    for (Loop = 0; Loop < 512; Loop++)
        *pReadBuf++ = *PtrNandData;

    Dummy = NAND_ACCESS_END;
}


BOOL NAND_WritePage(TNandInfo *NandDev, int PageNumber, unsigned char *pBuf) {
    unsigned long Dummy;
    unsigned long Loop;
    unsigned char State;
    unsigned char *pWriteBuf;

    pWriteBuf = pBuf;

    Dummy = NAND_ACCESS_START;

    NAND_CMD = NAND_CMD_READ_A;      // NAND read start
    NAND_CMD = NAND_CMD_WRITE_START; // NAND write start
    NAND_ADDR = 0x00;
    NAND_ADDR = (PageNumber & 0xFF);            // Page Address L
    NAND_ADDR = ((PageNumber >> 8) & 0xFF);     // Page Address M
    if (NandDev->TotalSize >= 64 * 1024 * 1024) { // Page Address H
        NAND_ADDR = ((PageNumber >> 16) & 0xFF);
    }
    NAND_AddressSetupWait();

    // writes the 512 bytes
    for (Loop = 0; Loop < 512; Loop++)
        NAND_DATA = *pWriteBuf++;

    NAND_CMD = NAND_CMD_WRITE_RUN;

    while (1) {
        NAND_CMD = NAND_CMD_READ_STATE; // execute the command to get the current status
        State = NAND_DATA & 0xFF;       // get the current status
        if ((State & NAND_STATUS_BUSY) != 0)
            break; // keep loop it if the state is busy
    }
    Dummy = NAND_ACCESS_END;

    if ((State & NAND_STATUS_PROGRAM_OK) == 0)
        return TRUE;
    printf("NAND Write  Fail\n");

    if ((State & NAND_STATUS_PROTECTED) == 0)
    {
        printf("NAND Device Lock\n");
    }

    return FALSE;
}


int NAND_GetPageFromSector(TNandInfo *NandDev, int PartitionNumber, int SectorNumber) {
    int VirBlock;  // virtual block that skips the bad block
    int RealBlock; // real block

    if (PartitionNumber >= NandDev->PartitionNR)
        return -1;

    RealBlock = NandDev->pPartition[PartitionNumber].BaseBlock + (SectorNumber / 32);
    VirBlock = NandDev->VirBlock[RealBlock];

    if (0xffff == VirBlock)
        return -1;

    return (VirBlock * 32 + (SectorNumber % 32));
}

BOOL NAND_WriteSector(TNandInfo *NandDev, int PartitionNumber, int SectorNumber, unsigned char *pBuf) {
    int PageNumber;
    int BlockNumber;

    PageNumber = NAND_GetPageFromSector(NandDev, PartitionNumber, SectorNumber);
    if (PageNumber < 0)
        return FALSE;

    BlockNumber = PageNumber / 32;

    if (NandDev->pPartition[PartitionNumber].BaseBlock > BlockNumber)
        return FALSE;
    if ((NandDev->pPartition[PartitionNumber].BaseBlock + NandDev->pPartition[PartitionNumber].BlockCount) <= BlockNumber)
        return FALSE;


    return NAND_WritePage(NandDev, PageNumber, pBuf);
}


BOOL NAND_ReadSector(TNandInfo *NandDev, int PartitionNumber, int SectorNumber, unsigned char *pBuf) {
    int PageNumber;
    int BlockNumber;

    PageNumber = NAND_GetPageFromSector(NandDev, PartitionNumber, SectorNumber);
    if (PageNumber < 0)
        return FALSE;

    BlockNumber = PageNumber / 32;

    if (NandDev->pPartition[PartitionNumber].BaseBlock > BlockNumber)
        return FALSE;
    if ((NandDev->pPartition[PartitionNumber].BaseBlock + NandDev->pPartition[PartitionNumber].BlockCount) <= BlockNumber)
        return FALSE;

    NAND_ReadPage(NandDev, PageNumber, pBuf);

    return TRUE;
}


BOOL NAND_DETECT(unsigned long BaseAddress, TNandInfo *NandDev, TNandPartition *par) {
    unsigned char MakerID, SizeID;
    unsigned short Dummy;
    int Loop;
    int RestBlockCount;

    printf("NAND Chip Check .......................\n");
    //printf( "  Base Address [%08X]\n", (unsigned int)BaseAddress );

    NandDev->BaseAddress = BaseAddress;

    Dummy = NAND_ACCESS_START;
    NAND_CMD = NAND_CMD_READ_ID; // read ID command
    NAND_ADDR = 0x00;            // additional things that would be done after finishing the main task of the command

    MakerID = NAND_DATA & 0xFF; // 제조사 읽기
    SizeID = NAND_DATA & 0xFF;  // 크기 읽기
    Dummy = NAND_ACCESS_END;

    //printf( "  NAND Maker ID [%02X]  ", MakerID );
    //printf( "  NAND Size  ID [%02X]\n", SizeID );

    NandDev->Type = NAND_TYPE_UNKNOW;
    switch (MakerID) {
        case 0xEC:
                switch (SizeID) {
                case 0x73:
                    NandDev->Type = NAND_TYPE_SAMSUNG_16M;
                    NandDev->TotalSize = 16 * 1024 * 1024;
                    NandDev->EraseSize = 16 * 1024;
                    break;
                case 0x75:
                    NandDev->Type = NAND_TYPE_SAMSUNG_32M;
                    NandDev->TotalSize = 32 * 1024 * 1024;
                    NandDev->EraseSize = 16 * 1024;
                    break;
                case 0x76:
                    NandDev->Type = NAND_TYPE_SAMSUNG_64M;
                    NandDev->TotalSize = 64 * 1024 * 1024;
                    NandDev->EraseSize = 16 * 1024;
                    break;
                case 0x79:
                    NandDev->Type = NAND_TYPE_SAMSUNG_128M;
                    NandDev->TotalSize = 128 * 1024 * 1024;
                    NandDev->EraseSize = 16 * 1024;
                    break;
                }
                break;
    }

    switch (NandDev->Type) {
        case NAND_TYPE_SAMSUNG_16M:
            printf("  Detect SAMSUNG [%02x:%02x] 16MByte\n", MakerID, SizeID);
            break;
        case NAND_TYPE_SAMSUNG_32M:
            printf("  Detect SAMSUNG [%02x:%02x] 32MByte\n", MakerID, SizeID);
            break;
        case NAND_TYPE_SAMSUNG_64M:
            printf("  Detect SAMSUNG [%02x:%02x] 64MByte\n", MakerID, SizeID);
            break;
        case NAND_TYPE_SAMSUNG_128M:
            printf("  Detect SAMSUNG [%02x:%02x] 128MByte\n", MakerID, SizeID);
            break;
        default:
            printf("  Unknown Type\n");
            break;
    }

    if (NandDev->Type == NAND_TYPE_UNKNOW)
        return FALSE;

    // add the partition data to the linked list
    NandDev->pPartition = par;
    NandDev->PartitionNR = 0;

    // scan the bad block
    printf("  BAD BLOCK SCAN ->");
    printf("  Kernel, Ramdisk Bad Block [%d]\n", NAND_ScanBadBlock(NandDev));

    // check the partition information
    RestBlockCount = NandDev->TotalSize / NandDev->EraseSize;
    for (Loop = 0; Loop < NAND_MAX_PATITION; Loop++) {
        if (NandDev->pPartition[Loop].BaseBlock == NAND_PARTITON_END)
            break;

        NandDev->PartitionNR++;
        if (NandDev->pPartition[Loop].BlockCount == NAND_PARTITON_REST_SIZE) {
            NandDev->pPartition[Loop].BlockCount = RestBlockCount;
            break;
        } else
        {
            RestBlockCount -= NandDev->pPartition[Loop].BlockCount;
        }
    }
    /*
    	// print out the partition information. 
    	printf( " ====================================================================\n" );
    	for( Loop = 0; Loop < NandDev->PartitionNR; Loop++ )
    	{
        	printf( "  NAND Partition %d -> START %6d, SIZE %6d, Name : %s\n",
                     Loop,
                     NandDev->pPartition[Loop].BaseBlock ,
                     NandDev->pPartition[Loop].BlockCount ,
                     NandDev->pPartition[Loop].Name );
    	}
    	printf( " --------------------------------------------------------------------\n" );
    */

    return TRUE;
}


static int dpywidth = 50;
static char bar[] = "===============================================================";
static char spaces[] = "                                                               "
                       "                                                               ";
static int progress_pos = 0;

void NandFlash_UpdateProgress(const char *title, unsigned long cur, unsigned long max) {
    const char spinner[] = "\\|/-";
    unsigned int percent;
    int i;

    progress_pos = (progress_pos + 1) & 3;
    percent = (cur * 100) / max;
    i = ((percent * dpywidth) + 50) / 100;

    printf("%s: |%s%s", title, bar + (sizeof(bar) - (i + 1)), spaces + (sizeof(spaces) - (dpywidth - i + 1)));

    if (percent == 100)
        putc('|');
    else
        putc(spinner[progress_pos & 3]);

    printf(" %4d%%   \r", percent);

    // clear prog bar
    if (percent == 100) {
        printf("%s\r", spaces + (sizeof(spaces) - 80));
    }
}

BOOL Nand_ErasePartition(int pnb, BOOL NoEraseBadBlock) {
    int From, MaxCount, Loop;
    int BadBlockCnt = 0;

    From = Nand1.pPartition[pnb].BaseBlock;
    MaxCount = Nand1.pPartition[pnb].BlockCount;

    for (Loop = 0; Loop < MaxCount; Loop++) {
        NandFlash_UpdateProgress("    Erasing", Loop + 1, MaxCount);
        // if it is a bad block, shouldn't erase it
        if (NoEraseBadBlock == TRUE) {
            if (Nand1.BadBlock[From] != 'X') {
                NAND_EraseBlock(&Nand1, From);
            } else {
                BadBlockCnt++;
            }
        } else {
            if (NAND_EraseBlock(&Nand1, From) == FALSE) {
                BadBlockCnt++;
            }
        }

        From++;
    }
    NandFlash_UpdateProgress("    Erasing", 100, 100);
    printf("    Erase : OK\n");

    return TRUE;
}

int Nand_ProgramPartition(int pnb, unsigned int src, int size) {
    int page_cnt, page;
    unsigned char *pbuf = (unsigned char *)src;

    TParPageInfo *pParInfo;
    unsigned char page_buff[512];

    if ((0 > pnb) || (pnb > USE_PARTITION_MAX)) {
        return -1;
    }

    // put the data info header in the first page
    pParInfo = (TParPageInfo *)page_buff;
    pParInfo->data_size = size;
    NAND_WriteSector(&Nand1, pnb, 0, page_buff);

    // writes data
    page_cnt = (size + 511) / 512;
    for (page = 0; page < page_cnt; page++) {
        if (0 == (page % 128))
            NandFlash_UpdateProgress("    Writing", page + 1, page_cnt);

        // The 0th page is using as data area
        NAND_WriteSector(&Nand1, pnb, page + 1, pbuf);

        pbuf += 512;
    }
    NandFlash_UpdateProgress("    Writing", 100, 100);
    printf("    Write : OK\n");

    return size;
}

BOOL Nand_VerifyPartition(int pnb, unsigned int src, int size) {
    unsigned char pbuf[512];
    unsigned int *cmp_src, *cmp_flh;
    int page_cnt, page;
    int loop;

    page_cnt = (size + 511) / 512;
    cmp_src = (unsigned int *)src;

    for (page = 0; page < page_cnt; page++) {
        if (0 == (page % 128))
            NandFlash_UpdateProgress("    Verifing", page + 1, page_cnt);

        // The 0th page is using as data area
        NAND_ReadSector(&Nand1, pnb, page + 1, pbuf);

        cmp_flh = (unsigned int *)pbuf;
        for (loop = 0; loop < 512 / 4; loop++) {
            if (*cmp_src != *cmp_flh) {
                printf("Verify Error page-%d: %08x/%08x\n", page, *cmp_src, *cmp_flh);
                return FALSE;
            }
            cmp_src++;
            cmp_flh++;
        }
    }
    NandFlash_UpdateProgress("    Verifing", 100, 100);
    printf("    Verify: OK\n");

    return TRUE;
}

int Nand_ReadPartition(int pnb, unsigned int dst, int size) {
    int page_cnt, page, max_page;
    unsigned char *pbuf = (unsigned char *)dst;

    // The 0th page is using as data area
    TParPageInfo *pParInfo;
    unsigned char page_buff[512];

    NAND_ReadSector(&Nand1, pnb, 0, page_buff);

    pParInfo = (TParPageInfo *)page_buff;

    if (size > pParInfo->data_size) {
        size = pParInfo->data_size;
    }

    page_cnt = (size + 511) / 512;
    max_page = Nand1.pPartition[pnb].BlockCount * 32;
    if (page_cnt > max_page)
        page_cnt = max_page;

    for (page = 0; page < page_cnt; page++) {
        // The 0th page is using as data area
        NAND_ReadSector(&Nand1, pnb, page + 1, pbuf);
        pbuf += 512;
    }

    return size;
}

int NandFlash_Init(void) {
    NAND_Par_Fixup();
    NAND_DETECT(NAND_BASE, &Nand1, &Nand1Partition[0]);

    return 0;
}

int CopyTo_NandFlash_Kernel(unsigned int src, int size) {
    if (TRUE != Nand_ErasePartition(PARTITION_NUM_KERNEL, TRUE))
        return -1;
    if (size != Nand_ProgramPartition(PARTITION_NUM_KERNEL, src, size))
        return -1;
    if (TRUE != Nand_VerifyPartition(PARTITION_NUM_KERNEL, src, size))
        return -1;

    return size;
}

int CopyTo_NandFlash_Ramdisk(unsigned int src, int size) {
    if (TRUE != Nand_ErasePartition(PARTITION_NUM_RAMDISK, TRUE))
        return -1;
    if (size != Nand_ProgramPartition(PARTITION_NUM_RAMDISK, src, size))
        return -1;
    if (TRUE != Nand_VerifyPartition(PARTITION_NUM_RAMDISK, src, size))
        return -1;

    return size;
}

int CopyTo_SDRAM_Kernel(unsigned int dst) {
    int readcnt;

    readcnt = (Nand1.pPartition[PARTITION_NUM_KERNEL].BlockCount - 4) * 32 * 512;
    Nand_ReadPartition(PARTITION_NUM_KERNEL, dst, readcnt);

    return readcnt;
}

int CopyTo_SDRAM_Ramdisk(unsigned int dst) {
    int readcnt;

    readcnt = (Nand1.pPartition[PARTITION_NUM_RAMDISK].BlockCount - 12) * 32 * 512;
    Nand_ReadPartition(PARTITION_NUM_RAMDISK, dst, readcnt);

    return readcnt;
}
