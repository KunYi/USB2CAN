#include "stdafx.h"
#include "MCP2515.h"

MCP2515::MCP2515(FT_HANDLE handle) : spiHandle(handle) {
    /* reset 3 times*/
    for (int i = 0; i < 3; i++) {
        resetMCP2515();
        Sleep(10);
    }
    initMCP2515();
};

MCP2515::~MCP2515() {
}

boolean MCP2515::setBaudrate(void) {
    /*
    * FOsc:25MHz
    * settings for 125KBps
    *
    * for CNF1
    * BRP:(4+1)*2 = 10, So TQ=1/(FOsc/BRP): 1/2.5MHz
    * SJW:3TQ
    *
    * For CNF2
    * BTLMODE: Enabled
    * SAM: Enabled
    * PS1: (6+1) = 7 TQ
    * PropSeg: (5+1) = 6TQ
    *
    * For CNF3
    * SOF: Enabled
    * WAKFIL: disabled
    * PS2: (5+1) = 6TQ
    *
    * Total: 20TQ
    *   SyncSeg(1TQ) + PropSeg(7TQ) + PS1(6TQ) + PS2(6TQ)
    */
#if 1
    /* 125Kbps, 20TQ */
    RegWrite(REG_CNF1, 0x84);
    RegWrite(REG_CNF2, 0xF5);
    RegWrite(REG_CNF3, 0x85);
#else
    /* 125Kbps, 10TQ */
    RegWrite(REG_CNF1, 0x49);
    RegWrite(REG_CNF2, 0xD2);
    RegWrite(REG_CNF3, 0xC2);
#endif

#if 0
    /* 250Kbps, 10TQ*/
    RegWrite(spiHandle, REG_CNF1, 0x44);
    RegWrite(spiHandle, REG_CNF2, 0xD2);
    RegWrite(spiHandle, REG_CNF3, 0xC2);
    /* 250Kbps, 25TQ */
    RegWrite(spiHandle, REG_CNF1, 0xC1);
    RegWrite(spiHandle, REG_CNF2, 0xFF);
    RegWrite(spiHandle, REG_CNF3, 0xC7);
#endif

    return true;
}

boolean MCP2515::resetMCP2515(void) {
    uint8 buff[1];
    uint32 len;
    FT_STATUS ftStatus = FT_OK;

    buff[0] = CMD_RESET;
    ftStatus = SPI_Write(spiHandle, buff, 1, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 1)) {
        printf("Failed! send Reset command into MCP2515, len=%d\n", len);
        return false;
    }
    return true;
}

boolean MCP2515::initMCP2515(void) {
    if (!checkMode(STAT_CONFIGMODE)) {
        printf("FAILED!!!, Could not find a MCP2515\n");
        return false;
    }
    setBaudrate();
    setFilter();
    turnOffTxRTSpin();
    setMode(MODE_NORMAL);
    if (!checkMode(STAT_NORMALMODE)) {
        printf("FAILED!!!, operation MCP2515 back to Normal mode failed\n");
        return false;
    }
    setRXBnBF();
    RegWrite(REG_RXB0CTRL, 0);
    setInterrupt();

    return true;
}

UINT8 MCP2515::ReadRxStatus(void) {
    FT_STATUS ftStatus = FT_OK;
    uint8 buff[2] = { CMD_RXSTATUS, 0x00 };
    uint32 len;

    ftStatus = SPI_Write(spiHandle, buff, 1, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    if ((ftStatus != FT_OK) && (len != 1)) {
        printf("Failed! ReadRxStatus, len=%d for instruction stage\n", len);
        return false;
    }

    ftStatus = SPI_Read(spiHandle, buff, 2, &len,
                        SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                        SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 2)) {
        printf("Failed! ReadRxStatus, len=%d for read stage\n", len);
        return false;
    }
    return buff[0];
}

boolean MCP2515::RegRead(const UINT8 reg, UINT8& val) {
    FT_STATUS ftStatus = FT_OK;
    uint8 buff[2] = { CMD_READ, reg };
    uint32 len;
    val = 0xFF; // default error value
    ftStatus = SPI_Write(spiHandle, buff, 2, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    if ((ftStatus != FT_OK) && (len != 2)) {
        printf("Failed! Register Read, len=%d for instruction stage\n", len);
        return false;
    }

    ftStatus = SPI_Read(spiHandle, buff, 1, &len,
                        SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                        SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 1)) {
        printf("Failed! Register Read, len=%d for read stage\n", len);
        return false;
    }

    val = buff[0];
    return true;
}

boolean MCP2515::RegModify(const UINT8 reg, const UINT8 mask, const UINT8 val) {
    FT_STATUS ftStatus = FT_OK;
    uint8 buff[4] = { CMD_BITMOD, reg, mask, val};
    uint32 len = 4;
    ftStatus = SPI_Write(spiHandle, buff, 4, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 3)) {
        printf("Failed! Register Modify, len=%d\n", len);
        return false;
    }
    return true;
}

boolean MCP2515::RegRead2(const UINT8 reg, UINT8& val, UINT8& val1) {
    FT_STATUS ftStatus = FT_OK;
    uint8 buff[2] = { CMD_READ, reg };
    uint32 len;
    val = 0xFF; // default error value
    ftStatus = SPI_Write(spiHandle, buff, 2, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    if ((ftStatus != FT_OK) && (len != 2)) {
        printf("Failed! Register Read, len=%d for instruction stage\n", len);
        return false;
    }

    ftStatus = SPI_Read(spiHandle, buff, 1, &len,
                        SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                        SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 1)) {
        printf("Failed! Register Read, len=%d for read stage\n", len);
        return false;
    }

    val = buff[0];
    val1 = buff[1];
    return true;
}

boolean MCP2515::RegWrite(const UINT8 reg, const UINT8 val) {
    FT_STATUS ftStatus = FT_OK;
    uint8 buff[3] = { CMD_WRITE, reg, val };
    uint32 len = 3;
    ftStatus = SPI_Write(spiHandle, buff, 3, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 3)) {
        printf("Failed! Register Write, len=%d\n", len);
        return false;
    }
    return true;
}

boolean MCP2515::turnOffTxRTSpin(void) {
    return RegWrite(REG_TXRTSCTRL, 0x00);
}

boolean MCP2515::setRXBnBF(void) {
    /* To enabled RXnBF pin functions */
    return RegWrite(REG_BFPCTRL, 0x0F);
}

boolean MCP2515::setInterrupt(void) {
    boolean ret = true;
    if (!RegWrite(REG_CANINTE, (FLG_MERRF | FLG_ERRIF | FLG_RX1IF | FLG_RX0IF | FLG_TX0IF)))
        ret = false;

    if (!RegWrite(REG_CANINTF, (0x00)))
        ret = false;

    return ret;
}

boolean MCP2515::TXB0RTS(void) {
    uint8 buff[1] = { CMD_RTS(0) };
    uint32 len;

    FT_STATUS ftStatus = FT_OK;
    ftStatus = SPI_Write(spiHandle, buff, 1, &len,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (len != 1)) {
        printf("Failed! send TXnRTS into MCP2515, len=%d\n", len);
        return false;
    }
    return true;
}

boolean MCP2515::loadTx0Buf(const UINT8 buf[], UINT32 len) {
    uint8 buff[14] = { 0 };
    uint32 rlen;
    FT_STATUS ftStatus = FT_OK;

    if (len >= 14) {
        printf("Faild: send length %d, should lessthen 14\n", len);
        return false;
    }

    buff[0] = CMD_LDTXB(0); /* START FROM TXB0SIDH */
    memcpy(&buff[1], buf, len);

    ftStatus = SPI_Write(spiHandle, buff, len + 1, &rlen,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);

    if ((ftStatus != FT_OK) && (len != rlen)) {
        printf("Failed! Load data into TX0BUFF of MCP2515, len=%d, rlen=%d\n", len, rlen);
        return false;
    }
    return true;
}

boolean MCP2515::ReadRX0Buf(UINT8 buf[], const UINT32 len) {
    FT_STATUS ftStatus = FT_OK;
    uint32 rlen;

    if ((len < 13) && (buf != NULL)) {
        printf("FAILED, Buffer size >= 13\n");
        return false;
    }

    buf[0] = CMD_READRXB(0);
    ftStatus = SPI_Write(spiHandle, buf, 1, &rlen,
                         SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                         SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE);
    if ((ftStatus != FT_OK) && (rlen != 1)) {
        printf("Failed! Read RXB0, len=%d for instruction stage\n", rlen);
        return false;
    }

    ftStatus = SPI_Read(spiHandle, buf, 13, &rlen,
                        SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
                        SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE);
    if ((ftStatus != FT_OK) && (rlen != 13)) {
        printf("Failed! Read RXB0, len=%d for read stage\n", rlen);
        return false;
    }

    return true;
}

boolean MCP2515::checkTxError(void) {
    UINT8 val;
    UINT i = 0;
    do {
        RegRead(REG_TXB0CTRL, val);
        if (val == 0) {
            return true;
        }

        if (i++ > 50) {
            // over 500 milisecond
            return false;
        }

        if (val & STAT_TX_ABTF) {
            printf("FAILED! Aborted sending message\n");
            break;
        }
        if (val & STAT_TX_MLOA) {
            printf("FAILED! Lost arbitration sending message\n");
            break;
        }
        if (val & STAT_TX_ERR) {
            printf("FAILED! Error for sending message. TX_ERR indicat\n");
            break;
        }
        if (val & STAT_TX_REQ) {
            printf("Still TX Req\n");
        }
        Sleep(10);
    } while (true);

    RegWrite(REG_TXB0CTRL, 0); // clean all error
    return false;
}

UINT8 MCP2515::getMode(void) {
    UINT8 val;
    if (RegRead(REG_CANSTAT, val)) {
        val &= STAT_MODEMASK;
    }
    return val;
}

boolean MCP2515::setMode(const UINT8 mode) {
    return RegWrite(REG_CANCTRL, mode);
}

boolean MCP2515::checkMode(const UINT8 mode) {
    for (int i = 0; i < 10; i++ ) {
        if (getMode() == mode)
            return true;

        Sleep(20);
    }

    return false;
}

boolean MCP2515::setFilter(void) {
    /* disable filter to accept all message */
    boolean ret = true;
    if (!RegWrite(REG_RXB0CTRL, 0))
        ret = false;
    if (!RegWrite(REG_RXB1CTRL, 0))
        ret = false;

    // we just accept 0x123456FF & 0x123456FE
    const UINT32 fEID = 0x123456FE;
    RegWrite(REG_RXF0SIDH, (fEID >> 21) & 0xff);
    RegWrite(REG_RXF0SIDL, ((fEID >> 13) & 0xE0 |
                            (BIT3) | (fEID>>16) & 0x03));
    RegWrite(REG_RXF0EID8, (fEID >> 8) & 0xFF);
    RegWrite(REG_RXF0EID0, fEID & 0xFF);
    RegWrite(REG_RXM0SIDH, (fEID >> 21) & 0xff);
    RegWrite(REG_RXM0SIDL, ((fEID >> 13) & 0xE0 |
                            (fEID >> 16) & 0x03));
    RegWrite(REG_RXM0EID8, (fEID >> 8) & 0xFF);
    RegWrite(REG_RXM0EID0, fEID & 0xFF);

    return ret;
}

boolean MCP2515::send(CAN_MESSAGE & msg) {
    UINT8 TXB[13] = { 0 };
    const UINT32 id = msg.Id();
    /* fill ID*/
    if (msg.isExtended()) {
        /* Fill Extended ID */
        TXB[0] = (id >> 21) & 0xFF;		/* EID 28..21*/
        TXB[1] = ((id >> 13) & 0xE0) |	/* EID20 .. EID18 */
                 BIT3 | /* EXIDE BIT*/
                 ((id >> 16) & 0x03);			/* EID17 .. EID16 */
        TXB[2] = ((id >> 8) & 0xFF);		/* EID15 .. EID8 */
        TXB[3] = (id & 0xFF);			/* EID7 .. EID0 */
    } else {
        TXB[0] = (id >> 3) & 0xFF;
        TXB[1] = (id << 5) & 0xE0;
        TXB[2] = 0;
        TXB[3] = 0;
    }

    TXB[4] = msg.size();
    if (msg.isRTR()) {
        TXB[4] = BIT6; // for TXBnDLC
    } else {
        msg.clone(&TXB[5]);
    }
    loadTx0Buf(TXB, msg.size() + 5);
    RegWrite(REG_CANINTF, 0);
    RegModify(REG_TXB0CTRL, BIT0 | BIT1, BIT0 | BIT1);  // Highest Priority
    TXB0RTS();
    UINT8 flags;
    do {
        RegRead(REG_CANINTF, flags);
        if (flags & FLG_TX0IF)
            break;
        Sleep(10);
    } while (true);
    RegModify(REG_CANINTF, FLG_TX0IF, 0);
    return true;
}

boolean MCP2515::recv(CAN_MESSAGE & msg, const UINT32 milliseconds) {
    UINT8 flags;
    UINT8 RXB[13] = { 0 };
    UINT32 rID = 0;

    UINT32 i = 0;
    while (true) {
        flags = ReadRxStatus();
        UINT8 bMessage = ((flags >> 6) & (BIT1 | BIT0));
        if (bMessage) {
            if (flags & BIT6) {
                // in RXB0
                (flags & BIT4) ? msg.setExended() :
                msg.setStandard();
                msg.setRTR((flags & BIT3) ? true : false);
                ReadRX0Buf(RXB, sizeof(RXB));
                rID = 0;
                if (msg.isExtended()) {
                    rID |= RXB[0] << 21;
                    rID |= ((RXB[1] & 0xE0) << 13);
                    rID |= ((RXB[1] & 0x3) << 16);
                    rID |= RXB[2] << 8;
                    rID |= RXB[3];
                } else {
                    rID |= RXB[0] << 3;
                    rID |= (RXB[1] >> 5) & 0x07;
                }
                msg.setID(rID);
                msg.setLen(RXB[4] & 0x0F);
                msg.fillData(&RXB[5], msg.size());
                break;
            }
            if (flags & BIT7) {
                // in RXB1
                break;
            }
        }
        Sleep(10);
        if ((i += 10) >= milliseconds)
            break;
    }

    RegModify(REG_CANINTF, FLG_RX0IF | FLG_RX1IF, 0);
    return true;
}
