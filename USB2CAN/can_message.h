#pragma once
#ifndef _CAN_MESSAGE_H_
#define _CAN_MESSAGE_H_

class CAN_MESSAGE {
    UINT32 id;   /* for Extended(29bits)/Standard(11bits) */
    ULONG timestamp;
    boolean bExtended;
    boolean bRTR;
    UINT8  len;
    UINT8  data[8];
    static const UINT32 MAX_EXTENDED_ID = ~(BIT31 | BIT30 | BIT29);
    static const UINT32 MAX_STANDARD_ID = BIT10 | BIT9 | BIT8 | 0xFF;
  public:
    CAN_MESSAGE(const UINT32 ID, const boolean bExtended, boolean bRTR) :
        id(ID), bExtended(bExtended), bRTR(bRTR), timestamp(0) {

        if (bExtended && id > MAX_EXTENDED_ID) {
            id = MAX_EXTENDED_ID;
            printf("WARNING! Extended ID over MAX VALUE, re-assign to MAX\n");
        } else if (!bExtended && id > MAX_STANDARD_ID) {
            id = MAX_STANDARD_ID;
            printf("WARNING! Standard ID over MAX VALUE, re-assign to MAX ID\n");
        }
        len = 0;
    }

    const UINT32 Id(void) {
        return id;
    }
    const boolean isExtended(void) {
        return bExtended;
    }
    const boolean isRTR(void) {
        return bRTR;
    }
    const UINT8 size(void) {
        return len;
    }
    void setExended(void) {
        bExtended = true;
    }
    void setStandard(void) {
        bExtended = false;
    }
    void setRTR(boolean val) {
        bRTR = val;
    }
    void setID(UINT32 id) {
        this->id = id;
    }
    void setLen(UINT32 val) {
        this->len = val;
    }
    void clone(UINT8* dst) {
        if (dst != NULL)
            for (int i = 0; i < len; i++) {
                *(dst + i) = data[i];
            }
    }

    UINT8 operator [] (int i) const {
        if (i >= 8) {
            static UINT8 t = 0xFF;
            printf("over index\n");
            return t;
        }
        return data[i];
    }

    const UINT8& operator [] (int i) {
        if (i >= 8) {
            static UINT8 t = 0xFF;
            printf("over index\n");
            return t;
        }
        return data[i];
    }

    void fillData(const UINT8 dat[], int size) {
        if (bRTR)
            return;

        len = (size > 8) ? 8 : size;
        for (int i = 0; i < size; i++) {
            data[i] = dat[i];
        }
    }
};

#endif
