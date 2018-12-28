// The aim of this file is to define the memory mapping functions of the Blueberrian OS
// This file is modified from linux kernel project (concise version).
//
// modified by Yeonwoo Sung (2018-12-28)

#ifndef BLUEBERRIAN_MEM_MAP_HEADER
#define BLUEBERRIAN_MEM_MAP_HEADER

#define LINUX_MACH_TYPE 303

#define BANK0_START 0xA0000000        // 뱅크0 시작 위치
#define BANK0_SIZE (64 * 1024 * 1024) // 뱅크0 크기

#define DEFAULT_nCS_CS8900_BASE (PXA_CS0_PHYS + 0x00400000)

#define DEFAULT_nCS_NAND_BASE (PXA_CS1_PHYS + 0x00000000)
#define DEFAULT_nCS_MK712_BASE (PXA_CS1_PHYS + 0x00400000)

#define DEFAULT_FLASH_BOOT 0x00000000                               // boot start address in flash
#define DEFAULT_BOOT_SIZE (128 * 1024)                              // size of the boot area
#define DEFAULT_BOOT_PARAM (DEFAULT_FLASH_BOOT + DEFAULT_BOOT_SIZE) // boot parameter area in flash
#define DEFAULT_BOOT_PARAM_SIZE (2 * 1024)

#define DEFAULT_RAM_BOOT_START 0xA0F00000    // RAM boot starting address
#define DEFAULT_RAM_BOOT_PARAMS 0xA0000100   // kernel boot parameter area
#define DEFAULT_RAM_KERNEL_START 0xA0008000  // RAM kernel starting address
#define DEFAULT_RAM_RAMDISK_START 0xA0800000 // RAM RAM_DISK starting address
#define DEFAULT_RAM_WORK_START 0xA1000000    // boot loader starting address


// The start address of the area that is conveyed from bootloader to kernel
#define DEFAULT_RAM_KERNEL_ZERO_PAGE 0xA0000000

#define DEFAULT_RAM_KERNEL_IMG_SIZE (1 * 1024 * 1024)
#define DEFAULT_RAM_RAMDISK_IMG_SIZE (4 * 1024 * 1024)
#define DEFAULT_RAM_RAMDISK_SIZE (8 * 1024 * 1024)

#define DEFAULT_PARAM_SIZE (2 * 1024)

#endif //BLUEBERRIAN_MEM_MAP_HEADER
