// This file contains the functions that process the arp protocol.
//
// modified by Yeonwoo Sung on 2017.01.17 ...


#include <pxa255.h>
#include <serial.h>
#include <time.h>
#include <gpio.h>
#include <stdio.h>
#include <string.h>

#include <cs8900.h>
#include <net.h>
#include <config.h>

extern int Cfg_parse_args(char *cmdline, char **argv);
extern Byte StrToByte(char *ptr, int hex);

static short __ICMP_SEQ = 1;

char *GetHostMacAddress(void) {
    return Cfg.Host_MacAddress;
}

Word32 GetLocalIP(void) {
    return Cfg.Local_IP;
}

Word32 GetHostIP(void) {
    return Cfg.Host_IP;
}

BOOL SendArpPacket(void) {

    Byte BroadcastMAC[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // broadcast the mac address
    ARP_Packet_t ARP_Packet;

    // initialise the memory variable
    memset(&ARP_Packet, 0, sizeof(ARP_Packet));

    // set the BOOTP area
    SetArpRequestHeader(&ARP_Packet, CS8900_GetMacAddress(), GetLocalIP(), GetHostIP());

    // set the ethernet packet information area
    SetEthernetARPHeader((EthernetIP_t *)&ARP_Packet, CS8900_GetMacAddress(), BroadcastMAC);

    // transmit the packet
    CS8900_Transmit((char *)&ARP_Packet, sizeof(ARP_Packet));
    return TRUE;
}


BOOL ARPReply(void *rxPktBuf, int len) {
    int lp;
    // 수신된 ARP 패켓의 이상 유무를 검사한다.
    // check if error occurred in the sent ARP packet
    if (CheckARPRxPacket(rxPktBuf, len, &(Cfg.Host_IP), &(Cfg.Local_IP)) == TRUE) {

        GetHostMacAddressFromArpPacket((ARP_Packet_t *)rxPktBuf, Cfg.Host_MacAddress);
        printf("ARP PACKET Resive\n");

        // print out the host mac address
        printf("HOST MAC : [ ");
        for (lp = 0; lp < 6; lp++)
            printf("%02X ", Cfg.Host_MacAddress[lp] & 0xFF);
        printf("]\n");
    } else {
        printf("ARP PACKET Error\n");
        return FALSE;
    }
    return TRUE;
}


BOOL ARPProcess(void) {
    char RxBuff[1500];
    Word16 RxSize;
    Word16 Protocol;
    int lp;

    for (lp = 0; lp < 10; lp++) {
        memset(RxBuff, 0, sizeof(RxBuff));
        printf("Send ARP Packet \n");
        SendArpPacket();

        // set 1-sec
        ReloadTimer(0, 1000);

        // check the time overflow
        while (0 == TimeOverflow(0)) {

            // check if the packet has been sent successfully
            RxSize = CS8900_Resive(RxBuff, 1500);
            if (RxSize) {
                // printf( "RESIVE OK [ size : %d ]\n", RxSize );
                Protocol = GetProtocolFromRxPacket((EthernetIP_t *)RxBuff, RxSize);

                // printf( "Ethernet Protocol Type : [%04X]\n", Protocol );
                if (Protocol == PROT_ARP) {
                    // printf( "Resive Protocol Type : [ARP]\n" );
                    if (ARPReply((EthernetIP_t *)RxBuff, RxSize) == TRUE) {
                        FreeTimer(0); // free the timer
                        return TRUE;
                    }
                }
            }
        }
    }

    FreeTimer(0); // free the timer
    return FALSE;
}


BOOL SendEchoPacket(void) {

    ECHO_Packet_t ECHO_Packet;
    //int lp;


    memset(&ECHO_Packet, 0, sizeof(ECHO_Packet));

    ECHO_Packet.ICMP.icmp_type = ECHO_REQUEST;
    ECHO_Packet.ICMP.icmp_code = 0;
    ECHO_Packet.ICMP.icmp_cksum = 0;
    ECHO_Packet.ICMP.icmp_id = SWAP16(8988);
    ECHO_Packet.ICMP.icmp_seq = __ICMP_SEQ++;

    ECHO_Packet.ICMP.icmp_cksum = SWAP16(GetCheckSumICMP((Word16 *)&(ECHO_Packet.ICMP), sizeof(ECHO_Packet.ICMP)));

    // set the IP information area
    SetIPHeaderByICMP(&ECHO_Packet.IP,
                      GetHostIP(),  // host IP
                      GetLocalIP(), // local IP
                      sizeof(ECHO_Packet.ICMP));


    // set the ethernet packet information area
    SetEthernetIPHeader((EthernetIP_t *)&ECHO_Packet, CS8900_GetMacAddress(), GetHostMacAddress());

    // transmit the packet
    CS8900_Transmit((char *)&ECHO_Packet, sizeof(ECHO_Packet));
    return TRUE;
}


BOOL ECHOHandler(void *rxPktBuf, int len) {

    if (CheckECHORxPacket(rxPktBuf, len, &(Cfg.Local_IP)) == TRUE) {
        return TRUE;
    }
    return FALSE;
}


BOOL EChoProcess(void) {
    char RxBuff[1500];
    Word16 RxSize;
    Word16 Protocol;
    int lp;


    // Generate and send Bootp Packet every 1 second, and get the IP address from the Reply.
    for (lp = 0; lp < 10; lp++) {
        memset(RxBuff, 0, sizeof(RxBuff));
        printf("Send Echo Packet \n");
        SendEchoPacket();

        // set 1-sec
        ReloadTimer(0, 1000);


        // check the time overflow
        while (0 == TimeOverflow(0)) {

            // check if the packet has been sent successfully
            RxSize = CS8900_Resive(RxBuff, 1500);

            if (RxSize) {
                // printf( "RESIVE OK [ size : %d ]\n", RxSize );
                Protocol = GetProtocolFromRxPacket((EthernetIP_t *)RxBuff, RxSize);

                // printf( "Ethernet Protocol Type : [%04X]\n", Protocol );
                if (Protocol == PROT_IP) {
                    // printf( "Resive Protocol Type : [IP]\n" );
                    if (ECHOHandler(RxBuff, RxSize) == TRUE) {
                        printf("PING TEST GOOD\n");
                        FreeTimer(0); // free the timer
                        return TRUE;
                    }
                }
            }
        }
    }

    FreeTimer(0); // free the timer
    printf("PING TEST FAIL\n");
    return FALSE;
}

BOOL AutoRebuildNetworkInfo(void) {
    if ((GetLocalIP() == 0) || (GetLocalIP() == 0xffffffff) || (GetHostIP() == 0) || (GetHostIP() == 0xffffffff)) {
        printf("- Invalid [LOCAL IP] or [HOST IP], Type [SET] command\n");
        return FALSE;
    }

    return ARPProcess();
}

int Arp(int argc, char **argv) {

    // do the ethernet link test
    if (CS8900_CheckLink() == FALSE) {
        printf("Ethernet Link failed. Check line.\n");
        return -1;
    }

    memset(Cfg.Host_MacAddress, 0, sizeof(Cfg.Host_MacAddress));
    ARPProcess();

    return TRUE;
}

int Ping(int argc, char **argv) {
    char *str[128];
    int adrcnt;
    Word32 TargetIP;
    Word32 OldHostIP;

    if (argc < 2) {
        printf("Error argument count\n");
        return -1;
    }

    // gets IP from the client
    adrcnt = Cfg_parse_args(argv[1], str);

    if (adrcnt == 4) {
        TargetIP = (StrToByte(str[3], 0) << 24) | (StrToByte(str[2], 0) << 16) | (StrToByte(str[1], 0) << 8) | (StrToByte(str[0], 0));
    } else {
        printf("Error IP Address\n");
        return -1;
    }

    OldHostIP = Cfg.Host_IP;
    Cfg.Host_IP = TargetIP;

    // check if the ethernet is linked
    if (CS8900_CheckLink() == FALSE) {
        printf("Ethernet Link failed. Check Line.\n");
        return -1;
    }

    if (AutoRebuildNetworkInfo() == FALSE)
        return FALSE;

    EChoProcess();

    Cfg.Host_IP = OldHostIP;

    return TRUE;
}
