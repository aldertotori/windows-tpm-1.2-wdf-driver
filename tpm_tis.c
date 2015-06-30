
#include "precomp.h"
#pragma data_seg(".text")
#include <initguid.h>
#include "guid.h"
#pragma data_seg()

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, TpmAbort)
#pragma alloc_text (PAGE, TpmGetManufacturer)
#pragma alloc_text (PAGE, TpmActivateCurrentLocality)
#pragma alloc_text (PAGE, TpmGetDurations)
#pragma alloc_text (PAGE, TpmGetTimeoutInfo)
#pragma alloc_text (PAGE, TpmSaveState)
#pragma alloc_text (PAGE, TpmSetDefaultTimingValues)
#pragma alloc_text (PAGE, TpmVerifyAccessRegister)
#pragma alloc_text (PAGE, TpmHandleTransmit)
#endif

NTSTATUS TpmGetManufacturer(PTPM_CONTEXT TpmContext)
{
    NTSTATUS                    Status;
    TPM_GET_CAPABILITY_CMD      Cmd;
    TPM_GET_MANUFACTURER_RESULT Result;
    ULONG                       ResultLen = sizeof(TPM_GET_MANUFACTURER_RESULT);

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
             "TpmGetVendorId().\n"));

    INIT_GET_MANUFACTURER_CMD(Cmd);

    Status = TpmSubmitRetry(TpmContext,
                            &Cmd.Cmd,
                            sizeof(TPM_GET_CAPABILITY_CMD),
                            &Result.Result,
                            &ResultLen,
                            TpmContext->Shortduration);
    if(NT_SUCCESS(Status)               ||
       Status == STATUS_BUFFER_OVERFLOW )
    {
        if(Result.Result.returnCode == TPM_SUCCESS)
        {
            TpmContext->Manufacturer = BIG_ENDIAN_UINT(Result.Manufacturer);
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmGetManufacturer: TpmSubmitRetry returned with TPM error: 0x%x.  Using default values\n",
                    BIG_ENDIAN_UINT(Result.Result.returnCode)));

            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmGetManufacturer: TpmSubmitRetry returned with NTSTATUS error: 0x%x \n",
                Status));
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
              "TPM Manufacturer : 0x%x\n",
              TpmContext->Manufacturer));
    return Status;
}

NTSTATUS TpmGetTimeoutInfo(PTPM_CONTEXT TpmContext)
{
    NTSTATUS                    Status;
    TPM_GET_CAPABILITY_CMD      Cmd;
    TPM_GET_TIMEOUT_INFO_RESULT Result;
    ULONG                       ResultLen = sizeof(TPM_GET_TIMEOUT_INFO_RESULT);

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmGetTimeoutInfo().\n"));

    INIT_GET_TIMEOUT_INFO_CMD(Cmd);

    Status = TpmSubmitRetry(TpmContext,
                            &Cmd.Cmd,
                            sizeof(TPM_GET_CAPABILITY_CMD),
                            &Result.Result,
                            &ResultLen,
                            TpmContext->Shortduration);

    if(NT_SUCCESS(Status)               ||
       Status == STATUS_BUFFER_OVERFLOW )
    {
        if(Result.Result.returnCode == TPM_SUCCESS)
        {
              TpmContext->AccessTimeOut         =   BIG_ENDIAN_UINT(Result.AccessTimeOut) / 1000;
              TpmContext->WaitForBitSetTime     =   BIG_ENDIAN_UINT(Result.WaitForBitSetTime) / 1000;
              TpmContext->WriteCommandTimeOut   =   BIG_ENDIAN_UINT(Result.WriteCommandTimeOut) / 1000;
              TpmContext->BurstValueTimeOut     =   BIG_ENDIAN_UINT(Result.BurstValueTimeOut) / 1000;

            if(TpmContext->AccessTimeOut < TIS_SHORT_TIMEOUT)
                TpmContext->AccessTimeOut = TIS_SHORT_TIMEOUT;
            if(TpmContext->WriteCommandTimeOut < TIS_SHORT_TIMEOUT)
                TpmContext->WriteCommandTimeOut = TIS_SHORT_TIMEOUT;
            if(TpmContext->BurstValueTimeOut < TIS_SHORT_TIMEOUT)
                TpmContext->BurstValueTimeOut = TIS_SHORT_TIMEOUT;
            if(TpmContext->WaitForBitSetTime < TIS_MEDIUM_TIMEOUT)
                TpmContext->WaitForBitSetTime = TIS_MEDIUM_TIMEOUT;
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmGetTimeoutInfo: TpmSubmitRetry returned with TPM error: 0x%x.  Using default values\n",
                    BIG_ENDIAN_UINT(Result.Result.returnCode)));
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmGetTimeoutInfo: TpmSubmitRetry returned with NTSTATUS error: 0x%x \n",
                Status));
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
              "TPM Timeouts in ms: A: %u B: %u C: %u D: %u\n",
              TpmContext->AccessTimeOut,
              TpmContext->WaitForBitSetTime,
              TpmContext->WriteCommandTimeOut,
              TpmContext->BurstValueTimeOut));
    return Status;
}

NTSTATUS TpmGetDurations(PTPM_CONTEXT TpmContext)
{
    NTSTATUS                    Status;
    TPM_GET_CAPABILITY_CMD      Cmd;
    TPM_GET_DURATIONS_RESULT    Result;
    ULONG                       ResultLen = sizeof(TPM_GET_DURATIONS_RESULT);

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
             "TpmGetDurations().\n"));

    INIT_GET_DURATIONS_CMD(Cmd);

    Status = TpmSubmitRetry(TpmContext,
                            &Cmd.Cmd,
                            sizeof(TPM_GET_CAPABILITY_CMD),
                            &Result.Result,
                            &ResultLen,
                            TpmContext->Shortduration);

    if(NT_SUCCESS(Status)               ||
       Status == STATUS_BUFFER_OVERFLOW )
    {
        if(Result.Result.returnCode == TPM_SUCCESS)
        {
            TpmContext->Shortduration = BIG_ENDIAN_UINT(Result.Shortduration) / 1000;
            TpmContext->Mediumduration= BIG_ENDIAN_UINT(Result.Mediumduration) / 1000;
            TpmContext->Longduration  = BIG_ENDIAN_UINT(Result.Longduration) / 1000;
            if(TpmContext->Shortduration < TIS_SHORT_TIMEOUT)
                TpmContext->Shortduration = TIS_SHORT_TIMEOUT;
            if(TpmContext->Mediumduration < TIS_MEDIUM_TIMEOUT)
                TpmContext->Mediumduration = TIS_MEDIUM_TIMEOUT;
            if(TpmContext->Longduration < TIS_LONG_TIMEOUT)
                TpmContext->Longduration = TIS_LONG_TIMEOUT;
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmGetDurations: TpmSubmitRetry return with TPM error: 0x%x.  Using default values.\n",
                    BIG_ENDIAN_UINT(Result.Result.returnCode)));
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmGetDurations: TpmSubmitRetry returned with NTSTATUS error: 0x%x \n",
                Status));
    }
    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TPM durations in ms: Short: %u Medium: %u Long: %u\n",
            TpmContext->Shortduration,
            TpmContext->Mediumduration,
            TpmContext->Longduration));
    return Status;
}

NTSTATUS TpmActivateCurrentLocality(PTPM_CONTEXT TpmContext,PUCHAR locality)
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BOOLEAN		bVal = FALSE;

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmActivateCurrentLocality: Enter with ACCESS register set at 0x%02x\n",
            (*locality)));

    if(TpmContext->AccessRegister &&
       ((*locality) & TPM_ACCESS_SERIZED))
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmActivateCurrentLocality: seized bit set\n"));

        TpmLogEvent(0xC006000E,
                        WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                        (*locality),
                        9);
    }

    if((~(*locality)) & TPM_ACCESS_ACTIVE_LOCALITY) // TPM_ACCESS_ACTIVE_LOCALITY
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmActivateCurrentLocality: Activating Locality #0.\n"));

        if(TpmContext->AccessRegister)
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmActivateCurrentLocality: activeLocality bit cleared\n"));

            TpmLogEvent(0xC006000E,
                        WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                        (*locality),
                        10);
        }

        TpmOUTB(TpmContext, TPM_ACCESS(0), TPM_ACCESS_REQUEST_USE);

        Status = TpmWaitForBitSet(TpmContext,
                                  TPM_ACCESS(0),
                                  TPM_ACCESS_ACTIVE_LOCALITY,
                                  TpmContext->AccessTimeOut,
                                  locality);
        if(NT_SUCCESS(Status))
        {
            Status = TpmVerifyAccessRegister(TpmContext,
                                             (*locality),
                                             1);
            if(NT_SUCCESS(Status))
                TpmContext->AccessRegister = TRUE;
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmActivateCurrentLocality:  Failed to set locality bit for locality #0 (0x%02x)\n",
                    (*locality)));

            TpmLogEvent(0xC006000E,
                        WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                        (*locality),
                        11);
        }
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
             "TpmActivateCurrentLocality: Exiting with status 0x%x\n", Status));

    return Status;
}

NTSTATUS TpmSaveState(PTPM_CONTEXT TpmContext)
{
    NTSTATUS            Status;
    TPM_CMD_BUFFER      SaveState;
    TPM_RESULT_BUFFER   CmdReturn;
    ULONG               RetLen = sizeof(TPM_RESULT_BUFFER);

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmSaveState called - entry to Sleep state -issuing TPM_SaveState().\n"));

    INIT_CMD_BUFFER(SaveState,TPM_ORD_SaveState);

    Status = TpmSubmitRetry(TpmContext,
                            &SaveState,
                            sizeof(TPM_CMD_BUFFER),
                            &CmdReturn,
                            &RetLen,
                            TpmContext->Mediumduration);

    if(NT_SUCCESS(Status))
    {
        if(CmdReturn.returnCode != TPM_SUCCESS)
        {
            Status = STATUS_DEVICE_PROTOCOL_ERROR;
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmSaveState: TpmSubmitRetry failed with TPM error 0x%x.\n",
                    CmdReturn.returnCode));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmSaveState: TpmSubmitRetry failed with Status 0x%x\n",
                Status));
    }
    return Status;
}

NTSTATUS TpmGetNextBurstValue(PTPM_CONTEXT TpmContext,USHORT* BurstValue,ULONG TimeOut)
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT    Value;
    int       OffLen = (int)TimeOut;

    *BurstValue = 0;

    while(TRUE)
    {
        Value =  TpmINW(TpmContext,TPM_STS(0) + 1) & 0xFFFF;

		*BurstValue = Value;

        if(Value != 0 && Value != 0xFFFF)
			break;

        Status = TpmPollDelay(TpmContext);
        if(!NT_SUCCESS(Status)) break;

        OffLen -= 10;
        if(OffLen < 0) return STATUS_DEVICE_PROTOCOL_ERROR; //0xC0000186;
    }

    return Status;
}

NTSTATUS TpmCheckStatusReady(PTPM_CONTEXT TpmContext)
{
    NTSTATUS    Status;
    UCHAR       BitValue = 0;
    ULONG       Index = 0;
    do
    {
        TpmOUTB(TpmContext,TPM_STS(0),TPM_STS_COMMAND_READY);

        Status = TpmWaitForBitSet(TpmContext,
                                  TPM_STS(0),
                                  TPM_STS_COMMAND_READY,
                                  TpmContext->WaitForBitSetTime,
                                  &BitValue);

        if(Status != STATUS_DEVICE_PROTOCOL_ERROR)
            break;

		Index++;
    } 
	while(Index < 2);

    if(!NT_SUCCESS(Status) && Status != STATUS_CANCELLED)
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmCheckStatusReady: Start failed! (%02x).\n",BitValue));

        TpmLogEvent(0xC006000F,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    Status,
                    0);
    }

    return Status;
}

NTSTATUS TpmSetDefaultTimingValues(PTPM_CONTEXT TpmContext)
{

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "Setting default timing values.\n"));

    TpmContext->AccessTimeOut = \
    TpmContext->WriteCommandTimeOut = \
    TpmContext->BurstValueTimeOut = TIS_SHORT_TIMEOUT;

    TpmContext->WaitForBitSetTime = TIS_MEDIUM_TIMEOUT;
    TpmContext->Shortduration  = TIS_SHORT_TIMEOUT;
    TpmContext->Mediumduration = TIS_MEDIUM_TIMEOUT;
    TpmContext->Longduration   = TIS_LONG_TIMEOUT;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TPM durations in ms: Short: %u Medium: %u Long: %u\n",
            TpmContext->Shortduration,
            TpmContext->Mediumduration,
            TpmContext->Longduration));

    return STATUS_SUCCESS;
}

NTSTATUS TpmPollDelay(PTPM_CONTEXT TpmContext)
{
    LARGE_INTEGER Interval;

    if(TpmContext->TpmState != StAborting		&&
       TpmContext->TpmState != StSuspendPending	)
    {
         Interval.HighPart = -1;
         Interval.LowPart = -100 * 1000;
         return KeDelayExecutionThread(0, 0, &Interval);
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmPollDelay: Cancel flag was set! Bailing out.\n"));
        return STATUS_CANCELLED; // 0xC0000120;
    }
}

UCHAR Vendor1INB(PTPM_CONTEXT TpmContext,ULONG Offset)
{
    ULONG    Index = 0;
    PUCHAR   PortAddr = (PUCHAR)TpmContext->PortAddr;
    if(Offset)
    {
        WRITE_PORT_UCHAR(PortAddr + 4,(UCHAR)(Offset & 0xFF));
        WRITE_PORT_UCHAR(PortAddr + 8,(UCHAR)(((Offset) >> 8) & 0xFF));
        WRITE_PORT_UCHAR(PortAddr + 12,1);
        while(Index < 2000)
        {
            if( !(READ_PORT_UCHAR(PortAddr + 12) & 2) )
            {
                return READ_PORT_UCHAR(PortAddr);
            }
            Index++;
        }
        return 0;
    }
    else
    {
        return 160;
    }
}

UCHAR TpmINB(PTPM_CONTEXT TpmContext,ULONG Offset)
{
    if(TpmContext->UsePortBasedIO)
    {
        if(TpmContext->UsePortBasedIO == 1)
            return Vendor1INB(TpmContext,Offset);
    }
    else
    {
        return TpmContext->MemAddr[Offset];
    }
    return 0;
}

VOID Vendor1OUTB(PTPM_CONTEXT TpmContext,ULONG Offset,UCHAR Value)
{
    ULONG    Index = 0;
    PUCHAR   PortAddr = TpmContext->PortAddr;

    WRITE_PORT_UCHAR(PortAddr + 4,(UCHAR)(Offset & 0xFF));
    WRITE_PORT_UCHAR(PortAddr + 8,(UCHAR)(((Offset) >> 8) & 0xFF));
    WRITE_PORT_UCHAR(PortAddr,Value);
    WRITE_PORT_UCHAR(PortAddr + 12,5);
    while(Index < 2000)
    {
        if( !(READ_PORT_UCHAR(PortAddr + 12) & 2) )
        {
            break;
        }
        Index++;
    }
}

VOID  TpmOUTB(PTPM_CONTEXT TpmContext,ULONG Offset,UCHAR Value)
{
    if(TpmContext->UsePortBasedIO)
    {
        if(TpmContext->UsePortBasedIO == 1)
            Vendor1OUTB(TpmContext,Offset,Value);
    }
    else
    {
        TpmContext->MemAddr[Offset] = Value;
    }
}

USHORT Vendor1INW(PTPM_CONTEXT TpmContext,ULONG Offset)
{
    SHORT Value = Vendor1INB(TpmContext, Offset + 1) << 8;
    Value += Vendor1INB(TpmContext, Offset);
    return Value;
}

USHORT TpmINW(PTPM_CONTEXT TpmContext,ULONG Offset)
{
    USHORT ret = 0;
    if(TpmContext->UsePortBasedIO)
    {
        if(TpmContext->UsePortBasedIO == 1)
            ret = Vendor1INW(TpmContext,Offset);
    }
    else
    {
        ret = *(USHORT*)(((ULONG)TpmContext->MemAddr) + Offset);
    }
    return ret;
}

NTSTATUS TpmContinueSelfTest(PTPM_CONTEXT TpmContext)
{
    NTSTATUS            Status;
    TPM_CMD_BUFFER      Cmd;
    ULONG               CmdLen = sizeof(TPM_CMD_BUFFER);
    TPM_RESULT_BUFFER   Result = { 0 };
    ULONG               ResultSize = sizeof(TPM_RESULT_BUFFER);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmContinueSelfTest().\n"));

    INIT_CMD_BUFFER(Cmd,TPM_ORD_ContinueSelfTest);

    Status = TpmSubmitRetry(TpmContext,
                            &Cmd,
                            CmdLen,
                            &Result,
                            &ResultSize,
                            TpmContext->Longduration);

    if(!NT_SUCCESS(Status))
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmContinueSelfTest: TpmSubmitRetry failed with Status 0x%x\n",Status));
        return Status;
    }

    if(Result.returnCode != TPM_SUCCESS)
    {
        Status = STATUS_DEVICE_PROTOCOL_ERROR;

        TpmLogEvent(0xC0060002,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    Result.returnCode,
                    0);

        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmContinueSelfTest: TpmSubmitRetry failed with TPM error 0x%x\n",
            Result.returnCode));
    }

    return Status;
}

NTSTATUS TpmSubmitRetry(PTPM_CONTEXT            TpmContext,
                        IN PTPM_CMD_BUFFER      InputBuffer,
                        IN ULONG                InputSize,
                        IN PTPM_RESULT_BUFFER   OutputBuffer,
                        IN OUT PULONG           OutputSize,
                        IN ULONG                duration)
{
    NTSTATUS    Status;
    ULONG       retStatus;
    ULONG       i = 0;
    ULONG       ByteSize = *OutputSize;
    PTPM_RESULT_BUFFER Result = (PTPM_RESULT_BUFFER)OutputBuffer;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmSubmitRetry().\n"));
    while(TRUE)
    {
        *OutputSize = ByteSize;
        RtlZeroMemory(OutputBuffer,ByteSize);
        Status = TpmSubmit(TpmContext,
                           InputBuffer,
                           InputSize,
                           OutputBuffer,
                           OutputSize,
                           duration);
        if(!NT_SUCCESS(Status)) break;

        if(BIG_ENDIAN_UINT(Result->returnCode) == TPM_E_NON_FATAL ||
           BIG_ENDIAN_UINT(Result->returnCode) == TPM_E_DOING_SELFTEST)
        {
            TpmPollDelay(TpmContext);
            if(++i < 3)
                continue;
        }
        return Status;
    }
    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmSubmitRetry: TpmSubmit failed with Status 0x%x\n",
                Status));
    return Status;
}

NTSTATUS TpmSubmit(IN PTPM_CONTEXT          TpmContext,
                   IN PTPM_CMD_BUFFER       InputBuffer,
                   IN ULONG                 InputSize,
                   IN PTPM_RESULT_BUFFER    OutputBuffer,
                   IN OUT PULONG            OutputSize,
                   IN ULONG                 duration)
{
    UCHAR       Locality;
    NTSTATUS    Status;

    PTPM_CMD_BUFFER CmdBuffer = (PTPM_CMD_BUFFER)InputBuffer;
    PTPM_RESULT_BUFFER Result = (PTPM_RESULT_BUFFER)OutputBuffer;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmSubmit: Enter\n"));

    if(!InputSize || !*OutputSize )
    {
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    if(InputSize < sizeof(TPM_CMD_BUFFER))
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmSubmit input buffer length (%Iu) too small\n", InputSize));
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    if(*OutputSize < sizeof(TPM_RESULT_BUFFER))
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmSubmit output buffer length (%Iu) too small\n", *OutputSize));
        Status = STATUS_BUFFER_TOO_SMALL;
        return Status;
    }

    if(BIG_ENDIAN_UINT(CmdBuffer->paramSize) != InputSize)
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmSubmit input buffer length (%Iu) does not match length embedded in command (%u)\n",
                InputSize,
                BIG_ENDIAN_UINT(CmdBuffer->paramSize)));
        Status = STATUS_INVALID_PARAMETER;
        return Status;
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_TRACE_LEVEL,
			"Input Command:\n"));

    TpmDumpBuffer((PUCHAR)InputBuffer, InputSize, 2);

    Locality = TpmINB(TpmContext, TPM_ACCESS(0));

    Status = TpmVerifyAccessRegister(TpmContext,
                                     Locality,
                                     1);
    if(NT_SUCCESS(Status))
    {
        Status = TpmActivateCurrentLocality(TpmContext, &Locality);

        if(!NT_SUCCESS(Status))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmActivateCurrentLocality failed with Status 0x%x\n",
                    Status));
            return Status;
        }

        Status = TpmCheckStatusReady(TpmContext);

        if(!NT_SUCCESS(Status))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmCheckStatusReady failed with Status 0x%x\n",
                    Status));
            return Status;
        }

        Status = TpmWriteCommandAndStart(TpmContext, InputSize, (PUCHAR)InputBuffer);

        if(!NT_SUCCESS(Status))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmWriteCommandAndStart failed with Status 0x%x\n",
                    Status));

            return Status;
        }

        Status = TpmGetCommandResult(TpmContext, OutputSize, (PUCHAR)OutputBuffer, duration);

        if((*OutputSize) != 0               ||
           (Status != STATUS_SUCCESS && Status == STATUS_BUFFER_OVERFLOW ))
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, 2,
                "Data Returned:\n"));

            TpmDumpBuffer((PUCHAR)OutputBuffer, *OutputSize, 2);
        }

        if(!NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW) //0x80000005)
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmGetCommandResult failed with Status 0x%x\n",
                    Status));

            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmSubmit exiting with NTSTATUS failure: 0x%x\n",
                    Status));

            return Status;
        }
        else if(Result->returnCode == TPM_SUCCESS)
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                        "TpmSubmit exiting with TPM failure: 0x%x\n",
                        BIG_ENDIAN_UINT(Result->returnCode)));
        }
    }
    return Status;
}

NTSTATUS TpmProvideEntropy(PTPM_CONTEXT TpmContext)
{
    NTSTATUS            status = STATUS_SUCCESS;
    TPM_RANDOM_CMD      Random;
    ULONG               CmdLen = sizeof(TPM_RANDOM_CMD);
    TPM_RANDOM_RESULT   Result;
    ULONG               ResultLen = sizeof(TPM_RANDOM_RESULT);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmProvideEntropy().\n"));

#if _NT_TARGET_VERSION >= 0x601
    // TPM_ORD_GetRandom
    INIT_RANDOM_CMD(Random);

    status = TpmSubmitRetry(TpmContext,
                            &Random.Cmd,
                            sizeof(TPM_RANDOM_CMD),
                            &Result.Result,
                            &ResultLen,
                            TpmContext->Shortduration);
    if(NT_SUCCESS(status))
    {
        if(Result.Result.returnCode == TPM_SUCCESS)
        {
            EntropyProvideData(TpmContext->hEntropySource,
                               (PCBYTE)Result.randomBytes,
                               BIG_ENDIAN_UINT(Result.randomBytesSize),
							   BIG_ENDIAN_UINT(Result.randomBytesSize) * TpmContext->EntropyDensity);

        }
        else if(BIG_ENDIAN_UINT(Result.Result.returnCode) != TPM_E_DEACTIVATED  &&
                BIG_ENDIAN_UINT(Result.Result.returnCode) != TPM_E_DISABLED     )
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "TpmProvideEntropy: TpmSubmitRetry failed with TPM error 0x%x",status));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmProvideEntropy: TpmSubmitRetry failed with Status 0x%x\n",status));
    }
#endif

    TpmContext->PendingEntropy = FALSE;
    return status;
}

NTSTATUS TpmHandleTransmit(WDFDEVICE Device,WDFREQUEST Request,size_t OutputBufferLength,size_t InputBufferLength)
{
    PTPM_CONTEXT    TpmContext;
    NTSTATUS        Status = STATUS_SUCCESS;
    NTSTATUS        StateStatus;
    WDFMEMORY       InputMemory = NULL;
    WDFMEMORY       OutputMemory= NULL;
    PUCHAR          InputBuffer = NULL;
    PUCHAR          OutputBuffer= NULL;
    size_t          InputSize;
    size_t          OutputSize = OutputBufferLength;

	PAGED_CODE();

    TpmContext = GetTpmContext(Device);

    Status = TpmUpdateTpmState(TpmContext,StBusy,IdThread);

    if(NT_SUCCESS(Status))
    {
        Status = WdfRequestRetrieveInputMemory(Request,&InputMemory);

        if(NT_SUCCESS(Status))
        {
            InputBuffer = WdfMemoryGetBuffer(InputMemory,&InputSize);

            Status = WdfRequestRetrieveOutputMemory(Request,&OutputMemory);

            if(NT_SUCCESS(Status))
            {
                OutputBuffer = WdfMemoryGetBuffer(OutputMemory,&OutputSize);

                KeClearEvent(&TpmContext->Event);

                Status = TpmSubmit(TpmContext,
                      (PTPM_CMD_BUFFER)InputBuffer,
                      InputBufferLength,
                      (PTPM_RESULT_BUFFER)OutputBuffer,
                      &OutputSize,
                      TpmContext->Longduration);

                KeSetEvent(&TpmContext->Event,0,FALSE);

                if(NT_SUCCESS(Status)   ||
                   Status == STATUS_BUFFER_OVERFLOW )
                {
                    WdfRequestSetInformation(Request,OutputSize);

                    if(TpmContext->PendingEntropy)
                    {
                        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                            "TpmHandleTransmit: found pending entropy request. \n"));
                        TpmProvideEntropy(TpmContext);
                    }
                }
                else
                {
                    if(OutputSize)
                        RtlZeroMemory(OutputBuffer,OutputSize);
                }
            }
        }

        if(Status != STATUS_CANCELLED) // C0000120
        {
            StateStatus = TpmUpdateTpmState(TpmContext,StAvailable,IdThread);

            if(NT_SUCCESS(Status) && !NT_SUCCESS(StateStatus))
            {
                Status = StateStatus;
            }
        }
    }

    return Status;
}

NTSTATUS TpmVerifyAccessRegister(PTPM_CONTEXT TpmContext,UCHAR BitSet,UCHAR Value)
{
    NTSTATUS Status = STATUS_SUCCESS;

	PAGED_CODE();

    if(BitSet == 0xFF)
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmVerifyAccessRegister: The TPM is in shutdown mode!\n"));

        TpmLogEvent(0xC006000E,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    0xFF,
                    7);
    }
    else
    {
        if(Value)
        {
            if((~BitSet)  & TPM_ACCESS_VALID)
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                        "TpmVerifyAccessRegister: The Access register's Valid Status bit is not set! (0x%02x)\n",
                        BitSet));

                TpmLogEvent(0xC006000E,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    BitSet,
                    8);
            }
            else
            {
                if(!TpmContext->SkipAccessRegisterCheck)
                {
                    if(BitSet & TPM_ACCESS_ILLEGAL)
                    {
                        Status = STATUS_INVALID_DEVICE_STATE;
                        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                                "TpmVerifyAccessRegister: illegal bits set in 0x%02x!\n", BitSet));
                    }
                }
            }
        }
    }

    return Status;
}

NTSTATUS TpmWriteCommandAndStart(PTPM_CONTEXT TpmContext,ULONG InputSize,PUCHAR InputBuffer)
{
    NTSTATUS        Status;
    ULONG           DataLeft;
    USHORT          BurstValue = 0;
    ULONG           Offset = 0;
    UCHAR           Bitset = 0;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmWriteCommandAndStart: Enter\n"));

	DataLeft = InputSize;

    do
    {
        Status = TpmGetNextBurstValue(TpmContext,&BurstValue, TpmContext->WriteCommandTimeOut);

        if(!NT_SUCCESS(Status))
        {
			TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_COMMAND_READY);

            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmWriteCommandAndStart: failed to get next burst value.\n"));

            goto ErrorExit;
        }

        if(BurstValue <= DataLeft)
		{
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmWriteCommandAndStart: writing burst of %Iu bytes.\n", BurstValue));
		}
		else
		{
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "TpmWriteCommandAndStart: writing DataLeft of %Iu bytes., BurstValue : %Iu\n", DataLeft , DataLeft));
		}

		while(BurstValue)
		{
			if ( Offset >= (InputSize - 1 ))
				break;
			
			TpmOUTB(TpmContext, TPM_DATA_FIFO(0), InputBuffer[Offset]);
			
			BurstValue --;

			DataLeft --;
			
			++Offset;
		}

    } while(BurstValue == 0); // left last byte to exit

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmWriteCommandAndStart: checking for overflow. DataLeft -> %d bytes, BurstValue -> %d \n",DataLeft, BurstValue));

    Status = TpmWaitForBitSet(
         TpmContext,
         TPM_STS(0), //0x18u,
         TPM_STS_VALID | TPM_STS_DATA_EXPECT, //0x88u,
         TpmContext->WriteCommandTimeOut + (TpmContext->DelayExpectBitCheck ? 0x64 : 0),
         &Bitset);

    if(NT_SUCCESS(Status)       ||
       Status == STATUS_CANCELLED) //0xC0000120 )
    {
		// write last data byte
		if(BurstValue)
			TpmOUTB(TpmContext, TPM_DATA_FIFO(0), InputBuffer[Offset]);

        Status = TpmWaitForBitSet(TpmContext, TPM_STS(0), TPM_STS_VALID,
                                  TpmContext->WriteCommandTimeOut,
                                  &Bitset);
        if(NT_SUCCESS(Status))
        {

            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "TpmWriteCommandAndStart: checking for underflow.\n"));

            if(TpmContext->SkipUnderflowCheck ||
               !(Bitset & TPM_STS_DATA_EXPECT) )
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                        "TpmWriteCommandAndStart: set TPM_STATUS_GO request bit\n"));

                TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_GO);
            }
            else
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                        "TpmWriteCommandAndStart: underflow detected.\n"));

                TpmLogEvent(0xC006000F,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    Status,
                    2);

                TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_COMMAND_READY);
            }
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                    "TpmWriteCommandAndStart: failed waiting for valid bit in status register (0x%02x).\n",
                    Bitset));
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "TpmWriteCommandAndStart: overflow detected.\n"));

        TpmLogEvent(0xC006000F,
                    WdfDeviceWdmGetDeviceObject(TpmContext->Device),
                    Status,
                    1);

        TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_COMMAND_READY);
    }

ErrorExit:
    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
             "TpmWriteCommandAndStart: Exiting with status 0x%x\n", Status));

    return Status;
}

NTSTATUS TpmGetCommandResultLoop(PTPM_CONTEXT TpmContext,
                             PULONG OutputSize,
                             PUCHAR OutputBuffer,
                             ULONG duration)
{
    NTSTATUS        Status;
    UCHAR           Bitset;
    USHORT          BurstValue = 0;
    ULONG           i;
	ULONG			LoopLen = sizeof(TPM_RESULT_BUFFER);
	PTPM_RESULT_BUFFER	Result = (PTPM_RESULT_BUFFER)OutputBuffer;
	ULONG			Error;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmGetCommandResultLoop: Enter\n"));

    Bitset	= 0;
    i		= 0;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmGetCommandResultLoop: waiting %ums.\n",
            duration));

    Status = TpmWaitForBitSet(TpmContext,
                              TPM_STS(0),
                              TPM_STS_VALID | TPM_STS_DATA_AVAIL, //0x90,
                              duration,
                              &Bitset);
    if(NT_SUCCESS(Status))
    {
		// loc_11659
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmGetCommandResultLoop: reading response.\n"));
		
		do 
		{
			// loc_11667
			Status = TpmWaitForBitSet(TpmContext,
				TPM_STS(0),
				TPM_STS_VALID | TPM_STS_DATA_AVAIL, //0x90,
				TpmContext->WriteCommandTimeOut,
				&Bitset);

			if(!NT_SUCCESS(Status))
			{
				// loc_117A3
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
					"TpmGetCommandResultLoop: Timeout while reading response (status 0x%02x).\n",
					Bitset));
				// loc_118AF
				break;
			}

			// loc_11686
			Status = TpmGetNextBurstValue(TpmContext,
										&BurstValue,
										TpmContext->BurstValueTimeOut);

			if(!NT_SUCCESS(Status))
			{
				// loc_117BB loc_117AD
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
					"TpmGetCommandResultLoop: TpmGetNextBurstValue failed with Status 0x%x.\n",
					Status));
				// loc_118AF
				break;
			}

			// loc_1169E
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
					"TpmGetCommandResultLoop: reading burst of %u bytes.\n",
					BurstValue));
			
			if(BurstValue < sizeof(TPM_RESULT_BUFFER))
			{
				Status = STATUS_DEVICE_PROTOCOL_ERROR;

				TpmLogEvent(0xC006000F,
					WdfDeviceWdmGetDeviceObject(TpmContext->Device),
					STATUS_DEVICE_PROTOCOL_ERROR, //0xC0000186,
					3);
				
				// loc_118AF
				break;
			}

			// loc_11749 read BurstValue - 1 bytes
			while(BurstValue && i < (LoopLen - 1))
			{
				// loc_116C2
				OutputBuffer[i] = TpmINB(TpmContext,TPM_DATA_FIFO(0));

				i++;
				BurstValue --;

				if(i == FIELD_OFFSET(TPM_RESULT_BUFFER,paramSize))
				{
					if( OutputBuffer[0] != 0 )
					{

						KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
							"TpmGetCommandResultLoop: response size %u claims to be only %u.\n",
							LoopLen,
							(*OutputSize)));
					
						TpmLogEvent(0xC006000F,
							WdfDeviceWdmGetDeviceObject(TpmContext->Device),
							STATUS_DEVICE_PROTOCOL_ERROR, //0xC0000186,
							4);
					
						// loc_118AF
						if(TpmContext->TpmState == StAborting || TpmContext->TpmState == StSuspendPending)
							Status = STATUS_CANCELLED; // 0xC0000120;
					
						KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
							"TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));
					
						return Status;
					}
				}

				if(i == FIELD_OFFSET(TPM_RESULT_BUFFER,returnCode))
				{
					LoopLen = BIG_ENDIAN_UINT(Result->paramSize) & 0xFFFF;

					// 116E4
					if(LoopLen > (*OutputSize))
					{
						KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
							"TpmGetCommandResultLoop: output buffer length (%Iu) too small (%u) needed).\n",
							*OutputSize,
							BIG_ENDIAN_UINT(Result->paramSize)));
					}
					// loc_1172A
				}
				
				if( i > ((*OutputSize) - 1) )
				{
					Status = STATUS_BUFFER_OVERFLOW;

					// loc_117C3
					KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
						"TpmGetCommandResultLoop: response size %u claims to be only %u.\n",
						LoopLen,
						(*OutputSize)));

					Error = (LoopLen << 16) | (*OutputSize);
					
					TpmLogEvent(Error, //0xC006000F,
						WdfDeviceWdmGetDeviceObject(TpmContext->Device),
						STATUS_DEVICE_PROTOCOL_ERROR, //0xC0000186,
						4);
					
					// loc_118AF
					if(TpmContext->TpmState == StAborting || TpmContext->TpmState == StSuspendPending)
						Status = STATUS_CANCELLED; // 0xC0000120;
					
					KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
						"TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));
					
					return Status;
				}
				
				// loc_1172A
				if(	TpmContext->TpmState == StAborting		|| 
					TpmContext->TpmState == StSuspendPending	)
				{
					// loc_118AF
					Status = STATUS_CANCELLED; // 0xC0000120;
					
					KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
						"TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));
					
					return Status;	
				}
				
			}

			if(BurstValue == 0) continue;

			// 1175F
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
				"TpmGetCommandResultLoop: checking for underflow.\n"));
			
			Status = TpmWaitForBitSet(TpmContext,
				TPM_STS(0),
				TPM_STS_VALID | TPM_STS_DATA_AVAIL, //0x90,
				TpmContext->WriteCommandTimeOut,
				&Bitset);
			
			if(!NT_SUCCESS(Status))
			{
				// loc_11785
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
					"TpmGetCommandResultLoop: TPM has fewer bytes than expected.\n"));

				// loc_11795 // loc_117DF
				TpmLogEvent(0xC006000F,
					WdfDeviceWdmGetDeviceObject(TpmContext->Device),
					STATUS_DEVICE_PROTOCOL_ERROR, //0xC0000186,
					5);

				// loc_118AF
				if(TpmContext->TpmState == StAborting || TpmContext->TpmState == StSuspendPending)
					Status = STATUS_CANCELLED; // 0xC0000120;
				
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
					"TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));
				
				return Status;
			}

			// loc_117FB
			if(i < (*OutputSize))
			{
				OutputBuffer[i] = TpmINB(TpmContext,TPM_DATA_FIFO(0));
				i++;
			}

			// loc_1181E
			if(BIG_ENDIAN_UINT(Result->paramSize) > (*OutputSize))
			{
				Status = STATUS_BUFFER_OVERFLOW; // 0x80000005;

				// loc_118AF
				if(TpmContext->TpmState == StAborting || TpmContext->TpmState == StSuspendPending)
					Status = STATUS_CANCELLED; // 0xC0000120;
				
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
					"TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));
				
				return Status;
			}

			// loc_1184A
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
				"TpmGetCommandResultLoop: checking for overflow.\n"));
			
			Status = TpmWaitForBitSet(TpmContext,
				TPM_STS(0),
				TPM_STS_VALID,
				TpmContext->WriteCommandTimeOut,
				&Bitset);

			if(!NT_SUCCESS(Status))
			{
				// 11873
				// loc_117AD
				KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
					"TpmGetCommandResultLoop: failed to get final status (0x%02x).\n",
					Bitset));
				// loc_118AF
				break;
			}
			
			// loc_11882
			if( TpmContext->SkipOverflowCheck ||
				!(Bitset & TPM_STS_DATA_AVAIL))
			{
				// loc_118A3
				*OutputSize = i;
				Status = STATUS_SUCCESS;
				break;
			}

			// 1188E
			KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
				"TpmGetCommandResultLoop: TPM has more bytes than expected.\n"));

			// loc_11795 // loc_117DF
			TpmLogEvent(0xC006000F,
				WdfDeviceWdmGetDeviceObject(TpmContext->Device),
				STATUS_DEVICE_PROTOCOL_ERROR, //0xC0000186,
				6);

			// loc_118AF
			break;
			
		} while (TRUE);
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmGetCommandResultLoop: Timeout while waiting for the response\n"));
        // loc_118AF
    }

	// loc_118AF
    if(TpmContext->TpmState == StAborting || TpmContext->TpmState == StSuspendPending)
        Status = STATUS_CANCELLED; // 0xC0000120;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmGetCommandResultLoop: Exiting with status 0x%x\n", Status));

    return Status;
}

NTSTATUS TpmGetCommandResult(PTPM_CONTEXT TpmContext,
                             PULONG OutputSize,
                             PUCHAR OutputBuffer,
                             ULONG duration)
{
    NTSTATUS     Status;
    ULONG        i = 0;
    ULONG        OutputLength;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmGetCommandResult: Enter\n"));

    do
    {
        OutputLength = *OutputSize;

        Status = TpmGetCommandResultLoop(TpmContext,
                                         &OutputLength,
                                         OutputBuffer,
                                         duration);

        if ( Status != STATUS_DEVICE_PROTOCOL_ERROR )
            break;

        if( i < 4)
            TpmOUTB(TpmContext, TPM_STS(0), TPM_ACCESS_REQUEST_USE);

		i++;

    } while( i < 5 );

	*OutputSize = OutputLength;

    TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_COMMAND_READY);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmGetCommandResult: Exiting with status 0x%x\n", Status));

    return Status;
}

NTSTATUS TpmAbort(PTPM_CONTEXT TpmContext, TPM_STATE TpmOldState, TPM_STATE TpmNewState, TPM_THREAD_ID ThreadId)
{
    NTSTATUS        Status;
    LARGE_INTEGER   Timeout;

	PAGED_CODE();
	
    Timeout.LowPart  = 0;
    Timeout.HighPart = 0;
	
    Status = TpmUpdateTpmState(TpmContext,TpmNewState,ThreadId);
	
    if(NT_SUCCESS(Status))
    {
        if ( !KeReadStateEvent(&TpmContext->Event) )
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
				"TpmAbort: writing a '1' to TPM_STS_1.commandReady to abort the running operation.\n"));
			
            TpmOUTB(TpmContext, TPM_STS(0), TPM_STS_COMMAND_READY);
            Timeout.HighPart = -1;
            Timeout.LowPart = -100*1000*1000;
            KeWaitForSingleObject(&TpmContext->Event,Executive,KernelMode,FALSE,&Timeout);
        }
        Status = TpmUpdateTpmState(TpmContext, TpmOldState, ThreadId);
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
			"TpmAbort: TpmUpdateTpmState returned NTSTATUS 0x%x\n", Status));
    }
    return Status;
}

NTSTATUS TpmWaitForBitSet(PTPM_CONTEXT TpmContext,ULONG Register,UCHAR Value,int TimeOut,OUT PUCHAR Bitset)
{
    NTSTATUS    Status = STATUS_DEVICE_PROTOCOL_ERROR;
    UCHAR       i;
	int			iTimeOut = TimeOut;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
		"TpmWaitForBit: waiting for %ums for bit(s) %02x to be set.\n",
		TimeOut,
		(UCHAR)Value));

    while (TRUE)
    {

		i = TpmINB(TpmContext, Register);

        *Bitset = i;

        if((Value & i) == Value)
        {
            Status = STATUS_SUCCESS;
            break;
        }

        if(iTimeOut <= 0)
            break;

        Status = TpmPollDelay(TpmContext);

        if(!NT_SUCCESS(Status)) 
			break;

        iTimeOut -= 10;
    }

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
		"TpmWaitForBit: exiting with ~%ums remaining; register value 0x%02x.\n",
		iTimeOut,
		(UCHAR)(*Bitset)));

    return Status;
}

