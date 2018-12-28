// This file contains the environment settings of the Blueberrian OS.
//
// author: Yeonwoo Sung (2018-12-28)

#ifndef BLUEBERRIAN_CONFIG_HEADER
#define BLUEBERRIAN_CONFIG_HEADER

#define CONFIG_MAGIC 0x89880017
#define CONFIG_CMD_MAGIC 0x20030702
#define DEFAULT_BOOT_WAIT 3
#define DEFAULT_ENTRY_KEY ' '
#define DEFAULT_SERIAL 2

#define DEFAULT_LOCAL_MAC { 0x00, 0xA2, 0x55, 0xF2, 0x26, 0x25 }
#define DEFAULT_LOCAL_IP (192 << 0) | (168 << 8) | (10 << 16) | (155 << 24)
#define DEFAULT_HOST_IP (192 << 0) | (168 << 8) | (10 << 16) | (24 << 24)

#define DEFAULT_TFTP_DIR ""
#define DEFAULT_KERNEL_FILENAME "zImage.x5"
#define DEFAULT_RAMDISK_FILENAME "ramdisk.x5-12M.gz"
#define DEFAULT_EZBOOT_FILENAME "ezboot.x5"

#define CMD_ARCH_NAME "EZ-X5"

#define DEFAULT_NAND_PAR_KERNEL_SZ 1
#define DEFAULT_NAND_PAR_RAMDISK_SZ 5
#define DEFAULT_NAND_PAR_APP_SZ 58

#define DEFAULT_KERNEL_COMMAND_A "initrd=0xa0800000,5M root=/dev/ram ramdisk=12288"
#define DEFAULT_KERNEL_COMMAND_B "console=ttyS02,115200"
#define DEFAULT_KERNEL_COMMAND_C ""

#define KCMD_BUFF_MAX 80
#define KCMD_COUNT 3

typedef struct {
    Word32 CMD_MagicNumber; // kernel command magic number
    char CMD_Tag[8];        // default kernel command line -> ""
    char cmd_A[KCMD_BUFF_MAX];
    char cmd_B[KCMD_BUFF_MAX];
    char cmd_C[KCMD_BUFF_MAX];
} __attribute__((packed)) TKernelCommandLine;


typedef struct {
	Word32	MagicNumber; 
	char    Buff[512]; // parameter string
        
} __attribute__ ((packed)) TParam;

/**
 * The struct that contains the basic config information of the Blueberrian.
 */
typedef struct SConfig {
    Word32 MagicNumber; // magic number that ensures the environment settings (0x8988000x)

    Word32 AutoBootWaitTime; // auto boot wating time (sec)

    char BootMenuKey; // basic boot loader key ' '
    char UseRamdisk;  // boolean to check if the RAM is used
    char Watchdog;    // watchdog
    char Rev1;        // reserved area

    Byte Local_MacAddress[8]; // board MAC Address  [xx:xx:xx:xx:xx:xx]
    Byte Host_MacAddress[8];  // host  MAC Address  [xx:xx:xx:xx:xx:xx]
    Word32 Local_IP;          // local IP  = 0
    Word32 Host_IP;           // host  IP  = 0

    char TFTP_Directory[64];       // name of TFTP directory        - default: ""
    char TFTP_zImageFileName[64];  // file name of the kernel image - default: "zImage"
    char TFTP_RamDiskFileName[64]; // file name of RAM disk image   - default: "ramdisk.gz"
    char TFTP_LoaderFileName[64];  // bootloader file name          - default: "ezboot_x5"

    Word32 SeriaNumber;       // serial port number that will be used
    Word32 Kernel_ArchNumber; // kernel architecture number - default: 303

    Word16 NandPar[4]; // NAND-Partition size

    TKernelCommandLine KCmd; // kernel command

} __attribute__((packed)) TConfig;


#ifndef CONFIG_VAR
extern TConfig Cfg;
#endif

extern void LoadConfig(void);
extern void SaveConfig(void);
extern int ModifyCfg(int argc, char **argv);
extern int GetKCmdStr(char *dest);

#endif // BLUEBERRIAN_CONFIG_HEADER
