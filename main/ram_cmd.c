// This file contains the command routines about memory of ezBoot board.
//
// modified by Yeonwoo Sung on 2019.01.18

#include <pxa255.h>
#include <stdio.h>
#include <string.h>

static unsigned long LastRamDumpAddress = 0x000;


void MemoryHexDump(void *address, int size) {
    int lp1, lp2, lp3;
    unsigned char *ptrData;

    ptrData = address;

    for (lp1 = 0; lp1 < size * 4; lp1 += 16) {
        // print out the address
        printf("%04X-%04X :", (((unsigned int)ptrData) >> 16) & 0xFFFF, ((unsigned int)ptrData) & 0xFFFF);

        // 총 4번 표시
        for (lp2 = 0; lp2 < 4; lp2++) {

            for (lp3 = 0; lp3 < 4; lp3++) {
                printf("%02X", ptrData[lp2 * 4 + lp3] & 0xFF);
            }
            printf(" ");

        }

        printf("  ");

        for (lp3 = 0; lp3 < 16; lp3++) {
            if (ptrData[lp3] < ' ')
                printf(".");
            else if (ptrData[lp3] > '~')
                printf(".");
            else
                printf("%c", ptrData[lp3]);
        }

        printf("\n");
        ptrData += 16;
    }

    LastRamDumpAddress = (unsigned long)ptrData;
}

int RamDump(int argc, char **argv) {
    int size;

    switch (argc) {
    case 1: // when the command is given
        MemoryHexDump((void *)LastRamDumpAddress, 16);
        break;
    case 2: // when the starting address is given
        LastRamDumpAddress = strtoul(argv[1], NULL, 0);
        MemoryHexDump((void *)LastRamDumpAddress, 16);
        break;
    case 3: // starting address and the count are given
        LastRamDumpAddress = strtoul(argv[1], NULL, 0);
        size = strtoul(argv[2], NULL, 0);
        if (size < 0)
            size *= -1;

        MemoryHexDump((void *)LastRamDumpAddress, size);
        break;
    default:
        printf("Argument Count Error!\n");
        return -1;
    }

    return 0;
}


int RamWriteChar(int argc, char **argv) {

    unsigned char *ptrTo;
    int lp;

    if (argc < 3) {
        printf("Argument Count Error!\n");
        return -1;
    }

    // get the memory address that will be used

    ptrTo = (unsigned char *)strtoul(argv[1], NULL, 0);

    for (lp = 2; lp < argc; lp++)
        ptrTo[lp - 2] = strtoul(argv[lp], NULL, 0);

    return 0;
}

int RamWriteWord(int argc, char **argv) {
    unsigned short *ptrTo;
    int lp;

    if (argc < 3) {
        printf("Argument Count Error!\n");
        return -1;
    }

    // get the memory address that will be used

    ptrTo = (unsigned short *)strtoul(argv[1], NULL, 0);

    for (lp = 2; lp < argc; lp++)
        ptrTo[lp - 2] = strtoul(argv[lp], NULL, 0);
    return 0;
}

int RamWriteLong(int argc, char **argv) {

    unsigned long *ptrTo;
    int lp;

    if (argc < 3) {
        printf("Argument Count Error!\n");
        return -1;
    }

    // get the memory address that will be used

    ptrTo = (unsigned long *)strtoul(argv[1], NULL, 0);

    for (lp = 2; lp < argc; lp++)
        ptrTo[lp - 2] = strtoul(argv[lp], NULL, 0);

    return 0;
}
