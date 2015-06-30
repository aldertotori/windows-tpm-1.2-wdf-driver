#define WIN9X_COMPAT_SPINLOCK
#include <ntddk.h>
#pragma warning(disable:4201)  // nameless struct/union warning

#include <stdarg.h>
#include <wdf.h>

#pragma warning(default:4201)

#include <initguid.h> // required for GUID definitions
#include <wdmguid.h>  // required for WMILIB_CONTEXT

#ifdef WIN32
#include <basetsd.h>
typedef  unsigned char    BYTE;  
typedef  signed char      TSS_BOOL;
#ifndef _BASETSD_H_
// basetsd.h provides definitions of UINT16, UINT32 and UINT64.
typedef  unsigned short   UINT16;
typedef  unsigned long    UINT32;
typedef  unsigned __int64 UINT64;
#endif
typedef  unsigned short   TSS_UNICODE;
typedef  void*            PVOID;
#endif

#if _MSC_VER <= 1200
#if (WDM_MAJORVERSION == 0x01 && WDM_MINORVERSION == 0x10 && DBG == 1)
#pragma warning( disable : 4002)
#define KdPrintEx
#else
#define KdPrintEx
#endif	// (WDM_MAJORVERSION == 0x01 && WDM_MINORVERSION == 0x10)

#ifndef DPFLTR_PNPMEM_ID
#define DPFLTR_PNPMEM_ID		0x72
#define DPFLTR_WARNING_LEVEL	1
#define DPFLTR_TRACE_LEVEL		2
#define DPFLTR_INFO_LEVEL		3
#endif

#endif // WDM_MAJORVERSION < 6



#include "cng/cng.h"
#include "defs.h"
//#include "acpiioct.h"
#include "tpm.h"
#include "acpi.h"
#include "dbg.h"
#include "ioctl.h"
#include "tpm_tis.h"
//#include "CmdBuf.h"

#ifndef TPM_SUCCESS
#define TPM_SUCCESS						0
#define TPM_E_NON_FATAL				0x00000800
#define TPM_E_DOING_SELFTEST		0x00000802
#define TPM_E_DEACTIVATED			0x00000006
#define TPM_E_DISABLED				0x00000007
#endif
