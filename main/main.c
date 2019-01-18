// The main routine of the Blueberrian.
//
// modified by Yeonwoo Sung (2018-12-28)

#include <pxa255.h>
#include <serial.h>
#include <time.h>
#include <gpio.h>
#include <stdio.h>
#include <string.h>
#include <mem_map.h>
#include <config.h>
#include <cs8900.h>
#include <flash.h>
#include <main.h>


/* external functions */

extern int RamDump(int argc, char **argv);      // memory data dump
extern int RamWriteChar(int argc, char **argv); // write data to memory ( 1 Byte )
extern int RamWriteWord(int argc, char **argv); // write data to memory ( 2 Byte )
extern int RamWriteLong(int argc, char **argv); // write data to memory ( 3 Byte )

extern int Flash_Memory(int argc, char **argv); // write data to Flash

extern int ZModem_Memory(int argc, char **argv);    // download file by using ZMODEM protocol, and write it in the memory
extern int ZModem_FlashBoot(int argc, char **argv); // download file by using ZMODEM protocol, and write it in the flash
extern int ZModem_Kernel(int argc, char **argv);
extern int ZModem_RamDisk(int argc, char **argv);
extern unsigned ZModem_Baudrate;

extern int Arp(int argc, char **argv);  // get the HOST Mac Address by using Arp protocol
extern int Ping(int argc, char **argv); // Do the Ping test by using the ICMP protocol

extern int Tftp(int argc, char **argv);           // Get data by using Tftp protocol 
extern int Tftp_FlashBoot(int argc, char **argv); // Download the boot file by using the Tftp, and store it in the flash
extern int Tftp_Kernel(int argc, char **argv);    // Download the kernel file by using the Tftp protocol
extern int Tftp_RamDisk(int argc, char **argv);   // By using the Tftp protocol, download the Ram Disk image file
extern int Tftp_AppImage(int argc, char **argv);  // By using the Tftp protocol, download the AppImage file

extern int GoKernel(int argc, char **argv);   // Jump to kernel
extern int GoFunction(int argc, char **argv); // Jump to user function that is downloaded in the memory

extern int Help(int argc, char **argv); // help

extern int gets_his(char *s);
extern void MemoryHexDump(void *address, int size);
extern int getc_timed(char cmpKey, int mTimeOver);

/* internel functions */

int Soft_Reset(int argc, char **argv);

/* global variables */

extern TConfig Cfg; // defined in the config.c file


/**
 * Command list
 */
TCommand Cmds[] = {

    {"HELP", Help},
    {"?", Help},

    {"RST", Soft_Reset}, // main.c

    {"ARP", Arp}, // arp_cmd.c
    {"PING", Ping},

    {"GK", GoKernel}, // go_cmd.c
    {"GO", GoFunction},

    {"MD", RamDump}, // ram_cmd.c
    {"MWB", RamWriteChar},
    {"MWW", RamWriteWord},
    {"MWL", RamWriteLong},

    {"NEK", ErasePartiotion}, // flash_cmd.c
    {"NER", ErasePartiotion},
    {"NEA", ErasePartiotion},

    {"TMK", Tftp_Kernel}, // tftp_cmd.c
    {"TFK", Tftp_Kernel},
    {"TMR", Tftp_RamDisk},
    {"TFR", Tftp_RamDisk},
    {"TFB", Tftp_FlashBoot},
    {"TM", Tftp},
    {"TF", Tftp},

    {"SET", ModifyCfg}, // config.c

    {NULL, NULL}
};


/**
 * This function does the soft reset.
 *
 * @param (argc) the number of tokens.
 * @param (argv) the array that contains tokens.
 * @return On success, 0 will be returend. Otherwise, -1 will be returned.
 * @warning This function uses the watchdog timer to reboot.
 */
int Soft_Reset(int argc, char **argv) {
    printf("System Soft Reset.......\n");

    SetWatchdog(10); // 10msec

    return 0;
}

static const char *delim = " \f\n\r\t\v"; //delimeter for parse_args function

/**
 * This function tokenize the commands.
 *
 * @param (cmdline) The command line
 * @param (argv) The array that will contain the tokens
 * @return The number of tokens.
 */
static int parse_args(char *cmdline, char **argv) {
    char *tok;
    int argc = 0;

    argv[argc] = NULL;

    for (tok = strtok(cmdline, delim); tok; tok = strtok(NULL, delim)) {
        argv[argc++] = tok;
    }

    return argc;
}

/**
 * A simple function that controls the machine to blinke the LEDs.
 */
int LedBlink(void) {
    int delay = (Cfg.AutoBootWaitTime >= 2) ? 250 : 50;

    GPIO_SetLED(0, LED_ON);
    GPIO_SetLED(1, LED_OFF);
    GPIO_SetLED(2, LED_ON);
    GPIO_SetLED(3, LED_OFF);

    msleep(delay);

    GPIO_SetLED(0, LED_OFF);
    GPIO_SetLED(1, LED_ON);
    GPIO_SetLED(2, LED_OFF);
    GPIO_SetLED(3, LED_ON);

    msleep(delay);

    GPIO_SetLED(0, LED_OFF);
    GPIO_SetLED(1, LED_OFF);
    GPIO_SetLED(2, LED_OFF);
    GPIO_SetLED(3, LED_OFF);
}


int main(void) {
    int start_option = *(int *)(DEFAULT_RAM_KERNEL_ZERO_PAGE);
    char ReadBuffer[1024];
    int argc;
    char *argv[128];
    int cmdlp;

    // load the config
    LoadConfig();

    // init the timer
    TimerInit();

    // init the GPIO
    GPIOInit();

    // init the serial
    SerialInit(BAUD_115200);
    ZModem_Baudrate = 115200;

    // display the starting message
    printf("\n\n");
    printf("WELCOME EZBOOT.X5 V1.8...................for PXA255\n");
    printf("Program by You Young-chang, fooji (FALinux Co.,Ltd)\n");
    printf("Last Modify %s\n\n", __DATE__);

    // blink the LED
    LedBlink();

    // initialise the boot flash
    BootFlash_Init();
    printf("\n");

    // NAND flash initialisation
    NandFlash_Init();
    printf("\n");

    // CS8900 initilisation
    CS8900_Init();
    printf("\n");


    // If it is from eztiny, copoy all data in memory to flash.
    switch (start_option) {

        // [ezboot]
        case 0x0000:
            // Autoboot =======================
            printf("Quickly Autoboot [ENTER] / ");
            if (Cfg.BootMenuKey == ' ')
                printf("Goto BOOT-MENU press [space bar]");
            else
                printf("Goto BOOT-MENU press [%c]", Cfg.BootMenuKey);

            if (getc_timed(Cfg.BootMenuKey, Cfg.AutoBootWaitTime * 1000)) {
                printf("\n");
                CopyImage();
                GoKernel(1, NULL); //go to kernel image
            }
            break;

        // [eztiny]
        case 0x0001:
            SelfCopy_BootLoader();
            break;
    }

    printf("\n\n");


    // Boot propmt
    // wait for command
    while (1) {

        printf("BLUEBERRIAN>");

        memset(ReadBuffer, 0, sizeof(ReadBuffer));

        // wait until the user input the command
        gets_his(ReadBuffer);
        printf("\n");

        argc = parse_args(ReadBuffer, argv); //parse the command

        if (argc) { // check if the command contains more than 1 command tokens (to check invalid command)
            UpperStr(argv[0]);
            cmdlp = 0;
            //run all given commands
            while (Cmds[cmdlp].CmdStr) {
                if (strcmp(argv[0], Cmds[cmdlp].CmdStr) == 0) {
                    Cmds[cmdlp].func(argc, argv);
                    printf("\n");
                    break;
                }
                cmdlp++; //increase the cmdlp to run next command
            }
        }
    }
}
