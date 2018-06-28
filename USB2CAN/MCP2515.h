#pragma once


class MCP2515 {
  public:
    ~MCP2515();
    MCP2515(const FT_HANDLE handle);
    // we just support fixed baudrate */
    boolean setBaudrate(void);
    boolean setMode(const UINT8 mode);
    UINT8 getMode(void);
    boolean checkMode(const UINT8 mode);
    boolean setFilter(void);
    boolean send(CAN_MESSAGE& msg);
    boolean recv(CAN_MESSAGE& msg,const UINT32 milliseconds);
    boolean resetMCP2515(void);
    boolean initMCP2515(void);

  private:
    static const UINT8 CMD_RESET = BIT7 | BIT6;
    static const UINT8 CMD_READ = BIT1 | BIT0;
    static const UINT8 CMD_WRITE = BIT1;
    static const UINT8 CMD_BITMOD = BIT2 | BIT0;
    static const UINT8 CMD_RTS(UINT8 ch) {
        return BIT7 | (1 << ch) & 0x07;
    }
    static const UINT8 CMD_STATUS = BIT7 | BIT5;
    static const UINT8 CMD_RXSTATUS = BIT7 | BIT5 | BIT4;
    static const UINT8 CMD_READRXB(const UINT8 n) {
        return (n == 0) ? (BIT7 | BIT4) : (BIT7 | BIT4 | BIT2) ;
    }

    static const UINT8 CMD_LDTXB(UINT8 ch) {
        UINT8 val = (1<<ch);
        val = ((ch == 0) || (ch > 2)) ? 0 : val;
        return BIT6 | val;
    }

    static const UINT8 REG_TXRTSCTRL = 0x0D;
    /* REG CANSTAT */
    static const UINT8 REG_CANSTAT = 0x0E;
    static const UINT8 STAT_MODEMASK = 0xE0;
    static const UINT8 STAT_NORMALMODE = 0x00;
    static const UINT8 STAT_SLEEPMODE = 0x20;
    static const UINT8 STAT_LOOPBACKMODE = 0x40;
    static const UINT8 STAT_LISTENMODE	= 0x60;
    static const UINT8 STAT_CONFIGMODE = 0x80;
    /* REG CANCTRL */
    static const UINT8 REG_CANCTRL = 0x0F;
    static const UINT8 MODE_NORMAL = 0x08;
    static const UINT8 MODE_CONFIG = 0x80;
    /* REG CNF1..CNF3*/
    static const UINT8 REG_CNF1 = 0x2A;
    static const UINT8 REG_CNF2 = 0x29;
    static const UINT8 REG_CNF3 = 0x28;

    static const UINT8 REG_RXB0CTRL = 0x60;
    static const UINT8 REG_RXB1CTRL = 0x70;
    static const UINT8 REG_TXB0CTRL = 0x30;
    static const UINT8 REG_TXB1CTRL = 0x40;
    static const UINT8 REG_TXB2CTRL = 0x50;
    static const UINT8 STAT_TX_ABTF = BIT6;
    static const UINT8 STAT_TX_MLOA = BIT5;
    static const UINT8 STAT_TX_ERR	= BIT4;
    static const UINT8 STAT_TX_REQ	= BIT3;
    static const UINT8 TX_PRIO_MASK = (BIT1 | BIT0);

    static const UINT8 REG_BFPCTRL	= 0x0C;
    static const UINT8 REG_TXB0SIDH = 0x31;
    static const UINT8 REG_TXB0SIDL = 0x32;
    static const UINT8 REG_TXB0EID8 = 0x33;
    static const UINT8 REG_TXB0EID0 = 0x34;
    static const UINT8 REG_TXB0DLC	= 0x35;
    static const UINT8 REG_TXB0DATA = 0x36;

    static const UINT8 REG_CANINTE	= 0x2B;
    static const UINT8 REG_CANINTF	= 0x2C;
    static const UINT8 REG_EFLG		= 0x2D;
    static const UINT8 FLG_RX0IF	= BIT0;
    static const UINT8 FLG_RX1IF	= BIT1;
    static const UINT8 FLG_TX0IF	= BIT2;
    static const UINT8 FLG_TX1IF	= BIT3;
    static const UINT8 FLG_TX2IF	= BIT4;
    static const UINT8 FLG_ERRIF	= BIT5;
    static const UINT8 FLG_WAKIF	= BIT6;
    static const UINT8 FLG_MERRF	= BIT7;


    /* RXFn */
    static const UINT8 REG_RXF0SIDH	= 0x00;
    static const UINT8 REG_RXF0SIDL	= 0x01;
    static const UINT8 REG_RXF0EID8	= 0x02;
    static const UINT8 REG_RXF0EID0 = 0x03;
    static const UINT8 REG_RXF1SIDH = 0x04;
    static const UINT8 REG_RXF1SIDL = 0x05;
    static const UINT8 REG_RXF1EID8 = 0x06;
    static const UINT8 REG_RXF1EID0 = 0x07;
    /* RXMn */
    static const UINT8 REG_RXM0SIDH = 0x20;
    static const UINT8 REG_RXM0SIDL = 0x21;
    static const UINT8 REG_RXM0EID8 = 0x22;
    static const UINT8 REG_RXM0EID0 = 0x23;
    static const UINT8 REG_RXM1SIDH = 0x24;
    static const UINT8 REG_RXM1SIDL = 0x25;
    static const UINT8 REG_RXM1EID8 = 0x26;
    static const UINT8 REG_RXM1EID0 = 0x27;

    const FT_HANDLE spiHandle;
    UINT8	ReadRxStatus(void);
    boolean RegRead(const UINT8 reg, UINT8& val);
    boolean RegModify(const UINT8 reg, const UINT8 mask, const UINT8 val);
    boolean RegRead2(const UINT8 reg, UINT8& val, UINT8& val1);
    boolean RegWrite(const UINT8 reg, const UINT8 val);
    boolean turnOffTxRTSpin(void);
    boolean setRXBnBF(void);
    boolean setInterrupt(void);
    boolean TXB0RTS(void);
    boolean loadTx0Buf(const UINT8 buf[], UINT32 len);
    boolean ReadRX0Buf(UINT8 buf[], const UINT32 len);
    boolean checkTxError(void);
};

