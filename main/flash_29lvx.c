// This file contains the process routines of FB29LVxflash of ezboot device.
//
// modified by Yeonwoo Sung

#include <pxa255.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <flash_29lvx.h>
#include <mem_map.h>


// flash blocks
const int FB29LV400T_block_addr[] =
    {0x00000, 0x10000, 0x20000, 0x30000,
     0x40000, 0x50000, 0x60000, 0x70000,
     0x78000, 0x7A000, 0x7C000,
     -1};

const int FB29LV400B_block_addr[] =
    {0x00000, 0x04000, 0x06000, 0x08000,
     0x10000, 0x20000, 0x30000, 0x40000,
     0x50000, 0x60000, 0x70000,
     -1};

const int FB29LV800T_block_addr[] =
    {0x00000, 0x10000, 0x20000, 0x30000,
     0x40000, 0x50000, 0x60000, 0x70000,
     0x80000, 0x90000, 0xA0000, 0xB0000,
     0xC0000, 0xD0000, 0xE0000, 0xF0000,
     0xF8000, 0xFA000, 0xFC000,
     -1};

const int FB29LV800B_block_addr[] =
    {0x00000, 0x04000, 0x06000, 0x08000,
     0x10000, 0x20000, 0x30000, 0x40000,
     0x50000, 0x60000, 0x70000, 0x80000,
     0x90000, 0xA0000, 0xB0000, 0xC0000,
     0xD0000, 0xE0000, 0xF0000,
     -1};

const int FB29LV160T_block_addr[] =
    {0x000000, 0x010000, 0x020000, 0x030000,
     0x040000, 0x050000, 0x060000, 0x070000,
     0x080000, 0x090000, 0x0A0000, 0x0B0000,
     0x0C0000, 0x0D0000, 0x0E0000, 0x0F0000,
     0x100000, 0x110000, 0x120000, 0x130000,
     0x140000, 0x150000, 0x160000, 0x170000,
     0x180000, 0x190000, 0x1A0000, 0x1B0000,
     0x1C0000, 0x1D0000, 0x1E0000, 0x1F0000,
     0x1F8000, 0x1FA000, 0x1FC000,
     -1};

const int FB29LV160B_block_addr[] =
    {0x000000, 0x004000, 0x006000, 0x008000,
     0x010000, 0x020000, 0x030000, 0x040000,
     0x050000, 0x060000, 0x070000, 0x080000,
     0x090000, 0x0A0000, 0x0B0000, 0x0C0000,
     0x0D0000, 0x0E0000, 0x0F0000, 0x100000,
     0x110000, 0x120000, 0x130000, 0x140000,
     0x150000, 0x160000, 0x170000, 0x180000,
     0x190000, 0x1A0000, 0x1B0000, 0x1C0000,
     0x1D0000, 0x1E0000, 0x1F0000,
     -1};



// global variables for flash

int Flash_PID = 0; // flash id to identify each flash
int BlockCount = 0;
int FlashSize = 0;
int *pBlockAddr = (int *)FB29LV400B_block_addr;



// macros

#define RETRY_COUNT 100
#define WAIT_LOOP_COUNT 200
#define FB29LVx_WriteMemoryShort(adr, data)     \
    *(unsigned short *)(adr) = (data & 0xffff); \
    FB29LVx_Wait();
#define FB29LVx_ReadMemoryShort(adr) (*(unsigned short *)(adr)&0xffff);



//=============================================================================

/**
 * This function makes the flash wait.
 */
void FB29LVx_Wait() {
    int loop = WAIT_LOOP_COUNT;

    while (loop) {
        loop -= 1;
    };
}

/**
 * This function uses the predefined macro to reset the flash.
 *
 * @param (vBase) the base register address (32bit address)
 */
void FB29LVx_CMD_RESET(Word32 vBase) {
    FB29LVx_WriteMemoryShort(vBase, 0xF0);
}

/**
 * This function reads the id of the flash for the basic set up.
 *
 * @param (vBase) the address of the base register (32bit address)
 */
void FB29LVx_CMD_READID_SETUP(Word32 vBase) {
    FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0xAA);
    FB29LVx_WriteMemoryShort(vBase + (0x2AA << 1), 0x55);
    FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0x90);
}

/**
 * This function writes the given data.
 *
 * @param (vBase) the base register address
 * @param (vOffset) the offset to calculate the target memory address
 * @param (vData) the data that should be written
 */
void FB29LVx_CMD_WRITE(unsigned long vBase, unsigned long vOffset, Word16 vData) {
    FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0xAA);
    FB29LVx_WriteMemoryShort(vBase + (0x2AA << 1), 0x55);
    FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0xA0);
    FB29LVx_WriteMemoryShort(vBase + vOffset, vData);
}

/**
 * This function gets the command execution status.
 *
 * @param (vBase) the base register address (32 bit address)
 * @return The status code will be returned.
 */
int FB29LVx_GetStatus(unsigned long vBase) {
    unsigned char status, XorStatus;

    while (1) {
        status = FB29LVx_ReadMemoryShort(vBase);
        XorStatus = status ^ FB29LVx_ReadMemoryShort(vBase);
        XorStatus &= (64 | 4); // 64 = (1 << 6), 4 = (1 << 2)

        // check the timeout
        if (status & 32) { //32 = (1 << 5)
            return FB29LVx_STATUS_TIMEOUT;
        }

        // suspend
        if (status & (1 << 7)) {
            return FB29LVx_STATUS_ERSUSP;
        }

        // when there is no change in the status value, return the success code.
        if (XorStatus == 0) {
            return FB29LVx_STATUS_READY;
        }
    }

    return FB29LVx_STATUS_ERROR; // return the error code when the error is occurred
}

/**
 * Detect FB29LVx to get it's information.
 *
 * @param (vBase) the memory address of the base register (32 bit address)
 * @return
 */
int FB29LVx_Detect(Word32 vBase) {
    unsigned short mid, pid;
    char mid_str[32];

    FB29LVx_CMD_RESET(vBase);
    FB29LVx_CMD_READID_SETUP(vBase);

    // manufacturer company code
    mid = FB29LVx_ReadMemoryShort(vBase + 0);
    // product code of the flash
    pid = FB29LVx_ReadMemoryShort(vBase + (1 << 1));

    FB29LVx_CMD_RESET(vBase);

    // use the switch statement to set the corresponding manufacturer id
    switch (mid) {
        case MXIC_MID:
            strcpy(mid_str, "MX");
            break;
        case AMD_MID:
            strcpy(mid_str, "AM");
            break;
        case ES_MID:
            strcpy(mid_str, "ES");
            break;
        default: //unknown manufacturer id
            strcpy(mid_str, "xx");
            break;
    }

    // use the switch statement to set the corresponding product id
    switch (pid) {
        case FB_DID_400T:

            Flash_PID = pid;
            BlockCount = FB_400x_BLOCK_COUNT;
            FlashSize = 512 * 1024;
            pBlockAddr = (int *)FB29LV400T_block_addr;

            printf("  Detect %s29LV400 (TOP)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 4M-BIT [512Kbyte]\n");
            break;

        case FB_DID_400B:

            Flash_PID = pid;
            BlockCount = FB_400x_BLOCK_COUNT;
            FlashSize = 512 * 1024;
            pBlockAddr = (int *)FB29LV400B_block_addr;

            printf("  Detect %s29LV400 (BOTTOM)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 4M-BIT [512Kbyte]\n");
            break;

        case FB_DID_800T:

            Flash_PID = pid;
            BlockCount = FB_800x_BLOCK_COUNT;
            FlashSize = 1024 * 1024;
            pBlockAddr = (int *)FB29LV800T_block_addr;

            printf("  Detect %s29LV800T (TOP)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 8M-BIT [1Mbyte]\n");
            break;

        case FB_DID_800B:

            Flash_PID = pid;
            BlockCount = FB_800x_BLOCK_COUNT;
            FlashSize = 1024 * 1024;
            pBlockAddr = (int *)FB29LV800B_block_addr;

            printf("  Detect %s29LV800T (BOTTOM)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 8M-BIT [1Mbyte]\n");
            break;

        case FB_DID_160T:

            Flash_PID = pid;
            BlockCount = FB_160x_BLOCK_COUNT;
            FlashSize = 2048 * 1024;
            pBlockAddr = (int *)FB29LV160T_block_addr;

            printf("  Detect %s29LV160T (TOP)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 16M-BIT [2Mbyte]\n");
            break;

        case FB_DID_160B:

            Flash_PID = pid;
            BlockCount = FB_160x_BLOCK_COUNT;
            FlashSize = 2048 * 1024;
            pBlockAddr = (int *)FB29LV160B_block_addr;

            printf("  Detect %s29LV160B (BOTTOM)Flash : %04X\n", mid_str, pid);
            printf("  SIZE 16M-BIT [2Mbyte]\n");
            break;

        default:
            printf("  Unknown Flash : pid=%04X\n", pid);
            return -1;
    }

    return 0;
}

/**
 * Erases the block.
 *
 * @param (vBase) The base register address (32 bit address)
 * @param (vBlkAddr) The address of the target block
 * @return Returns 0 on success, and returns -1 on error.
 *
 * @{warning} Did not implement the validation part of the block address. Thus, the system should pass the valid block address.
 *            Otherwise, the undefined behaviour will be occurred with unexpected errors.
 */
int FB29LVx_EraseBlock(unsigned long vBase, unsigned long vBlkAddr) {
    Word16 status;
    int loop;

    for (loop = 0; loop < RETRY_COUNT; loop++) {
        FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0xAA);
        FB29LVx_WriteMemoryShort(vBase + (0x2AA << 1), 0x55);
        FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0x80);
        FB29LVx_WriteMemoryShort(vBase + (0x555 << 1), 0xAA);
        FB29LVx_WriteMemoryShort(vBase + (0x2AA << 1), 0x55);

        FB29LVx_WriteMemoryShort(vBase + vBlkAddr, 0x30);

        FB29LVx_Wait(); //use the FB28LVx_Wait() to sleep

        status = FB29LVx_GetStatus(vBase);
        if (status == FB29LVx_STATUS_READY) {
            FB29LVx_CMD_RESET(vBase);
            return 0;
        } else {
            FB29LVx_CMD_RESET(vBase);
        }
    }

    printf("Erase Error [retry count]...\n");
    return -1;
}

/**
 * Writes the data into the flash.
 *
 * @param (vBase) the base register address
 * @param (vOffset) the offset to calculate the target memory address
 * @param (data) the data that should be written in the flash memory
 * @return Returns 0 on success, and returns -1 on error.
 */
int FB29LVx_WriteWord(Word32 vBase, Word32 vOffset, Word16 data)
{
    Word16 status;
    int loop;

    for (loop = 0; loop < RETRY_COUNT; loop++) {
        FB29LVx_CMD_WRITE(vBase, vOffset, data); // write the data

        status = FB29LVx_GetStatus(vBase);

        if (status == FB29LVx_STATUS_READY) {
            FB29LVx_Wait();
            return 0;
        } else {
            FB29LVx_CMD_RESET(vBase);
        }
    }
    printf("Write Error [retry count]...\n");
    return -1;
}

/**
 * Finds the block number from the offset.
 * @return On success, returns the corresponding block number. Otherwise, returns the -1.
 */
int FB29LVx_GetBlockIndex(unsigned long vOffset) {
    int blk;

    // check if the given offset is greater than or equal to the flash size (to check if the offset is in the correct range)
    if (FlashSize <= vOffset) {
        return -1;
    }

    // find the block number by using the for loop
    for (blk = 0; blk < BlockCount; blk++) {
        if (0 > pBlockAddr[blk]) { //invalid address
            return -1;
        }

        if (0 < pBlockAddr[blk + 1]) {
            if ((pBlockAddr[blk] <= vOffset) && (vOffset < pBlockAddr[blk + 1])) {
                break; // break the loop when it finds the block number
            }
        } else {
            break; // because it is the last block
        }
    }

    return blk;
}



static int dpywidth = 50;
static char bar[] = "===============================================================";
static char spaces[] = "                                                                                                                              "; // 127 spaces
static int progress_pos = 0;

/**
 * This function prints out the progresses.
 *
 * @param (title) the title of the status
 * @param (cur) the current progress
 * @param (max) the maximum value
 */
void Flash_UpdateProgress(const char *title, unsigned long cur, unsigned long max) {
    const char spinner[] = "\\|/-";
    unsigned int percent;
    int i;

    progress_pos = (progress_pos + 1) & 3;
    percent = (cur * 100) / max;
    i = ((percent * dpywidth) + 50) / 100;

    printf("%s: |%s%s", title,
           bar + (sizeof(bar) - (i + 1)),
           spaces + (sizeof(spaces) - (dpywidth - i + 1)));

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


/**
 * Finds the boot flash to initialise it.
 *
 * @return The memory address of the boot flash.
 */
int BootFlash_Init(void) {
    printf("Boot Flash Check ......................\n");
    return FB29LVx_Detect((Word32)0x00000000);
}


/**
 * Writes data to the Flash.
 *
 * @param (dest) the flash starting address
 * @param (src) the starting address of the RAM disk that the data is stored in
 * @param (size) the size of the data
 * @return On error, returns -1. Otherwise, returns the size of written data.
 */
int BootFlash_ProgramEx(Word32 vBase, Word32 vOffset, char *src, int size, int with_erase) {
    int s_blk, e_blk;
    int idx;
    Word16 wData;

    // 지울 블럭의 시작과 끝을 찾는다.
    s_blk = FB29LVx_GetBlockIndex(vOffset);
    e_blk = FB29LVx_GetBlockIndex(vOffset + size);

    if ((s_blk < 0) || (e_blk < 0)) {
        printf("Invalid Address... 0x%08x\n", vBase + vOffset);
        return -1;
    }

    if (with_erase) {
        // 써넣을 공간의 블럭을 지운다.
        for (idx = s_blk; idx <= e_blk; idx++) {
            Flash_UpdateProgress("    Erasing", idx - s_blk + 1, e_blk - s_blk + 1);

            if (0 > FB29LVx_EraseBlock(vBase, pBlockAddr[idx]))
                return -1;
            msleep(1000);
        }
        Flash_UpdateProgress("    Erasing", 100, 100);
        printf("    Erase : OK\n");
    }

    for (idx = 0; idx < size; idx += 2) {
        if (0 == idx % 512)
            Flash_UpdateProgress("    Writing", idx, size);

        wData = (src[idx] & 0xff) | ((src[idx + 1] & 0xff) << 8);
        if (0 > FB29LVx_WriteWord(vBase, vOffset + idx, wData))
            return -1;

        if (*(Word16 *)(vBase + vOffset + idx) != wData) {
            if (0 > FB29LVx_WriteWord(vBase, vOffset + idx, wData))
                return -1;
        }
    }
    Flash_UpdateProgress("    Writing", 100, 100);
    printf("    Write : OK\n");

    FB29LVx_CMD_RESET(vBase);

    return size;
}


/**
 * Copies the data to the flash memory.
 *
 * @param (dest) the starting address of the destination
 * @param (src) the starting address of the source
 * @param (size) the size of the data
 * @param (with_erase) non zero value for with_erase mode
 *
 * @return Returns 0 on success. Otherwise, returns -1.
 */
int CopyTo_BootFlashEx(unsigned int dest, unsigned int src, int size, int with_erase) {
    int idx;
    unsigned short *pdst = (unsigned short *)dest;
    unsigned short *psrc = (unsigned short *)src;

    if (with_erase) {
        BootFlash_Program(0, dest, (char *)src, size);
    } else {
        BootFlash_ProgramEx(0, dest, (char *)src, size, 0);
    }

    for (idx = 0; idx < size / 2; idx++) {
        if (*pdst != *psrc) {
            printf("Verify Error %p: %04x/%04x\n", pdst, *pdst, *psrc);
            return -1;
        }
        pdst++;
        psrc++;
    }

    return 0;
}

/**
 * The default copy_to_bootloader function.
 */
void SelfCopy_BootLoader(void) {
    CopyTo_BootFlash(DEFAULT_FLASH_BOOT, DEFAULT_RAM_BOOT_START, DEFAULT_BOOT_SIZE);
}
