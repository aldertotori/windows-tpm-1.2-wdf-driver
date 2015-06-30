
#ifndef _DEFS_H_
#define _DEFS_H_

#pragma once

#include "compat11b.h"

#define TPM_DEFAULT_ADDRESS     0xFED40000

// #if defined(LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__) || defined(__ORDER_LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN)

#if PROCESSOR_ARCHITECTURE == AMD64 || PROCESSOR_ARCHITECTURE == X86

#define BIG_ENDIAN_UINT(x)	\
	(	( x << 24) | \
		((x & 0x0000FF00) << 8)	 | \
		((x & 0x00FF0000) >> 8)	 | \
		( x >> 24)   )

#define BIG_ENDIAN_WORD(x) 	( (USHORT)( (x & 0xFF00) >> 8 ) | ( (x & 0xFF) << 8 ) )

#else
#define BIG_ENDIAN_UINT(x) (x)
#define BIG_ENDIAN_WORD(x) (x)
#endif

typedef enum
{
    PROVIDER_STANDARD	= 0,  // ThinkPads with TPM 1.2
    PROVIDER_ATMEL		= 1,  // Atmel 97SC3201 chips
    PROVIDER_INFINEON	= 2,
    PROVIDER_NATIONAL	= 3,  // nsc ThinkPad T43/P and R52
    PROVIDER_INTEL		= 4,
    PROVIDER_BROADCOM	= 5
} TPM_PROVIDER;

#pragma pack(push,1)

typedef struct _TPM_CMD_BUFFER
{
    USHORT      tag;
    ULONG       paramSize;
    ULONG       ordinal;
} TPM_CMD_BUFFER,*PTPM_CMD_BUFFER;

#define INIT_CMD_BUFFER(x,y)  \
        x.tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND);   \
        x.paramSize = BIG_ENDIAN_UINT(sizeof(TPM_CMD_BUFFER)); \
        x.ordinal = BIG_ENDIAN_UINT(y);

typedef struct _TPM_RESULT_BUFFER
{
    USHORT      tag;
    ULONG       paramSize;
    ULONG       returnCode;
} TPM_RESULT_BUFFER,*PTPM_RESULT_BUFFER;

typedef struct _TPM_RANDOM_CMD
{
    TPM_CMD_BUFFER  Cmd;
    ULONG           bytesRequest;
} TPM_RANDOM_CMD,*PTPM_RANDOM_CMD;

#define RANDOM_REQUEST_SIZE  0x40

typedef struct _TPM_RANDOM_RESULT
{
    TPM_RESULT_BUFFER   Result;
    ULONG               randomBytesSize;
    UCHAR               randomBytes[RANDOM_REQUEST_SIZE];
} TPM_RANDOM_RESULT,*PTPM_RANDOM_RESULT;

#define INIT_RANDOM_CMD(x)    \
        x.Cmd.tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND); \
        x.Cmd.paramSize = BIG_ENDIAN_UINT(sizeof(TPM_RANDOM_CMD)); \
        x.Cmd.ordinal = BIG_ENDIAN_UINT(TPM_ORD_GetRandom); \
        x.bytesRequest = BIG_ENDIAN_UINT(RANDOM_REQUEST_SIZE);

// 22
typedef struct _TPM_GET_CAPABILITY_CMD
{
    TPM_CMD_BUFFER  Cmd;
    ULONG           capArea;
    ULONG           subCapSize;
    ULONG           subCap;
} TPM_GET_CAPABILITY_CMD,*PTPM_GET_CAPABILITY_CMD;

// 26
typedef struct _TPM_GET_DURATIONS_RESULT
{
    TPM_RESULT_BUFFER   Result;
    ULONG               respSize;
    ULONG               Shortduration;
    ULONG               Mediumduration;
    ULONG               Longduration;
} TPM_GET_DURATIONS_RESULT,*PTPM_GET_DURATIONS_RESULT;

#define INIT_GET_DURATIONS_CMD(x)    \
        x.Cmd.tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND); \
        x.Cmd.paramSize = BIG_ENDIAN_UINT(sizeof(TPM_GET_CAPABILITY_CMD)); \
        x.Cmd.ordinal = BIG_ENDIAN_UINT(TPM_ORD_GetCapability); \
        x.capArea     = BIG_ENDIAN_UINT(TPM_CAP_PROPERTY);   \
        x.subCapSize  = BIG_ENDIAN_UINT(sizeof(ULONG));     \
        x.subCap = BIG_ENDIAN_UINT(TPM_CAP_PROP_DURATION);

typedef struct _TPM_GET_TIMEOUT_INFO_RESULT
{
    TPM_RESULT_BUFFER   Result;
    ULONG               respSize;
    ULONG               AccessTimeOut;
    ULONG               WaitForBitSetTime;
    ULONG               WriteCommandTimeOut;
    ULONG               BurstValueTimeOut;
} TPM_GET_TIMEOUT_INFO_RESULT,*PTPM_GET_TIMEOUT_INFO_RESULT;

#define INIT_GET_TIMEOUT_INFO_CMD(x)    \
        x.Cmd.tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND); \
        x.Cmd.paramSize = BIG_ENDIAN_UINT(sizeof(TPM_GET_CAPABILITY_CMD)); \
        x.Cmd.ordinal = BIG_ENDIAN_UINT(TPM_ORD_GetCapability); \
        x.capArea     = BIG_ENDIAN_UINT(TPM_CAP_PROPERTY);   \
        x.subCapSize  = BIG_ENDIAN_UINT(sizeof(ULONG));     \
        x.subCap = BIG_ENDIAN_UINT(TPM_CAP_PROP_TIS_TIMEOUT);

typedef struct _TPM_GET_MANUFACTURER_RESULT
{
    TPM_RESULT_BUFFER   Result;
    ULONG               respSize;
    ULONG               Manufacturer;
} TPM_GET_MANUFACTURER_RESULT,*PTPM_GET_MANUFACTURER_RESULT;

#define INIT_GET_MANUFACTURER_CMD(x)    \
        x.Cmd.tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND); \
        x.Cmd.paramSize = BIG_ENDIAN_UINT(sizeof(TPM_GET_CAPABILITY_CMD)); \
        x.Cmd.ordinal = BIG_ENDIAN_UINT(TPM_ORD_GetCapability); \
        x.capArea     = BIG_ENDIAN_UINT(TPM_CAP_PROPERTY);   \
        x.subCapSize  = BIG_ENDIAN_UINT(sizeof(ULONG));     \
        x.subCap = BIG_ENDIAN_UINT(TPM_CAP_PROP_MANUFACTURER);

#pragma pack(pop)

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
#define TPM_ACCESS_VALID            ((UCHAR)0x80)
#define TPM_ACCESS_ILLEGAL          ((UCHAR)0x40)
#define TPM_ACCESS_ACTIVE_LOCALITY  ((UCHAR)0x20)
#define TPM_ACCESS_SERIZED          ((UCHAR)0x10)
#define TPM_ACCESS_REQUEST_PENDING  ((UCHAR)0x04)
#define TPM_ACCESS_REQUEST_USE      ((UCHAR)0x02)

// Status Values
#define TPM_STS_VALID           ((UCHAR)0x80)
#define TPM_STS_COMMAND_READY   ((UCHAR)0x40)
#define TPM_STS_GO              ((UCHAR)0x20)
#define TPM_STS_DATA_AVAIL      ((UCHAR)0x10)
#define TPM_STS_DATA_EXPECT     ((UCHAR)0x08)

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

#endif

