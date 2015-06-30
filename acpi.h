
#ifndef _ACPI_H_
#define _ACPI_H_
#pragma once

NTSTATUS TpmSetMorBitState(PTPM_CONTEXT TpmContext,UCHAR access);

NTSTATUS TPM_CallDSMMethod(IN PDEVICE_OBJECT DeviceObject,
                           IN const GUID*     Interface,
                           IN ULONG     FunsIndex,
                           IN PVOID		Arguments,
                           IN USHORT    ArgumentsSize,
                           OUT PUCHAR*  Buffer,
                           OUT PULONG   BufLen);

NTSTATUS TPM_CallDSMMethodPackageInt1(IN PDEVICE_OBJECT DeviceObject,
                                      IN const GUID*     Interface,
                                      IN ULONG     FunsIndex,
                                      IN ULONG     MinorFunsIndex,
                                      OUT PUCHAR*  Buffer,
                                      OUT PULONG   BufLen);

NTSTATUS TPM_PPICallDSMMethodPackageEmpty(IN PDEVICE_OBJECT DeviceObject,
                                          IN ULONG FunsIndex,
                                          OUT PUCHAR* Buffer,
                                          OUT PULONG  BufLen);


NTSTATUS TpmHandleSubmitAcpiCommand(IN WDFDEVICE Device,
                                    IN WDFREQUEST Request,
                                    IN size_t OutputBufferLength,
                                    IN size_t InputBufferLength);

#endif //

