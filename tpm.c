
#include "precomp.h"
#include "guid.h"

#pragma comment(lib,"cng.lib")

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath);

VOID  TpmEvtIoDeviceControl (
							 IN WDFQUEUE  Queue,
							 IN WDFREQUEST  Request,
							 IN size_t  OutputBufferLength,
							 IN size_t  InputBufferLength,
							 IN ULONG  IoControlCode);

ULONG GetProcessNameOffset();
BOOLEAN IsTbsServiceProcess();

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (INIT, GetProcessNameOffset)
#pragma alloc_text (PAGE, IsTbsServiceProcess)
#pragma alloc_text (PAGE, TpmEvtDeviceAdd)
#pragma alloc_text (PAGE, TpmEvtDeviceReleaseHardware)
#pragma alloc_text (PAGE, TpmEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, TpmEvtDeviceD0Exit)
#pragma alloc_text (PAGE, TpmEntropyInit)
#pragma alloc_text (PAGE, TpmEvtEntropyTimer)
#pragma alloc_text (PAGE, TpmEvtIoDeviceControl)
#pragma alloc_text (PAGE, TpmInitStateTable)

#endif

ULONG	ProcessNameOffset = 0;
// [OldState] [ NewState] [Thread] 
UCHAR	TpmTransTable[StStatusMax][StStatusMax][ThreadIdMax] = { 0 };

VOID TpmInitStateTable()
{

	PAGED_CODE();

    TpmTransTable[StAvailable][StAvailable][IdThread]			|= 1;	// TpmTransTable_1
    TpmTransTable[StAvailable][StBusy][IdThread]				|= 1;	// TpmTransTable_6
    TpmTransTable[StBusy][StAvailable][IdThread]				|= 1;	// TpmTransTable_26

	TpmTransTable[StBusy][StAborting][IdAbort]					|= 1;	// TpmTransTable_37
	TpmTransTable[StAborting][StAvailable][IdAbort]				|= 1;	// TpmTransTable_52

    TpmTransTable[StBusy][StSuspendPending][IdPowerdown]		|= 1;	// TpmTransTable_43
    TpmTransTable[StAborting][StSuspendPending][IdPowerdown]	|= 1;	// TpmTransTable_68
    TpmTransTable[StSuspendPending][StSuspend][IdPowerdown]		|= 1;	// TpmTransTable_98

    TpmTransTable[StAvailable][StSuspendPending][IdPowerdown]	|= 1;  // TpmTransTable_18
	TpmTransTable[StSuspend][StAvailable][IdPowerup]			|= 1;  // TpmTransTable_104

	TpmTransTable[StAvailable][StBusy][IdEntropy]				|= 1;	// TpmTransTable_10
    TpmTransTable[StBusy][StAvailable][IdEntropy]				|= 1;	// TpmTransTable_30

}

ULONG GetProcessNameOffset()
{
	ULONG Offset   = 0;
	char* Eprocess;

	PAGED_CODE();
	
	Eprocess = (char*)IoGetCurrentProcess();

	while(Offset < 0x3000)
	{
		if(strncmp(Eprocess+Offset,"System",6) == 0)
			break;
		Offset++;
	}
	return Offset < 0x3000 ? Offset : 0;
}

BOOLEAN IsTbsServiceProcess()
{
	char* ProcessName;

	PAGED_CODE();

	ProcessName = (char*)IoGetCurrentProcess();
	ProcessName += ProcessNameOffset;

	if(	strncmp(ProcessName,"svchost.exe",11) == 0)
	{
		return TRUE;
	}
	return FALSE;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING RegistryPath)
{
    NTSTATUS            status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG   config;

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TPM Driver - using Windows Driver Frame"));

	ProcessNameOffset = GetProcessNameOffset();
	
    //
    // Initialize the Driver Config structure.
    //
    WDF_DRIVER_CONFIG_INIT( &config, TpmEvtDeviceAdd );

    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //

    status = WdfDriverCreate( DriverObject,
		RegistryPath,
		NULL,
		&config,
		WDF_NO_HANDLE);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
			"WdfDriverCreate failed with status %!STATUS!", status));
    }
	
    return status;
}


NTSTATUS
TpmEvtDeviceAdd(
				IN WDFDRIVER        Driver,
				IN PWDFDEVICE_INIT  DeviceInit
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    WDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks;
    WDF_OBJECT_ATTRIBUTES       DeviceAttributes;
    WDFDEVICE                   device;
    PTPM_CONTEXT                TpmContext;
    WDF_IO_QUEUE_CONFIG         IoQueueConfig;
    WDFQUEUE                    hQueue;
    WDF_QUERY_INTERFACE_CONFIG  InterfaceConfig;
    TPM_INTERFACE_STANDARD      TpmInterface;
    WDF_DEVICE_POWER_CAPABILITIES   PowerCaps;

	PAGED_CODE();

    UNREFERENCED_PARAMETER( Driver );

    
    RtlZeroMemory(&PnpPowerEventCallbacks,sizeof(PnpPowerEventCallbacks));
    RtlZeroMemory(&PowerCaps,sizeof(PowerCaps));

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
		"TpmEvtDeviceAdd routine PDO: %p (%p)\n",
				WdfDriverWdmGetDriverObject(Driver)));
    //
    // Zero out the PnpPowerCallbacks structure.
    //
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&PnpPowerEventCallbacks);

    //
    // These two callbacks set up and tear down hardware state that must be
    // done every time the device moves in and out of the D0-working state.
    //
    PnpPowerEventCallbacks.EvtDeviceD0Entry         = TpmEvtDeviceD0Entry;

    PnpPowerEventCallbacks.EvtDeviceD0Exit          = TpmEvtDeviceD0Exit;
    //
    // Set Callbacks for any of the functions we are interested in.
    // If no callback is set, Framework will take the default action
    // by itself.
    //
    PnpPowerEventCallbacks.EvtDevicePrepareHardware = TpmEvtDevicePrepareHardware;
    PnpPowerEventCallbacks.EvtDeviceReleaseHardware = TpmEvtDeviceReleaseHardware;


    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit,&PnpPowerEventCallbacks);

    //
    // Specify the context type and size for the device we are about to create.
    //
	RtlZeroMemory(&DeviceAttributes,sizeof(DeviceAttributes));
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&DeviceAttributes, TPM_CONTEXT);
	DeviceAttributes.Size = sizeof(WDF_OBJECT_ATTRIBUTES);
	DeviceAttributes.SynchronizationScope	= WdfSynchronizationScopeInheritFromParent;
	DeviceAttributes.ExecutionLevel			= WdfExecutionLevelPassive;

    status = WdfDeviceCreate(&DeviceInit,&DeviceAttributes,&device);

    if(NT_SUCCESS(status))
    {
        TpmContext = GetTpmContext(device);
        RtlZeroMemory(TpmContext,sizeof(TPM_CONTEXT));
        TpmContext->Device = device;
        TpmContext->AccessRegister = FALSE;
        KeInitializeSpinLock(&TpmContext->SpinLock);
        TpmContext->TpmState = StSuspend;
        TpmInitStateTable();
        KeInitializeEvent(&TpmContext->Event,NotificationEvent,TRUE);

        status = WdfDeviceCreateDeviceInterface(device,&GUID_DEVINTERFACE_TPM,NULL);

        if(NT_SUCCESS(status))
        {
            WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
                                           &IoQueueConfig,
                                           WdfIoQueueDispatchParallel
                                           );
            IoQueueConfig.EvtIoDeviceControl = TpmEvtIoDeviceControl;

            status = WdfIoQueueCreate(
                              device,
                              &IoQueueConfig,
                              WDF_NO_OBJECT_ATTRIBUTES,
                              &hQueue
                              );

            if(NT_SUCCESS(status))
            {

                WDF_DEVICE_POWER_CAPABILITIES_INIT(&PowerCaps);

                WdfDeviceSetPowerCapabilities(device,&PowerCaps);

                //
                TpmGetRegistryFlags(device,TpmContext);

                //
                RtlZeroMemory(&TpmInterface,sizeof(TpmInterface));
                TpmInterface.InterfaceHeader.Size = sizeof(TpmInterface);
                TpmInterface.InterfaceHeader.Version = 1;
                TpmInterface.InterfaceHeader.Context = (PVOID)TpmContext;

                TpmInterface.InterfaceHeader.InterfaceReference = WdfDeviceInterfaceReferenceNoOp;
                TpmInterface.InterfaceHeader.InterfaceDereference = WdfDeviceInterfaceDereferenceNoOp;
                TpmInterface.pfn_TpmSetMorBitState = TpmSetMorBitState;

                //
                // Initialize the InterfaceConfig structure.
                //
                WDF_QUERY_INTERFACE_CONFIG_INIT(
                                &InterfaceConfig,
                                (PINTERFACE)&TpmInterface,
                                &GUID_TPM_INTERFACE_STANDARD,
                                NULL
                                );

                //
                // Create the interface.
                //
                status = WdfDeviceAddQueryInterface(
                                                    device,
                                                    &InterfaceConfig
                                                    );
                if(!NT_SUCCESS(status))
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                        "WdfDeviceAddQueryInterface failed with NTSTATUS 0x%x\n",status));
                }
            }
            else
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "WdfIoQueueCreate failed with Status code 0x%x\n",status));
            }
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "WdfDeviceCreateDeviceInterface failed with Status code 0x%x\n",status));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "WdfDeviceCreate failed with Status code 0x%x\n",status));
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmEvtDeviceAdd exited with Status code 0x%x\n",status));

    return status;
}

NTSTATUS
TpmEvtDevicePrepareHardware (
							 WDFDEVICE      Device,
							 WDFCMRESLIST   Resources,
							 WDFCMRESLIST   ResourcesTranslated
    )
{
    NTSTATUS                    status;
    ULONG                       ListCount = 0;
    ULONG                       MemLen = 0;
//    ULONG                       NumberOfBytes;
    PTPM_CONTEXT                TpmContext;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR ResourceDesc;
    ULONG                       ListIndex;
    PHYSICAL_ADDRESS            MemoryAddr;
    BOOLEAN                     bMemAddr = FALSE;

	PAGED_CODE();

    TpmContext = GetTpmContext(Device);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmEvtDevicePrepareHardware (raw count = %u, trans count = %u)\n",
                WdfCmResourceListGetCount(Resources),
                WdfCmResourceListGetCount(ResourcesTranslated)));

    ListIndex = 0;

    ListCount = WdfCmResourceListGetCount(ResourcesTranslated);

    if(ListCount)
    {
        while(ListIndex < ListCount)
        {
            ResourceDesc = WdfCmResourceListGetDescriptor(ResourcesTranslated,ListIndex);

            if(!ResourceDesc) break;

            if(ResourceDesc->Type == CmResourceTypePort)
            {
                if(TpmContext->UsePortBasedIO)
                {
                    TpmContext->PortAddr = (PUCHAR)ResourceDesc->u.Port.Start.u.LowPart;
                }
            }
            else if(ResourceDesc->Type == CmResourceTypeMemory && TpmContext->UsePortBasedIO == 0)
            {
                MemoryAddr.u.LowPart = ResourceDesc->u.Memory.Start.u.LowPart;
                MemoryAddr.u.HighPart = ResourceDesc->u.Memory.Start.u.HighPart;
                MemLen = ResourceDesc->u.Memory.Length;
                bMemAddr = TRUE;
            }
            ListIndex++;
        }

        do
        {
            if(ListIndex < WdfCmResourceListGetCount( ResourcesTranslated))
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "WdfCmResourceListGetDescriptor failed.\n"));
                status = STATUS_DEVICE_CONFIGURATION_ERROR; // 0xC0000182;
                break;
            }

            if(TpmContext->UsePortBasedIO)
            {
                if(!TpmContext->PortAddr)
                {
                    if(TpmContext->UsePortBasedIO != TRUE)
                    {
                        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                                  "Port I/O resources required but not allocated for Vendor index %u",
                                  TpmContext->UsePortBasedIO));
                        status = STATUS_DEVICE_CONFIGURATION_ERROR; // 0xC0000182;
                        goto Exit;
                    }

                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                            "No resources allocated for Port-based I/O."));

					status = STATUS_DEVICE_CONFIGURATION_ERROR;
					break;
                }
            }
            else
            {
                if(!bMemAddr)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "Physical address resource not provided via ACPI; using default address and size.\n"));
                    MemLen = 1024;
                    MemoryAddr.u.LowPart    = TPM_DEFAULT_ADDRESS;
                    MemoryAddr.u.HighPart   = 0;
                }

                if(MemLen < 1024)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "Physical address length provided is too small (%d)\n",MemLen));
                    status = STATUS_DEVICE_CONFIGURATION_ERROR; //0xC0000182;
                    break;
                }

                if(MemoryAddr.u.LowPart != TPM_DEFAULT_ADDRESS || MemoryAddr.u.HighPart)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                                "Physical address provided is incorrect: 0x%08x`%08x\n",MemoryAddr.u.HighPart,MemoryAddr.u.LowPart));
                    status = STATUS_DEVICE_CONFIGURATION_ERROR; //0xC0000182;
                    break;
                }

                TpmContext->MemAddr = MmMapIoSpace(MemoryAddr,MemLen,MmNonCached);
                if(!TpmContext->MemAddr)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "MmMapIoSpace couldn't map the TPM device: 0x%x.\n", MemoryAddr.u.LowPart));
                    status = STATUS_INSUFFICIENT_RESOURCES; //0xC000009A;
                    break;
                }

                TpmContext->MemLen = MemLen;

            }

            status = TpmSetDefaultTimingValues(TpmContext);
            if(!NT_SUCCESS(status))
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "TpmSetDefaultTimingValues failed with status 0x%x",status));
                break;
            }

            if(TpmContext->SkipInitCommands)
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "Skipping initialization commands\n"));
            }
            else
            {
                TpmGetTimeoutInfo(TpmContext);
                TpmGetDurations(TpmContext);
				// 获取硬件厂商信息
				TpmGetManufacturer(TpmContext);
            }

            status = TpmVerifyAccessRegister(TpmContext,TpmINB(TpmContext,0),0);
            if(!NT_SUCCESS(status))
            {
                break;
            }

#if _NT_TARGET_VERSION >= 0x601
            status = TpmEntropyInit(TpmContext);
#endif

        } while(FALSE);
    }
    else
    {
         KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "Physical address resource not provided via ACPI; using default address and size.\n"));
    }

Exit:
    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
        "TpmEvtDevicePrepareHardware exited with Status code 0x%x\n",status));

    return status;
}

NTSTATUS
TpmEvtDeviceReleaseHardware(
							IN  WDFDEVICE Device,
							IN  WDFCMRESLIST ResourcesTranslated
    )
{
    PTPM_CONTEXT                TpmContext;

	PAGED_CODE();

    TpmContext = GetTpmContext(Device);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
        "TpmEvtDeviceReleaseHardware\n"));

#if _NT_TARGET_VERSION >= 0x601
    if(TpmContext->hEntropySource)
        EntropyUnregisterSource(TpmContext->hEntropySource);
    TpmContext->hEntropySource = NULL;
#endif

    if(TpmContext->MemAddr)
        MmUnmapIoSpace(TpmContext->MemAddr,TpmContext->MemLen);
    TpmContext->MemAddr = NULL;
    TpmContext->MemLen  = 0;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
        "TpmEvtDeviceReleaseHardware exited with Status code 0x%x\n",STATUS_SUCCESS));

    return STATUS_SUCCESS;
}

NTSTATUS
TpmEvtDeviceD0Entry(
					IN  WDFDEVICE Device,
					IN  WDF_POWER_DEVICE_STATE PreviousState
    )
{
    PTPM_CONTEXT    TpmContext;
    NTSTATUS        status;

    UNREFERENCED_PARAMETER(PreviousState);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmEvtDeviceD0Entry: Enter\n"));

    TpmContext = GetTpmContext(Device);

    TpmContext->AccessRegister = FALSE;

    TpmContinueSelfTest(TpmContext);

#if _NT_TARGET_VERSION >= 0x601
    TpmProvideEntropy(TpmContext);

    if(TpmContext->TimeOutSecond)
    {
        WdfTimerStart(TpmContext->timerHandle,
                      WDF_REL_TIMEOUT_IN_MS(TpmContext->TimeOutSecond));
    }
#endif

    TpmUpdateTpmState(TpmContext,StAvailable,IdPowerup);

    return STATUS_SUCCESS;
}

NTSTATUS
TpmEvtDeviceD0Exit(
				   IN  WDFDEVICE Device,
				   IN  WDF_DEVICE_POWER_STATE TargetState
    )
{
    NTSTATUS     Status = STATUS_UNSUCCESSFUL;
	PTPM_CONTEXT TpmContext;

	PAGED_CODE();

	TpmContext = GetTpmContext(Device);

    if(TargetState == WdfPowerDeviceD3)
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmEvtDeviceD0Exit: going to sleep; aborting and saving state.\n"));

        Status = TpmAbort(TpmContext,StSuspend,StSuspendPending,IdPowerdown);

        if(!NT_SUCCESS(Status))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmEvtDeviceD0Exit: TpmAbort failed for existing command with Status 0x%x\n",Status));
        }

        Status = TpmSaveState(TpmContext);

        if(!NT_SUCCESS(Status))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmEvtDeviceD0Exit: TpmSaveState failed with Status 0x%x\n",Status));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmEvtDeviceD0Exit: hibernating or powering down (state 0x%x).  Discarding TPM state.\n",
			Status));
    }

    return STATUS_SUCCESS;
}


NTSTATUS TpmEntropyInit(PTPM_CONTEXT TpmContext)
{
    NTSTATUS                status = STATUS_SUCCESS;
#if _NT_TARGET_VERSION >= 0x601	// win7
    WDF_TIMER_CONFIG        timerConfig;
    WDF_OBJECT_ATTRIBUTES   timerAttributes;
    WDFTIMER                timerHandle;
    PTPM_ENTROPY_TIMER_CONTEXT  TimerContext;

	PAGED_CODE();

    status = EntropyRegisterSource(&TpmContext->hEntropySource,
                          ENTROPY_SOURCE_TYPE_TPM,
                          CNG_ENTROPY_NAME);

    TpmContext->EntropyDensity = TpmGetEntropyDensity();

    TpmContext->TimeOutSecond  = 60000;
	timerConfig.TolerableDelay = 60000;

    TpmContext->bUseTimeOut = FALSE;
    TpmContext->PendingEntropy = FALSE;

    WDF_TIMER_CONFIG_INIT(&timerConfig,
                          TpmEvtEntropyTimer);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&timerAttributes,TPM_ENTROPY_TIMER_CONTEXT);
	timerAttributes.ParentObject = TpmContext->Device;

    status = WdfTimerCreate(
                        &timerConfig,
                        &timerAttributes,
                        &TpmContext->timerHandle
                        );

    if(NT_SUCCESS(status))
    {
        TimerContext = GetTimerContext(TpmContext->timerHandle);
        TimerContext->TpmContext = TpmContext;
    }
#endif
    return status;
}

VOID TpmEvtEntropyTimer(IN WDFTIMER  Timer)
{
#if _NT_TARGET_VERSION >= 0x601

    ULONG			Interval;
    PTPM_CONTEXT    TpmContext;

	PAGED_CODE();

	TpmContext = GetTimerContext(Timer)->TpmContext;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmEvtEntropyTimer().\n"));

    if(TpmContext->bUseTimeOut)
    {
        TpmContext->PendingEntropy = TRUE;
        if(NT_SUCCESS(TpmUpdateTpmState(TpmContext,StBusy,IdEntropy)))
        {
            TpmProvideEntropy(TpmContext);
            TpmUpdateTpmState(TpmContext,StAvailable,IdEntropy);
        }
    }
    else
    {
		if(NT_SUCCESS(TpmGetEntropyInterval(&Interval)))
		{
            TpmContext->TimeOutSecond = 60000 * Interval;
            TpmContext->bUseTimeOut = TRUE;
       }
    }

    if(TpmContext->TimeOutSecond)
    {
        WdfTimerStart(TpmContext->timerHandle,
                      WDF_REL_TIMEOUT_IN_MS(TpmContext->TimeOutSecond));
    }
#endif
}


VOID  TpmEvtIoDeviceControl (
    IN WDFQUEUE  Queue,
    IN WDFREQUEST  Request,
    IN size_t  OutputBufferLength,
    IN size_t  InputBufferLength,
    IN ULONG  IoControlCode
    )
{
    NTSTATUS        status;
    PTPM_CONTEXT    TpmContext;
    WDFDEVICE       Device;

	PAGED_CODE();

    Device = WdfIoQueueGetDevice(Queue);
    TpmContext = GetTpmContext(Device);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmEvtIoDeviceControl called - IoControlCode: 0x%x \n",
            IoControlCode));

    switch(IoControlCode)
    {
    case TPM_CANCEL_COMMAND:
		
        status = TpmAbort(TpmContext,StAvailable,StAborting,IdAbort);
        break;

    case TPM_TRANSMIT_COMMAND:
		
        status = TpmHandleTransmit(Device,
                                   Request,
                                   OutputBufferLength,
                                   InputBufferLength);
        break;

    case TPM_SUMBIT_ACPI_COMMAND:

        if(TpmContext->TpmState != StAvailable)
        {
            status = STATUS_CANCELLED;
            WdfRequestComplete( Request, status);
			return;
        }

        status = TpmHandleSubmitAcpiCommand(Device,
                                   Request,
                                   OutputBufferLength,
                                   InputBufferLength);

        break;

    default:
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmEvtIoDeviceControl: Invalid request 0x%x\n",IoControlCode));
        status = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    WdfRequestComplete( Request, status);
}

ULONG TpmUpdateTpmState(PTPM_CONTEXT TpmContext,TPM_STATE TpmState,TPM_THREAD_ID ThreadId)
{
    NTSTATUS	Status		= STATUS_SUCCESS;
    ULONG		OldState	= TpmContext->TpmState;
    UCHAR		State;
    char*		TransState;
	KIRQL		OldIrql;

	PAGED_CODE();

    KeAcquireSpinLock(&TpmContext->SpinLock,&OldIrql);

    State = TpmTransTable[OldState][TpmState][ThreadId];
    Status = (State & 1) != 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;

    if(Status == STATUS_SUCCESS)
        TpmContext->TpmState = TpmState;

    KeReleaseSpinLock(&TpmContext->SpinLock,OldIrql);

	if(Status == STATUS_UNSUCCESSFUL)
		TransState = " - INVALID!";
	else
		TransState = "";

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmUpdateTpmState: %s -> %s (thread %s)%s\n",
            TpmDbgGetTpmStateString(OldState),
            TpmDbgGetTpmStateString(TpmState),
            TpmDbgGetTpmThreadIDString(ThreadId),
            TransState));

    return Status;
}


