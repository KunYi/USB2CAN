/* USB2CAN.cpp : Defines the entry point for the console application. */

#include "stdafx.h"

// #pragma comment(lib, "ftd2xx")
#pragma comment(lib, "libMPSSE")

#define DBG		(true)

int main() {
    FT_STATUS ftStatus = FT_OK;
    ChannelConfig channelConf = { 0 };
    FT_HANDLE spiHandle;
    uint32 numChannel;

    const uint32 spiClock = 1000000; /* 1 MHz */
    const uint8	latency = 255;
    const uint32 spiModeConfig = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    const uint32 pinNormalClose = ((BIT5 | BIT4 | BIT3 | BIT1 | BIT0) |           /* InitDir (for dir 0=in, 1=out)*/
                                   ((BIT4 | BIT3 | BIT1) << 8) |                  /* InitVal	*/
                                   ((BIT5 | BIT4 | BIT3 | BIT1 | BIT0) << 16) |   /* FinalDir	*/
                                   ((BIT4 | BIT3 | BIT1 | BIT0) << 24));          /* FinalVal */

    channelConf.ClockRate = spiClock;
    channelConf.LatencyTimer = latency;
    channelConf.configOptions = spiModeConfig;
    channelConf.Pin = pinNormalClose;

    /* init library */
    Init_libMPSSE();
    printf("USB-SPI-CAN Test Fixture\n");
    printf("The program for FTDI 2232 only\n");
    ftStatus = SPI_GetNumChannels(&numChannel);
    if (ftStatus == FT_OK)
        printf("Number of Channel is %d\n", numChannel);
    if (numChannel >= 2) {
        /* to open BDBUS */
        if ((ftStatus = SPI_OpenChannel(1, &spiHandle)) == FT_OK)
            printf("SPI_OpenChannel to Open B Channel okay\n");
        if ((ftStatus = SPI_InitChannel(spiHandle, &channelConf)) == FT_OK)
            printf("initial ChannelB SPI okay!\n");

        /* initial BCBUS*/
        FT_WriteGPIO(spiHandle, BIT6 | BIT2 | BIT1 | BIT0, (BIT2 | BIT1 |BIT0));
        Sleep(50);

        /* Pull reset to high */
        FT_WriteGPIO(spiHandle, BIT6 | BIT2 | BIT1 | BIT0, (BIT6 | BIT2 | BIT1 | BIT0));

        MCP2515 can(spiHandle);
        CAN_MESSAGE canmsg(0x123456FE, true, true);
        printf("send RTR frame\n");
        can.send(canmsg);

        CAN_MESSAGE rmsg(0, true, false);
        can.recv(rmsg, 200);
        printf("Receive RX message ID:0x%08X, Data:", rmsg.Id());
        for (int i = 0; i < rmsg.size(); i++)
            printf(" 0x%02X", rmsg[i]);
        printf("\n");

        if ((ftStatus = SPI_CloseChannel(spiHandle)) == FT_OK)
            printf("SPI_CloseChannel success\n");
    }
    Cleanup_libMPSSE();
    return 0;
}

