// This file contains the routines that uses the TFTP to get file data.
//
// modified by Yeonwoo Sung on 2018.12.29

#include <pxa255.h>
#include <serial.h>
#include <time.h>
#include <gpio.h>
#include <stdio.h>
#include <string.h>

#include <mem_map.h>
#include <cs8900.h>
#include <net.h>
#include <config.h>
#include <flash.h>


Word16 __TFTPLastBlockNumber = 0;             // 마지막으로 수신한 블럭 번호
Word16 __TFTPHostPortNumber = TFTP_PORT_HOST; // 호스트의 포트 번호
Word32 __TFTPResiveTotalSize = 0;             // 수신된 총 크기

// defined in the config.c file
extern TConfig Cfg;


/**
 * Send the TFTP packet.
 *
 * @param (ptrTFTP) packet address of transmitted packet
 * @param (HostPortNumber) the host port number
 * @param (tftpsize) The size of the TFTP data - tftpsize is not the size of entire transmitted frame
 * @return TRUE
 */
BOOL SendTFTPPacket(TFTP_Packet_t *ptrTFTP, Word16 HostPortNumber, int tftpsize) {

    // set the UDP data area
    SetUdpHeader(&(ptrTFTP->UDP), HostPortNumber, TFTP_PORT_CLIENT, tftpsize);

    // set the IP data area
    SetIPHeaderByUDP(&(ptrTFTP->IP),
                     GetHostIP(),  // Host IP
                     GetLocalIP(), // Local IP
                     sizeof(ptrTFTP->UDP) + tftpsize);

    // set the ethernet packet header
    SetEthernetIPHeader(&(ptrTFTP->ETHERNET), CS8900_GetMacAddress(), GetHostMacAddress());

    // transmit the packet
    CS8900_Transmit(ptrTFTP, sizeof(ptrTFTP->ETHERNET) + sizeof(ptrTFTP->IP) + sizeof(ptrTFTP->UDP) + tftpsize);

    return TRUE;
}


/**
 * Process the TFTP response packet.
 *
 * @param (ptrTFTP) the address of the TFTP packet buffer that stores the response packets.
 * @param (len) the length of the sent packet.
 * @param (LoadAddr) the load address
 * @return received data
 */
int TftpResiveHandler(TFTP_Packet_t *ptrTFTP, int len, Word32 LoadAddr) {
    Word16 *ptrBlockNumber;
    Word16 *ptrErrorNumber;
    char *SrcDataStr;  // data address of packet
    char *DestDataStr; // destination address
    int DataSize;
    Word16 Opcode;
    int lp, opcnt;

    DataSize = len - sizeof(TFTP_Packet_t) - 2;

    ptrBlockNumber = (Word16 *)ptrTFTP->Data;
    ptrErrorNumber = (Word16 *)ptrTFTP->Data;

    SrcDataStr = ptrTFTP->Data + 2; // get the data part

    Opcode = SWAP16(ptrTFTP->OPCODE);


    switch (Opcode) {

        //ignore the data that will not be processed
        case TFTP_RRQ:

        case TFTP_WRQ:
        case TFTP_ACK:
            break;

        case TFTP_ERROR: // process error
            printf("\nTFTP Error : [ %d ] %s \n", SWAP16(*ptrErrorNumber), SrcDataStr);

            return TFTP_STATE_FAILURE;
        case TFTP_DATA: // process received data

            // check if the size of the block is 0
            if (SWAP16(*ptrBlockNumber) == 0) {
                printf("\nTFTP Error : Block Number Zero\n");
                return TFTP_STATE_FAILURE;
            }

            // check the data block number
            // process the data blocks, whose block number is big enough
            if (__TFTPLastBlockNumber < SWAP16(*ptrBlockNumber)) {
                __TFTPLastBlockNumber = SWAP16(*ptrBlockNumber);
                // calculate the required memory
                DestDataStr = (char *)(LoadAddr + ((__TFTPLastBlockNumber - 1) * 512));
                // copy the memory
                memcpy(DestDataStr, SrcDataStr, DataSize);

                __TFTPResiveTotalSize += DataSize;
            }

            // if the data size is not 512, then we received all data
            if (DataSize < 512)
                return TFTP_STATE_END;
            return TFTP_STATE_DATA_OK;

        case TFTP_OACK: //process the data with option
            opcnt = 0;
            printf("OPTION [");
            for (lp = 0; lp < 64; lp++) {
                if (ptrTFTP->Data[lp] == 0) {
                    opcnt++;
                    if (opcnt == 4)
                        break;
                    printf(" ");
                } else {
                    printf("%c", ptrTFTP->Data[lp]);
                }
            }
            printf("]\n");
            return TFTP_STATE_DATA_OK;
    }

    return TFTP_NONE;
}


/**
 * Gets the file data by using BOOTP.
 *
 * @param (mHostIP) host IP that will be accessed
 * @param (mLocalIP) board IP
 * @param (LoadAddr) memory address of destination
 * @return On success, returns 0. Otherwise, returns -1.
 */
BOOL TftpProcess(Word32 mHostIP, Word32 mLocalIP, Word32 LoadAddr, char *FileName) {

    Word32 IPAddr;
    char TxBuff[1500];
    char RxBuff[1500];
    Word16 RxSize;
    Word16 Protocol;

    TFTP_Packet_t *ptrTFTP;
    int size;

    //CS8900_SwReset();

    if (AutoRebuildNetworkInfo() == FALSE) {
        printf("- Fail Network Infomation\n");
        return FALSE;
    }

    // print the host IP and local IP
    IPAddr = SWAP32(mHostIP);
    printf("HOST  IP : [%d.%d.%d.%d]\n", (IPAddr >> 24) & 0xff,
           (IPAddr >> 16) & 0xff,
           (IPAddr >> 8) & 0xff,
           (IPAddr >> 0) & 0xff);

    IPAddr = SWAP32(mLocalIP);
    printf("LOCAL IP : [%d.%d.%d.%d]\n", (IPAddr >> 24) & 0xff,
           (IPAddr >> 16) & 0xff,
           (IPAddr >> 8) & 0xff,
           (IPAddr >> 0) & 0xff);

    // print the resive address
    printf("Resive Address  : %04X-%04X\n", (LoadAddr >> 16) & 0xFFFF, LoadAddr & 0xFFFF);

    printf("TFTP Request Send\n");

    // initialise the variables for memory
    memset(TxBuff, 0, sizeof(TxBuff));
    ptrTFTP = (TFTP_Packet_t *)TxBuff;

    size = SetTFTP_RRQPacket(ptrTFTP, FileName);
    SendTFTPPacket(ptrTFTP, TFTP_PORT_HOST, size);

    __TFTPLastBlockNumber = 0; // initialise the block number
    __TFTPResiveTotalSize = 0; // total received size

    ReloadTimer(0, 5000); // timer 0    5sec

    while (1) {
        // test the packet receiving
        RxSize = CS8900_Resive(RxBuff, 1500);
        if (RxSize) {
            printf( "RESIVE OK [ size : %d ]\n", RxSize );
            Protocol = GetProtocolFromRxPacket((EthernetIP_t *)RxBuff, RxSize);

            printf( "Ethernet Protocol Type : [%04X]\n", Protocol );

            if (Protocol == PROT_ARP) {
                ARP_Packet_t *pARP = (ARP_Packet_t *)RxBuff;
                if (SWAP16(pARP->ar_op) == ARPOP_REQUEST) {
                    ARP_Packet_t arp_pk;
                    Word32 local_ip = Cfg.Local_IP;
                    Word32 host_ip = Cfg.Host_IP;

                    memset(&arp_pk, 0, sizeof(ARP_Packet_t));
                    arp_pk.ar_hrd = SWAP16(HWT_ETHER);
                    arp_pk.ar_pro = SWAP16(PROT_IP);
                    arp_pk.ar_hln = 6;
                    arp_pk.ar_pln = 4;
                    arp_pk.ar_op = SWAP16(ARPOP_REPLY); // Operation
                    arp_pk.ar_hrd = SWAP16(HWT_ETHER);
                    memcpy(arp_pk.SenderMac, CS8900_GetMacAddress(), sizeof(arp_pk.SenderMac));
                    memcpy(arp_pk.TargetMac, pARP->SenderMac, sizeof(arp_pk.TargetMac));
                    arp_pk.SenderIP = local_ip;
                    arp_pk.TargetIP = host_ip;

                    memcpy((char *)arp_pk.ETHERNET.et_dest, pARP->SenderMac, sizeof(arp_pk.ETHERNET.et_dest));
                    memcpy((char *)arp_pk.ETHERNET.et_src, CS8900_GetMacAddress(), sizeof(arp_pk.ETHERNET.et_src));
                    arp_pk.ETHERNET.et_protocol = SWAP16(PROT_ARP);
                    CS8900_Transmit((char *)&arp_pk, sizeof(ARP_Packet_t));
                }

                continue;
            }

            if (Protocol != PROT_IP)
                continue;

            printf( "Resive Protocol Type : [IP]\n" );
            if (CheckTftpRxPacket((TFTP_Packet_t *)RxBuff, RxSize, &mLocalIP)) {
                __TFTPHostPortNumber = GetHostPortNumberFromTftpPacket((TFTP_Packet_t *)RxBuff);
                printf( "Resive TFTP Packet [ Host Port : %d ] \n", __TFTPHostPortNumber );
                // get the host port number

                // processing
                switch (TftpResiveHandler((TFTP_Packet_t *)RxBuff, RxSize, LoadAddr)) {
                    case TFTP_STATE_DATA_OK: // data received

                        size = SetTFTP_ACKDATAPacket(ptrTFTP, __TFTPLastBlockNumber);
                        SendTFTPPacket(ptrTFTP, __TFTPHostPortNumber, size);

                        // printf( "DATA ACK [ Block Number : %d ]\n", __TFTPLastBlockNumber );
                        if ((__TFTPLastBlockNumber % 10) == 0)
                            printf("\rSize:%d KB", __TFTPResiveTotalSize / 1024);

                        ReloadTimer(0, 5000);
                        continue;

                    case TFTP_STATE_DATA_FAILE: // data error occurred
                        printf("DATA FAIL\n");
                        break;

                    case TFTP_STATE_END: // received all data
                        size = SetTFTP_ACKDATAPacket(ptrTFTP, __TFTPLastBlockNumber);
                        SendTFTPPacket(ptrTFTP, __TFTPHostPortNumber, size);

                        printf("\rALL DATA RESIVE OK [ %d bytes ]\n", __TFTPResiveTotalSize);

                        FreeTimer(0);

                        return TRUE;
                }
            }
        }

        // check the time overflow
        if (TimeOverflow(0)) {
            printf("Time Overflow\n");
            break;
        }
    }

    FreeTimer(0); //free the timer
    return FALSE;
}


/**
 * Get file data by using TFTP.
 *
 * @param (argc) the number of tokens
 * @param (argv) token array
 * @return If error occurs, returns -1. Otherwise, returns 0.
 */
int Tftp(int argc, char **argv) {
    unsigned long To;
    char *Filename;

    printf("tftp Command\n");
    if (argc < 3) {
        printf("Argument Count Error!\n");
        return -1;
    }

    // get the file name
    Filename = argv[2];

    // get the destination address
    To = strtoul(argv[1], NULL, 0); //TODO did not do the validation for the returned address

    // get data via TFTP
    if (TftpProcess(GetHostIP(), GetLocalIP(), DEFAULT_RAM_WORK_START, Filename) == TRUE) {

        // use flash to record data
        if ('F' == argv[0][1]) {
            To &= 0x0fffffff;
            Flash_WriteFull( To, DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize );
        } else {
            memcpy((void *)To, (void *)DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
        }
    }

    return 0;
}


/**
 * Get data via TFTP, and write to boot.
 *
 * @param (argc) the number of tokens
 * @param (argv) the array that the tokens will be contained
 * @return If error occurs, returns -1. Otherwise, returns 0.
 */
int Tftp_FlashBoot(int argc, char **argv) {
    int size = 0;
    char FileName[256];

    // get the file name of bootloader
    strcpy(FileName, Cfg.TFTP_Directory);
    strcat(FileName, Cfg.TFTP_LoaderFileName);
    printf("Receive %s\n", FileName);

    // get data via TFTP
    if (TftpProcess(GetHostIP(), GetLocalIP(), DEFAULT_RAM_WORK_START, FileName) == TRUE) {
        CopyTo_BootFlash(DEFAULT_FLASH_BOOT, DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
    }

    return size;
}


/**
 * Get data via TFTP, and write the received data into kernel.
 *
 * @param (argc) the number of tokens
 * @param (argv) the array that the tokens will be stored in.
 * @return If there is no error, 0 might be returned. Otherwise, returns -1.
 */
int Tftp_Kernel(int argc, char **argv) {
    char FileName[256];

    // file name of kernel image
    strcpy(FileName, Cfg.TFTP_Directory);
    strcat(FileName, Cfg.TFTP_zImageFileName);
    printf("Receive %s\n", FileName);

    // get data via TFTP
    if (TftpProcess(GetHostIP(), GetLocalIP(), DEFAULT_RAM_WORK_START, FileName) == TRUE) {

        // use the flash to record data
        if ('F' == argv[0][1]) {
            CopyTo_NandFlash_Kernel(DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
        } else {
            memcpy((void *)(DEFAULT_RAM_KERNEL_START), (void *)DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
        }
    }

    return 0;
}


/**
 * Gets data via TFTP, and store it in the RAM disk.
 *
 * @param (argc) the number of tokens
 * @param (argv) the array that the tokens will be stored in
 * @return If error occurs, returns -1. Otherwise, returns 0.
 */
int Tftp_RamDisk(int argc, char **argv) {
    char FileName[256];

    // file name of the RAM DISK image
    strcpy(FileName, Cfg.TFTP_Directory);
    strcat(FileName, Cfg.TFTP_RamDiskFileName);
    printf("Receive %s\n", FileName);

    // get data via TFTP
    if (TftpProcess(GetHostIP(), GetLocalIP(), DEFAULT_RAM_WORK_START, FileName) == TRUE) {
        //use the flash memory to record data
        if ('F' == argv[0][1]) {
            CopyTo_NandFlash_Ramdisk(DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
        } else {
            memcpy((void *)(DEFAULT_RAM_RAMDISK_START), (void *)DEFAULT_RAM_WORK_START, __TFTPResiveTotalSize);
        }
    }

    return 0;
}
