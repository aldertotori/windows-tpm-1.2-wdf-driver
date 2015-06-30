#ifndef _TPM_TIS_H_
#define _TPM_TIS_H_

#pragma once

NTSTATUS TpmGetManufacturer(PTPM_CONTEXT TpmContext);

NTSTATUS TpmGetTimeoutInfo(PTPM_CONTEXT TpmContext);

NTSTATUS TpmGetDurations(PTPM_CONTEXT TpmContext);

NTSTATUS TpmActivateCurrentLocality(PTPM_CONTEXT TpmContext,PUCHAR locality);

NTSTATUS TpmSaveState(PTPM_CONTEXT TpmContext);

NTSTATUS TpmGetNextBurstValue(PTPM_CONTEXT TpmContext,USHORT* BurstValue,ULONG TimeOut);

NTSTATUS TpmCheckStatusReady(PTPM_CONTEXT TpmContext);

NTSTATUS TpmSetDefaultTimingValues(PTPM_CONTEXT TpmContext);

NTSTATUS TpmPollDelay(PTPM_CONTEXT TpmContext);

UCHAR Vendor1INB(PTPM_CONTEXT TpmContext,ULONG Offset);

UCHAR TpmINB(PTPM_CONTEXT TpmContext,ULONG Offset);

VOID Vendor1OUTB(PTPM_CONTEXT TpmContext,ULONG Offset,UCHAR Value);

VOID  TpmOUTB(PTPM_CONTEXT TpmContext,ULONG Offset,UCHAR Value);

USHORT Vendor1INW(PTPM_CONTEXT TpmContext,ULONG Offset);

USHORT TpmINW(PTPM_CONTEXT TpmContext,ULONG Offset);

NTSTATUS TpmContinueSelfTest(PTPM_CONTEXT TpmContext);

NTSTATUS TpmSubmitRetry(PTPM_CONTEXT            TpmContext,
                        IN PTPM_CMD_BUFFER      InputBuffer,
                        IN ULONG                InputSize,
                        IN PTPM_RESULT_BUFFER   OutputBuffer,
                        IN OUT PULONG           OutputSize,
                        IN ULONG                duration);

NTSTATUS TpmSubmit(IN PTPM_CONTEXT          TpmContext,
                   IN PTPM_CMD_BUFFER       InputBuffer,
                   IN ULONG                 InputSize,
                   IN PTPM_RESULT_BUFFER    OutputBuffer,
                   IN OUT PULONG            OutputSize,
                   IN ULONG                 duration);

NTSTATUS TpmProvideEntropy(PTPM_CONTEXT TpmContext);

NTSTATUS TpmHandleTransmit(WDFDEVICE Device,
                           WDFREQUEST Request,
                           size_t OutputBufferLength,
                           size_t InputBufferLength);

NTSTATUS TpmVerifyAccessRegister(PTPM_CONTEXT TpmContext,UCHAR BitSet,UCHAR Value);

NTSTATUS TpmWriteCommandAndStart(PTPM_CONTEXT TpmContext,
                                 ULONG InputSize,
                                 PUCHAR InputBuffer);

NTSTATUS TpmGetCommandResult(PTPM_CONTEXT TpmContext,
                             PULONG OutputSize,
                             PUCHAR OutputBuffer,
                             ULONG duration);

#endif
