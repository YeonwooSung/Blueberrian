// This file contains the upper routines for NOR and NAND flashes.
// Functions in this file call the corresponding subroutines to make the upper routines work properly.
//
// modified by Yeonwoo Sung


#include <pxa255.h>
#include <stdio.h>
#include <string.h>
#include <config.h>
#include <mem_map.h>
#include <flash.h>


extern TConfig Cfg; // config.c 에 정의


/**
 * This function copies the kernel image and ram disk image.
 */
void CopyImage(void) {
    printf("Copy Kernel Image .....\n");
    CopyTo_SDRAM_Kernel(DEFAULT_RAM_KERNEL_START);

    if (('Y' == Cfg.UseRamdisk) || ('y' == Cfg.UseRamdisk)) {
        printf("Copy Ramdisk Image .....\n");
        CopyTo_SDRAM_Ramdisk(DEFAULT_RAM_RAMDISK_START);
    }
}


/**
 * The aim of this function is to erase the partition.
 *
 * @param (argc) the number of arguments
 * @param (argv) the array that contains the arguments
 * @return Returns 0 on success. Returns 0 on error.
 */
int ErasePartiotion(int argc, char **argv) {
    int ee = TRUE;

    if ((argc >= 2) && (toupper(argv[1][0]) == 'E')) ee = FALSE;

    switch (argv[0][2]) {
    case 'K':
        Nand_ErasePartition(PARTITION_NUM_KERNEL, ee);
        return 0;
    case 'R':
        Nand_ErasePartition(PARTITION_NUM_RAMDISK, ee);
        return 0;
    case 'A':
        Nand_ErasePartition(PARTITION_NUM_APPIMAGE, FALSE);
        return 0;
    }

    return -1;
}
