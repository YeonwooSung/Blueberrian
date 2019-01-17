// This file contains the jump (go to) routines for ezboot board.
//
// modified by Yeonwoo Sung on 2018-01-08

#include <pxa255.h>
#include <stdio.h>
#include <string.h>

#include <config.h>
#include <mem_map.h>


extern TConfig Cfg; // defined in the config.c


int GoKernelSingle(void) {
    char buff[] = {0, 0, 0, 0, 0};
    void (*theKernel)(int zero, int arch);

    char kcmd[2048];
    int len;

    memset(kcmd, 0, 2048);
    len = GetKCmdStr(kcmd);

    printf("Starting kernel [MARCH %d]...\n", Cfg.Kernel_ArchNumber);

    memcpy((char *)DEFAULT_RAM_KERNEL_ZERO_PAGE, kcmd, 2048);

    theKernel = (void (*)(int, int))DEFAULT_RAM_KERNEL_START;
    theKernel((long)0, (long)Cfg.Kernel_ArchNumber);

    return 0;
}

int GoKernel(int argc, char **argv) {
    if (Cfg.Watchdog) {
        SetWatchdog(Cfg.Watchdog * 1000);
    }

    GoKernelSingle();
    return 0;
}

int GoFunction(int argc, char **argv) {
    Word32 go_addr;
    void (*theFunction)(void);

    if (argc < 1)
        return -1;

    go_addr = strtoul(argv[1], NULL, 0);

    theFunction = (void (*)(void))go_addr;
    theFunction();

    return 0;
}
