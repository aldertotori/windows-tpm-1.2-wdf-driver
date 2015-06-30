
#ifndef _CMDBUF_H_
#define _CMDBUF_H_

#ifndef BYTE
#define BYTE	UCHAR
#endif

#ifndef UINT16
#define UINT16	USHORT
#endif

#ifndef TSS_BOOL
#define TSS_BOOL BYTE
#endif

//--------------------------------------------------------------------
// definitions for TSS Service Provider (TSP)
//
typedef  UINT32  TSS_HANDLE;

typedef  UINT32  TSS_FLAG;  // object attributes
typedef  UINT32  TSS_RESULT;  // the return code from a TSS function

typedef  UINT32          TSS_HOBJECT;     // basic object handle
typedef  TSS_HOBJECT     TSS_HCONTEXT;    // context object handle
typedef  TSS_HOBJECT     TSS_HPOLICY;     // policy object handle
typedef  TSS_HOBJECT     TSS_HTPM;        // TPM object handle
typedef  TSS_HOBJECT     TSS_HKEY;        // key object handle
typedef  TSS_HOBJECT     TSS_HENCDATA;    // encrypted data object handle
typedef  TSS_HOBJECT     TSS_HPCRS;       // PCR composite object handle
typedef  TSS_HOBJECT     TSS_HHASH;       // hash object handle
typedef  TSS_HOBJECT     TSS_HNVSTORE;    // NV storage object handle
typedef  TSS_HOBJECT     TSS_HMIGDATA;    // migration data utility obj handle
typedef  TSS_HOBJECT     TSS_HDELFAMILY;  // delegation family object handle
typedef  TSS_HOBJECT     TSS_HDAA_CREDENTIAL; // daa credential
typedef  TSS_HOBJECT     TSS_HDAA_ISSUER_KEY; // daa credential issuer keypair
typedef  TSS_HOBJECT     TSS_HDAA_ARA_KEY;    // daa anonymity revocation
// authority keypair

typedef UINT32  TSS_EVENTTYPE;
typedef UINT16  TSS_MIGRATE_SCHEME;
typedef UINT32  TSS_ALGORITHM_ID;
typedef UINT32  TSS_KEY_USAGE_ID;
typedef UINT16  TSS_KEY_ENC_SCHEME;
typedef UINT16  TSS_KEY_SIG_SCHEME;
typedef BYTE    TSS_KEY_AUTH_DATA_USAGE;
typedef UINT32  TSS_CMK_DELEGATE;
typedef UINT32  TSS_NV_INDEX;
typedef UINT32  TSS_COUNTER_ID;

#include "tpm_defs.h"

#ifndef MANUFACTURER_AMD
#define MANUFACTURER_AMD        'AMD'
#define MANUFACTURER_ATMEL      'ATML'
#define MANUFACTURER_BROADCOM   'BRCM'
#define MANUFACTURER_IBM        'IBM'
#define MANUFACTURER_INFINEON   'IFX'
#define MANUFACTURER_INTEL      'INTC'
#define MANUFACTURER_LENOVO     'LEN'
#define MANUFACTURER_NATIONAL   'NSM '
#define MANUFACTURER_NATIONZ    'NTZ'
#define MANUFACTURER_NUVOTON    'NTC'
#define MANUFACTURER_QUALCOMM   'QCOM'
#define MANUFACTURER_SMSC       'SMSC'
#define MANUFACTURER_ST_MACRO   'STM '
#define MANUFACTURER_SAMSUNG    'SMSN'
#define MANUFACTURER_SINOSUN    'SNS'
#define MANUFACTURER_TEXAS      'TXN'
#define MANUFACTURER_WINBON     'WEC'
#endif

// Access Values
#define TPM_ACCESS_VALID            0x80
#define TPM_ACCESS_ILLEGAL          0x40
#define TPM_ACCESS_ACTIVE_LOCALITY  0x20
#define TPM_ACCESS_SERIZED          0x10
#define TPM_ACCESS_REQUEST_PENDING  0x04
#define TPM_ACCESS_REQUEST_USE      0x02

// Status Values
#define TPM_STS_VALID           0x80
#define TPM_STS_COMMAND_READY   0x40
#define TPM_STS_GO              0x20
#define TPM_STS_DATA_AVAIL      0x10
#define TPM_STS_DATA_EXPECT     0x08

// Int flags
#define TPM_GLOBAL_INT_ENABLE           0x80000000
#define TPM_INTF_BURST_COUNT_STATIC     0x100
#define TPM_INTF_CMD_READY_INT          0x080
#define TPM_INTF_INT_EDGE_FALLING       0x040
#define TPM_INTF_INT_EDGE_RISING        0x020
#define TPM_INTF_INT_LEVEL_LOW          0x010
#define TPM_INTF_INT_LEVEL_HIGH         0x008
#define TPM_INTF_LOCALITY_CHANGE_INT    0x004
#define TPM_INTF_STS_VALID_INT          0x002
#define TPM_INTF_DATA_AVAIL_INT         0x001

// TPM default values
#define TIS_MEM_BASE            0xFED40000
#define TIS_MEM_LEN             0x5000
#define TIS_SHORT_TIMEOUT       750	/* ms */
#define TIS_MEDIUM_TIMEOUT      2000	/* 2 sec */
#define TIS_LONG_TIMEOUT        600000	/* 600 sec */

#define	TPM_ACCESS(l)			(0x0000 | ((l) << 12))
#define	TPM_INT_ENABLE(l)		(0x0008 | ((l) << 12))
#define	TPM_INT_VECTOR(l)		(0x000C | ((l) << 12))
#define	TPM_INT_STATUS(l)		(0x0010 | ((l) << 12))
#define	TPM_INTF_CAPS(l)		(0x0014 | ((l) << 12))
#define	TPM_STS(l)				(0x0018 | ((l) << 12))
#define	TPM_DATA_FIFO(l)		(0x0024 | ((l) << 12))

#define	TPM_DID_VID(l)			(0x0F00 | ((l) << 12))
#define	TPM_RID(l)				(0x0F04 | ((l) << 12))

#ifndef BIG_ENDIAN_UINT

#define BIG_ENDIAN_UINT(value)	((value << 24) | \
								((value & 0x0000FF00) << 8)	 | \
								((value & 0x00FF0000) >> 8)	 | \
								 (value >> 24) )

#define BIG_ENDIAN_WORD(x) 		( ( (x & 0xFF00) >> 8 ) | ( (x & 0xFF) << 8 ))

#endif // BIG_ENDIAN_UINT


#ifdef __cplusplus
extern "C" {
#endif

VOID CmdBufInit(PUCHAR Buf,UINT32 Ordinal);
VOID CmdBufAddUINT32(PUCHAR Cmd,UINT32 data);
VOID CmdBufAddUSHORT(PUCHAR Cmd,USHORT data);
VOID CmdBufAddUCHAR(PUCHAR Cmd,UCHAR data);
VOID CmdBufAddBuf(PUCHAR Cmd,PUCHAR Data,UINT32 DataLen);
UINT32 CmdBufGetLen(PUCHAR Cmd);

TSS_RESULT GetResultStatus(PUCHAR Buf);

#ifdef __cplusplus
};
#endif

#endif

