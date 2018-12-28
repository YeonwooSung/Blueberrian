// This header file contains the codes for nand that the Blueberrian OS uses.
//
// Modified by Yeonwoo Sung 2018.12.28


#ifndef BLUEBERRIAN_NAND_HEADER
#define BLUEBERRIAN_NAND_HEADER

#define NAND_BASE DEFAULT_nCS_NAND_BASE

#define NAND_ACCESS_START *((volatile short *)(NandDev->BaseAddress + 0x000))
#define NAND_DATA *((volatile short *)(NandDev->BaseAddress + 0x000))
#define NAND_CMD *((volatile short *)(NandDev->BaseAddress + 0x100))
#define NAND_ADDR *((volatile short *)(NandDev->BaseAddress + 0x200))
#define NAND_ACCESS_END *((volatile short *)(NandDev->BaseAddress + 0x300))

#define PARTITION_NUM_KERNEL 0
#define PARTITION_NUM_RAMDISK 1
#define PARTITION_NUM_APPIMAGE 2

// 0th page of each partition will be used as an information area
typedef struct {
    int data_size;
} __attribute__((packed)) TParPageInfo;

// partition information
typedef struct {
    unsigned char Name[128];
    int BaseBlock;  // the base block
    int BlockCount; // the number of blocks
} __attribute__((packed)) TNandPartition;

#define NAND_MAX_PATITION 8

#define NAND_PARTITON_REST_SIZE -1
#define NAND_PARTITON_END -1

typedef struct {
    unsigned long Type;
    unsigned long BaseAddress; // the base address
    unsigned long TotalSize;   // total size of usable blocks
    unsigned long EraseSize;   // the size of erased blocks

    char BadBlock[1024 * 8];           // bad blocks will be marked as 'X', and normal blocks will be marked as 'O'
    unsigned short VirBlock[1024 * 8]; // the array contains the virtual blocks
    int BadBlockCount;

    TNandPartition *pPartition; // the address of partition information
    int PartitionNR;            // the number of partitions
} __attribute__((packed)) TNandInfo;

#define NAND_CMD_READ_ID 0x90     // ID read command
#define NAND_CMD_READ_STATE 0x70  // status read command
#define NAND_CMD_ERASE_SETUP 0x60 // erase setup command
#define NAND_CMD_ERASE_RUN 0xD0   // erase execute command
#define NAND_CMD_WRITE_START 0x80 // erase start command
#define NAND_CMD_WRITE_RUN 0x10   // write execute command
#define NAND_CMD_READ_A 0x00      // read area A
#define NAND_CMD_READ_B 0x01      // read area B
#define NAND_CMD_READ_C 0x50      // read area C

/* NAND status codes */
#define NAND_STATUS_ERASE_OK 0x01
#define NAND_STATUS_PROGRAM_OK 0x01
#define NAND_STATUS_BUSY 0x40
#define NAND_STATUS_PROTECTED 0x80

#define NAND_STATUS_NORMAL_MASK (NAND_STATUS_ERASE_OK || NAND_STATUS_PROGRAM_OK || NAND_STATUS_BUSY || NAND_STATUS_PROTECTED)

#define NAND_TYPE_UNKNOW 0
#define NAND_TYPE_SAMSUNG_16M 0x73
#define NAND_TYPE_SAMSUNG_32M 0x75
#define NAND_TYPE_SAMSUNG_64M 0x76
#define NAND_TYPE_SAMSUNG_128M 0x79

#ifndef NAND_VAR
extern TNandInfo Nand1;
extern TNandPartition Nand1Partition[NAND_MAX_PATITION];
#endif

extern int NandFlash_Init(void);
extern BOOL Nand_ErasePartition(int pnb, BOOL NoEraseBadBlock);
extern int CopyTo_NandFlash_Kernel(unsigned int src, int size);
extern int CopyTo_NandFlash_Ramdisk(unsigned int src, int size);
extern int CopyTo_NandFlash_AppImage(unsigned int src, int size);
extern int CopyTo_SDRAM_Kernel(unsigned int dst);
extern int CopyTo_SDRAM_Ramdisk(unsigned int dst);

#endif //BLUEBERRIAN_NAND_HEADER
