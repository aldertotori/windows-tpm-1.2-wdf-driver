
#include "precomp.h"
#include "guid.h"
#include <acpiioct.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, TPM_CallDSMMethod)
#pragma alloc_text (PAGE, TPM_CallDSMMethodPackageInt1)
#pragma alloc_text (PAGE, TPM_PPICallDSMMethodPackageEmpty)
#pragma alloc_text (PAGE, TpmHandleSubmitAcpiCommand)
#pragma alloc_text (PAGE, TpmSetMorBitState)
#endif

NTSTATUS TpmSetMorBitState(PTPM_CONTEXT TpmContext,UCHAR access)
{
    NTSTATUS                    Status;
    PUCHAR                      Buffer = NULL;
    ULONG                       BufLen = 0;
	PDEVICE_OBJECT				DeviceObject;

	PAGED_CODE();

    DeviceObject =
            WdfDeviceWdmGetAttachedDevice(TpmContext->Device);
    if(DeviceObject)
    {
        Status = TPM_CallDSMMethodPackageInt1(DeviceObject,
                                              &TCG_PLATFORM_ATTACK_MITIGATION_GUID,
                                              1,
                                              access ? 1 : 0,
                                              &Buffer,
                                              &BufLen);
        if(NT_SUCCESS(Status))
        {
            if(BufLen >= sizeof(ULONG))
            {
                Status = ((*Buffer)) != 0 ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
            }
            else
            {
                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                      "TpmSetMorBitState: Too few bytes returned (%d)\n",BufLen));
                Status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                      "TpmSetMorBitState failed with status 0x%x\n",Status));
        }

        if(Buffer) ExFreePoolWithTag(Buffer,0);

    }
    else
    {
        Status = STATUS_INVALID_PARAMETER;
    }
    return Status;
}

#define ACPI_DSM_NAME   'MSD_'
#define ACPI_METHOD_DSM_REVISION    1

#ifndef ACPI_METHOD_ARGUMENT_PACKAGE
#define ACPI_METHOD_ARGUMENT_PACKAGE	0x3
#endif

#ifndef ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE
#define ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE            'CieA'
#endif

#ifndef ACPI_METHOD_SET_ARGUMENT_BUFFER
#define ACPI_METHOD_SET_ARGUMENT_BUFFER(Argument, IntData , DataLen)	\
		Argument->Type = ACPI_METHOD_ARGUMENT_BUFFER;	\
		Argument->DataLength = DataLen;	\
        RtlCopyMemory(Argument->Data,IntData,DataLen)
#endif

#ifndef ACPI_METHOD_SET_ARGUMENT_INTEGER
#define ACPI_METHOD_SET_ARGUMENT_INTEGER(Argument,Integer)	\
		Argument->Type = ACPI_METHOD_ARGUMENT_INTEGER;	\
		Argument->DataLength = sizeof(ULONG);			\
        Argument->Argument = Integer
#endif

NTSTATUS TPM_CallDSMMethod(IN PDEVICE_OBJECT DeviceObject,
                           IN const GUID*     Interface,
                           IN ULONG     FunsIndex,
                           IN PVOID     Arguments,
                           IN USHORT    ArgumentsSize,
                           OUT PUCHAR*   Buffer,
                           OUT PULONG    BufLen)
{
    ULONG           i;
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    KEVENT          Event;
    PACPI_EVAL_INPUT_BUFFER_COMPLEX    InputBuffer;
    ULONG                               InputBufferLength;
    PACPI_EVAL_OUTPUT_BUFFER           OutputBuffer;
    ULONG                              OutputBufferLen;
    PACPI_METHOD_ARGUMENT   Argument;
    PIRP                    Irp;
    ULONG                   NumberOfBytes = 0;
    PUCHAR                  BytesReturn;
    ULONG                   BytesOffset = 0;

	PAGED_CODE();

    IoStatusBlock.Status = STATUS_SUCCESS;
    IoStatusBlock.Information = 0;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TPM_CallDSMMethod- Argument3:\n"));

    TpmDumpBuffer((PUCHAR)Arguments,ArgumentsSize,DPFLTR_INFO_LEVEL);

    if(ArgumentsSize <= sizeof(ULONG))
    {
        InputBufferLength = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + sizeof(GUID) - sizeof(ULONG) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + \
                            sizeof( ACPI_METHOD_ARGUMENT );
    }
    else
    {
        InputBufferLength = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + sizeof(GUID) - sizeof(ULONG) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) + \
                            sizeof( ACPI_METHOD_ARGUMENT ) - sizeof(ULONG) + ArgumentsSize;
    }

    InputBuffer = (PACPI_EVAL_INPUT_BUFFER_COMPLEX)ExAllocatePoolWithTag(PagedPool,
                                                                 InputBufferLength,
                                                                 POOL_TAG);
    if(InputBuffer)
    {
        RtlZeroMemory(InputBuffer,InputBufferLength);

        InputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        InputBuffer->MethodNameAsUlong = ACPI_DSM_NAME;

        InputBuffer->Size = InputBufferLength;
        InputBuffer->ArgumentCount = 4;

        // arg1
		Argument = &(InputBuffer->Argument[0]);
        ACPI_METHOD_SET_ARGUMENT_BUFFER(Argument,Interface,sizeof(GUID));
        // arg2
        Argument = ACPI_METHOD_NEXT_ARGUMENT(Argument); //(PACPI_METHOD_ARGUMENT)(InputBuffer->Argument.DUMMYUNIONNAME.Data + sizeof(GUID));
        ACPI_METHOD_SET_ARGUMENT_INTEGER(Argument,ACPI_METHOD_DSM_REVISION);
        // arg3
        Argument = ACPI_METHOD_NEXT_ARGUMENT(Argument); //(PACPI_METHOD_ARGUMENT)(AcpiArg->Argument.DUMMYUNIONNAME.Data + sizeof(ULONG));
        ACPI_METHOD_SET_ARGUMENT_INTEGER(Argument,FunsIndex);
        // arg4
        Argument = ACPI_METHOD_NEXT_ARGUMENT(Argument); // (PACPI_METHOD_ARGUMENT)(AcpiArg->Argument.DUMMYUNIONNAME.Data + sizeof(ULONG));
		Argument->Type = ACPI_METHOD_ARGUMENT_PACKAGE;
	    Argument->DataLength = ArgumentsSize;

		RtlCopyMemory(Argument->Data,Arguments,ArgumentsSize);


        OutputBufferLen = 1024;
        OutputBuffer = (PACPI_EVAL_OUTPUT_BUFFER)ExAllocatePoolWithTag(PagedPool,OutputBufferLen,POOL_TAG);
        if(OutputBuffer)
        {
            RtlZeroMemory(OutputBuffer,OutputBufferLen);
            KeInitializeEvent(&Event,SynchronizationEvent,FALSE);
            Irp = IoBuildDeviceIoControlRequest(IOCTL_ACPI_EVAL_METHOD,
                                                   DeviceObject,
                                                   InputBuffer,
                                                   InputBufferLength,
                                                   OutputBuffer,
                                                   OutputBufferLen,
                                                   FALSE,
                                                   &Event,
                                                   &IoStatusBlock);
            if(Irp)
            {
                Irp->IoStatus.Status      = STATUS_NOT_SUPPORTED;
                Irp->IoStatus.Information = 0;
                Status = IoCallDriver(DeviceObject,Irp);
                if(Status == STATUS_PENDING)
                {
                    KeWaitForSingleObject(&Event,Executive,KernelMode,FALSE,NULL);
                    Status = IoStatusBlock.Status;
                }

                if(NT_SUCCESS(Status) && IoStatusBlock.Information)
                {
                    if(OutputBuffer->Count)
                    {
						Argument = &(OutputBuffer->Argument[0]);

                        for(i = 0; i < OutputBuffer->Count; i++)
                        {
                            if(Argument->Type == ACPI_METHOD_ARGUMENT_INTEGER)
                            {
                                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                                        "Integer for method: %u = 0x%x\n",
                                        i,
                                        Argument->Argument));
                                NumberOfBytes += sizeof(ULONG);
                            }
                            else
                            {
                                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                                        "Type 0x%x for method: %u.  Bytes = %u\n",
                                        Argument->Type,
                                        i,
                                        *((PUSHORT)Argument->Data)));
                                NumberOfBytes += Argument->DataLength;
                            }

							Argument  = ACPI_METHOD_NEXT_ARGUMENT(Argument);
                        }
                    }

                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                        "ACPI returned %u bytes\n",
                        NumberOfBytes));

                    if(NumberOfBytes)
                    {
                        BytesReturn = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool,NumberOfBytes,POOL_TAG);
                        if(BytesReturn)
                        {
                            RtlZeroMemory(BytesReturn,NumberOfBytes);

                            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                                "Final OutputBuffer allocated at 0x%p\n",
                                BytesReturn));

                            // 
							Argument = &(OutputBuffer->Argument[0]);

                            for(i = 0; i < OutputBuffer->Count; i++)
                            {
                                if(OutputBuffer->Argument[i].Type == ACPI_METHOD_ARGUMENT_INTEGER)
                                {
                                    *((PULONG)(BytesReturn+BytesOffset)) =
                                        Argument->Argument;
                                    BytesOffset += sizeof(ULONG);
                                }
                                else
                                {
                                    RtlCopyMemory(BytesReturn+BytesOffset,
                                        Argument->Data,
                                        Argument->DataLength);

                                    BytesOffset += Argument->DataLength;
                                }

								Argument  = ACPI_METHOD_NEXT_ARGUMENT(Argument);
                            }

                            *Buffer = BytesReturn;
                            *BufLen = NumberOfBytes;

                            TpmDumpBuffer(BytesReturn,NumberOfBytes,DPFLTR_INFO_LEVEL);
                        }
                        else
                        {
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                }
            }
            else
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            ExFreePoolWithTag(OutputBuffer,POOL_TAG);
        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        ExFreePoolWithTag(InputBuffer,POOL_TAG);
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


NTSTATUS TPM_CallDSMMethodPackageInt1(IN PDEVICE_OBJECT DeviceObject,
                                      IN const GUID*     Interface,
                                      IN ULONG     FunsIndex,
                                      IN ULONG     Argument,
                                      OUT PUCHAR*  Buffer,
                                      OUT PULONG   BufLen)
{

	NTSTATUS Status;

	PAGED_CODE();

	Status =
        TPM_CallDSMMethod(DeviceObject,
                          Interface,
                          FunsIndex,
                          (PVOID)&Argument,
                          sizeof(ULONG),
                          Buffer,
                          BufLen);

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "Call_DSMMethodPackageInt1 - Call_DSMMethod returned 0x%x and %u bytes\n",
            Status,
            *BufLen));
    return Status;
}

NTSTATUS TPM_PPICallDSMMethodPackageEmpty(IN  PDEVICE_OBJECT DeviceObject,
                                          IN  ULONG		FunsIndex,
                                          OUT PUCHAR*	Buffer,
                                          OUT PULONG	BufLen)
{
	NTSTATUS Status;

	PAGED_CODE();

	Status =
            TPM_CallDSMMethodPackageInt1(DeviceObject,
					&PHYSICAL_PRESENCE_INTERFACE_GUID, 
					FunsIndex,
					0, 
					Buffer, 
					BufLen);
    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "Call_DSMMethodPackageEmpty - Call_DSMMethod returned (0x%x)\n",
            Status));
    return Status;
}


NTSTATUS TpmHandleSubmitAcpiCommand(WDFDEVICE Device,WDFREQUEST Request,size_t OutputBufferLength,size_t InputBufferLength)
{
    NTSTATUS        Status;
    PDEVICE_OBJECT  DeviceObject;
    WDFMEMORY       InputMemory = NULL;
    WDFMEMORY       OutputMemory= NULL;
    PUCHAR          InputBuffer = NULL;
    PUCHAR          OutputBuffer= NULL;
    ULONG           InputSize = 0;
    ULONG           OutputSize = 0;
    ULONG           FunsIndex = 0;
    ULONG           MinorFunsIndex = 0;
    char*           FunsString = NULL;
    PUCHAR          DSMMethodBuffer = NULL;
    ULONG           DSMMethodLen = 0;

	PAGED_CODE();

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                "Tpm_HandleSubmitAcpiCommand - Using Attached\n"));

    DeviceObject = WdfDeviceWdmGetAttachedDevice(Device);
    if(DeviceObject)
    {
        do
        {
            Status = WdfRequestRetrieveInputMemory(Request,&InputMemory);
            if(!NT_SUCCESS(Status)) break;

            InputBuffer = WdfMemoryGetBuffer(InputMemory,&InputSize);
            if(!InputBuffer)
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if(InputSize < sizeof(ULONG))
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            Status = WdfRequestRetrieveOutputMemory(Request,&OutputMemory);

            if(!NT_SUCCESS(Status)) break;

            OutputBuffer = WdfMemoryGetBuffer(OutputMemory,&OutputSize);

            FunsIndex = *((PULONG)InputBuffer);

            FunsString = (char*)TpmDbgGetPhysPresFuncString(FunsIndex);

            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                      "Tpm_HandleSubmitAcpiCommand - %p Function Index: 0x%x (%s)\n",
                      DeviceObject,
                      FunsIndex,
                      FunsString));

            KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                    "Submit to ACPI:\n"));

            if(FunsIndex == 2)
            {
                if(InputSize != 8)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "SET_PHYS_PRES_RQST - Invalid input buffer size: 0x%x\n",
                            InputSize));

                    Status = STATUS_INVALID_PARAMETER;

                    return Status;
                }

                if(OutputSize < 4)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "SET_PHYS_PRES_RQST - Invalid output buffer size: 0x%x\n",
                            InputSize));

                    Status = STATUS_INVALID_PARAMETER;

                    return Status;
                }

                MinorFunsIndex = *(PULONG)(InputBuffer + sizeof(ULONG));

                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                     "Calling TPM_CallDSMMethodPackageInt1 with func index %u, RequestIndex %u\n",
                            2,
                            MinorFunsIndex));

                if((MinorFunsIndex - 15) <= 112)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "Invalid RequestIndex %u\n",
                            MinorFunsIndex));

					return STATUS_INVALID_PARAMETER;
				}
				
				Status = TPM_CallDSMMethodPackageInt1(DeviceObject,
					&PHYSICAL_PRESENCE_INTERFACE_GUID,
					FunsIndex,
					MinorFunsIndex,
					&DSMMethodBuffer,
					&DSMMethodLen);
				
				if(NT_SUCCESS(Status))
				{
					if(DSMMethodLen > OutputSize)
					{
						KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
							"SET_PHYS_PRES_RQST - Too many bytes returned: %u (buffer size=%u)\n",
							DSMMethodLen,
							OutputSize));
						
						DSMMethodLen = OutputSize;
						
						Status = STATUS_BUFFER_OVERFLOW;
						
					}
					
					memcpy(OutputBuffer,DSMMethodBuffer,DSMMethodLen);
					
					KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
						"ACPI returned:\n"));
					
					TpmDumpBuffer(OutputBuffer,DSMMethodLen,DPFLTR_INFO_LEVEL);
					
					WdfRequestSetInformation(Request,DSMMethodLen);
					
				}
				
				if(DSMMethodBuffer) ExFreePoolWithTag(DSMMethodBuffer,0);
				
				return Status;
				
            }
            else if(FunsIndex == 1					 ||
				   (FunsIndex > 2 && FunsIndex <= 5) )
            {
                if(!OutputBuffer)
                {
                    Status = STATUS_INVALID_PARAMETER;
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_XXXX - NULL output buffer\n"));
                    return Status;
                }

                if(InputSize != sizeof(ULONG))
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_XXXX - Invalid input buffer size: 0x%x\n",
                            InputSize));
                    Status = STATUS_INVALID_PARAMETER;
                    return Status;
                }

                if(FunsIndex == 3 && OutputSize < 8)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_RQST - Invalid output buffer size: 0x%x\n",
                            OutputSize));
                    Status = STATUS_INVALID_PARAMETER;
                    return Status;
                }

                if(FunsIndex == 4 && OutputSize < sizeof(ULONG))
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_TRANS - Invalid output buffer size: 0x%x\n",
                            OutputSize));
                    Status = STATUS_INVALID_PARAMETER;
                    return Status;
                }

                if(FunsIndex == 5 && OutputSize < 12)
                {
                    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_RESP - Invalid output buffer size: 0x%x\n",
                            OutputSize));

                    Status = STATUS_INVALID_PARAMETER;

                    return Status;
                }

                KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                        "Calling TPM_PPICallDSMMethodPackageEmpty with func index %u\n",FunsIndex));

                Status = TPM_PPICallDSMMethodPackageEmpty(DeviceObject,
                                                          FunsIndex,
                                                          &DSMMethodBuffer,
                                                          &DSMMethodLen);
                if(NT_SUCCESS(Status))
                {
					if(DSMMethodLen > OutputSize)
					{
						KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
                            "GET_PHYS_PRES_XXXX - Too many bytes returned: %u (buffer size=%u)\n",
                            DSMMethodLen,
                            OutputSize));

						Status = STATUS_BUFFER_OVERFLOW;
						
						DSMMethodLen = OutputSize;
					}

					memcpy(OutputBuffer,DSMMethodBuffer,DSMMethodLen);
					
					KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
						"ACPI returned:\n"));
					
					TpmDumpBuffer(OutputBuffer,DSMMethodLen,DPFLTR_INFO_LEVEL);
					
					WdfRequestSetInformation(Request,DSMMethodLen);

                }

				if(DSMMethodBuffer) ExFreePoolWithTag(DSMMethodBuffer,0);
				
                return Status;

            }
            else
            {
                Status = STATUS_INVALID_PARAMETER;
            }
        } while(FALSE);
    }
    else
    {
        Status = STATUS_INVALID_PARAMETER;
    }
    return Status;
}
