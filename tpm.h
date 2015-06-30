
#ifndef _TPM_H
#define _TPM_H

#pragma once

#define POOL_TAG	'CTPM'

typedef enum
{
	StAvailable			= 0,
	StBusy				= 1,
	StAborting			= 2,
	StSuspendPending	= 3,
	StSuspend			= 4,
	StStatusMax			= 5
} TPM_STATE;

typedef enum
{
	IdThread	= 0,
	IdAbort		= 1,
	IdPowerdown = 2,
	IdPowerup	= 3,
	IdEntropy	= 4,
	ThreadIdMax = 5
} TPM_THREAD_ID;

//
typedef struct _TPM_CONTEXT
{
        WDFDEVICE       Device;		// 0x0			Store your control data here
        PUCHAR          MemAddr;	// 0x4
        ULONG           MemLen;		// 0x8
        KEVENT          Event;      // 0xCh

        TPM_STATE		TpmState;   // 0x1Ch
        KSPIN_LOCK      SpinLock;   // 0x20h

        ULONG    AccessTimeOut;             // 0x24h
        ULONG    WaitForBitSetTime;         // 0x28h
        ULONG    WriteCommandTimeOut;       // 0x2Ch
        ULONG    BurstValueTimeOut;         // 0x30h

        ULONG    Shortduration;  // 0x34h
        ULONG    Mediumduration; // 0x38h
        ULONG    Longduration;   // 0x3Ch

        BOOLEAN    SkipInitCommands; // 0x40h
        BOOLEAN    SkipUnderflowCheck; // 0x41
        BOOLEAN    SkipOverflowCheck; // 0x42
        BOOLEAN    SkipAccessRegisterCheck; // 0x43
        ULONG    UsePortBasedIO;    // 0x44h
        UCHAR    DelayExpectBitCheck; // 0x48
        BOOLEAN	 AccessRegister;    // 0x49h

        PUCHAR   PortAddr;          // 0x4Ch
        ULONG	 EntropyDensity;    // 0x50h
        ULONG	 TimeOutSecond;     // 0x54h
        UCHAR    bUseTimeOut;       // 0x58
        UCHAR    PendingEntropy;    // 0x59
	ENTROPY_SOURCE_HANDLE hEntropySource; // 0x5Ch
        WDFTIMER  timerHandle; // 0x60
		// my added
        ULONG		Manufacturer;
		ULONG		TPMVersion;
} TPM_CONTEXT, *PTPM_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(TPM_CONTEXT, GetTpmContext)

typedef NTSTATUS (*PFN_TpmSetMorBitState)(PTPM_CONTEXT TpmContext,UCHAR access);
// 0x14h
typedef struct _TPM_INTERFACE_STANDARD {
  INTERFACE  InterfaceHeader;
  PFN_TpmSetMorBitState  pfn_TpmSetMorBitState;
} TPM_INTERFACE_STANDARD, *PTPM_INTERFACE_STANDARD;

typedef struct _TPM_ENTROPY_TIMER_CONTEXT
{
    PTPM_CONTEXT    TpmContext;
} TPM_ENTROPY_TIMER_CONTEXT,*PTPM_ENTROPY_TIMER_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(TPM_ENTROPY_TIMER_CONTEXT, GetTimerContext)

NTSTATUS
TpmEvtDeviceAdd(
                                IN WDFDRIVER        Driver,
                                IN PWDFDEVICE_INIT  DeviceInit
    );

NTSTATUS
TpmEvtDevicePrepareHardware (
                                                         WDFDEVICE      Device,
                                                         WDFCMRESLIST   Resources,
                                                         WDFCMRESLIST   ResourcesTranslated
    );

NTSTATUS
TpmEvtDeviceReleaseHardware(
                                                        IN  WDFDEVICE Device,
                                                        IN  WDFCMRESLIST ResourcesTranslated
    );

NTSTATUS
TpmEvtDeviceD0Entry(
                                        IN  WDFDEVICE Device,
                                        IN  WDF_POWER_DEVICE_STATE PreviousState
    );

NTSTATUS
TpmEvtDeviceD0Exit(
                                   IN  WDFDEVICE Device,
                                   IN  WDF_DEVICE_POWER_STATE TargetState
    );


NTSTATUS TpmEntropyInit(PTPM_CONTEXT TpmContext);
VOID TpmEvtEntropyTimer(IN WDFTIMER  Timer);


NTSTATUS TpmAbort(PTPM_CONTEXT TpmContext, TPM_STATE TpmOldState, TPM_STATE TpmNewState, TPM_THREAD_ID ThreadId);

VOID TpmInitStateTable();

ULONG TpmUpdateTpmState(PTPM_CONTEXT TpmContext,TPM_STATE TpmState,TPM_THREAD_ID ThreadId);

NTSTATUS TpmWaitForBitSet(PTPM_CONTEXT TpmContext,ULONG Register,UCHAR Value,int TimeOut,PUCHAR Bitset);

NTSTATUS TpmGetCommandResultLoop(PTPM_CONTEXT TpmContext,
								 PULONG OutputSize,
								 PUCHAR OutputBuffer,
								 ULONG duration);

VOID TpmGetRegistryFlags(WDFDEVICE device,PTPM_CONTEXT TpmContext);

NTSTATUS TpmGetEntropyInterval(ULONG* Interval);

ULONG TpmGetEntropyDensity();

#if _MSC_VER <= 1200

#define ExFreePoolWithTag(x,s)	ExFreePool(x)

#else

NTKERNELAPI
VOID
ExFreePoolWithTag(
    __in PVOID P,
    __in ULONG Tag
    );

#endif

#endif // _TPM_H
