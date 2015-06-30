
#include "precomp.h"

ULONG TpmGetEntropyDensity();
NTSTATUS TpmGetEntropyInterval(ULONG* Interval);
NTSTATUS TpmQueryDWORD(WDFKEY  Key,PCWSTR KeyName,ULONG NameLen,PULONG retVal);
NTSTATUS TpmQueryFlag(WDFKEY  Key,PCWSTR KeyName,PULONG pulVal,PBOOLEAN pbVal);
VOID TpmGetRegistryFlags(WDFDEVICE device,PTPM_CONTEXT TpmContext);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, TpmGetEntropyDensity)
#pragma alloc_text (PAGE, TpmGetEntropyInterval)
#pragma alloc_text (PAGE, TpmQueryDWORD)
#pragma alloc_text (PAGE, TpmQueryFlag)
#pragma alloc_text (PAGE, TpmGetRegistryFlags)
#endif

typedef struct _DWORD_KEY_VALUE_PARTIAL_INFORMATION
{
    ULONG  TitleIndex;
    ULONG  Type;
    ULONG  DataLength;
    union
    {
        ULONG  lData[13];
        UCHAR  cData[53];
    } u;
} DWORD_KEY_VALUE_PARTIAL_INFORMATION,*PDWORD_KEY_VALUE_PARTIAL_INFORMATION;

ULONG TpmGetEntropyDensity()
{
    NTSTATUS            Status;
    UNICODE_STRING      NameString;
    UNICODE_STRING      ValueName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              Handle;
    DWORD_KEY_VALUE_PARTIAL_INFORMATION  DwordValueInfo;
    ULONG               Length;
    ULONG               Result = 8000;

	PAGED_CODE();

    RtlInitUnicodeString(&NameString,
                         L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Cryptography\\RNG");

    InitializeObjectAttributes(&ObjectAttributes,
                               &NameString,
                               OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, // 0x240
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_QUERY_VALUE, // 1
                       &ObjectAttributes);

    if(NT_SUCCESS(Status))
    {
        RtlInitUnicodeString(&ValueName,
                             L"TpmEntropyDensityInMilliBitsPerByte");
        Status = ZwQueryValueKey(Handle,
                        &ValueName,
                        KeyValuePartialInformation,
                        (PVOID)&DwordValueInfo,
                        sizeof(DWORD_KEY_VALUE_PARTIAL_INFORMATION),
                        &Length);

        if(NT_SUCCESS(Status))
        {
            if(DwordValueInfo.Type == REG_DWORD &&
               DwordValueInfo.DataLength == sizeof(ULONG))
            {
                Result = DwordValueInfo.u.lData[0];
                if(Result < 1 || Result > 8000)
                    Result = 8000;
            }
        }
        ZwClose(Handle);
    }
    return Result;
}

NTSTATUS TpmGetEntropyInterval(ULONG* Interval)
{
    NTSTATUS            Status;
    UNICODE_STRING      NameString;
    UNICODE_STRING      ValueName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              Handle;
    DWORD_KEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG               Length;
	ULONG				ResultLength;
	ULONG				Result;
    ULONG               Data;

	PAGED_CODE();

    RtlInitUnicodeString(&NameString,
                         L"\\Registry\\Machine\\SOFTWARE");

    InitializeObjectAttributes(&ObjectAttributes,
                               &NameString,
                               OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_QUERY_VALUE, // 1
                       &ObjectAttributes);
    if(!NT_SUCCESS(Status))
    {
        if(Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        return Status;
    }
    ZwClose(Handle);

    RtlInitUnicodeString(&NameString,
                         L"\\Registry\\Machine\\SOFTWARE\\Policies\\Microsoft\\Cryptography\\RNG");

    InitializeObjectAttributes(&ObjectAttributes,
                               &NameString,
                               OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    Status = ZwOpenKey(&Handle,
                       KEY_QUERY_VALUE, // 1
                       &ObjectAttributes);
    if(!NT_SUCCESS(Status))
    {
        if(Status != STATUS_OBJECT_NAME_NOT_FOUND)
            return Status;
        *Interval = 40;
        return STATUS_SUCCESS;
    }
    RtlInitUnicodeString(&ValueName, L"TpmRefreshEntropyIntervalInMinutes");
    Status = ZwQueryValueKey(Handle,
                    &ValueName,
                    KeyValuePartialInformation,
                    &KeyValueInformation,
                    sizeof(KeyValueInformation),
                    &ResultLength);
    ZwClose(Handle);

    if(!NT_SUCCESS(Status) ||
       KeyValueInformation.Type != REG_DWORD ||
       KeyValueInformation.DataLength != sizeof(ULONG))
    {
        *Interval = 40;
        return STATUS_SUCCESS;
    }
    Result = KeyValueInformation.u.lData[0];
    if(Result > 40)
        Result = 40;
    *Interval = Result;
    return Status;
}

NTSTATUS TpmQueryDWORD(WDFKEY  Key,PCWSTR KeyName,ULONG NameLen,PULONG retVal)
{
    UNICODE_STRING  NameString;
    ULONG           Value;
    NTSTATUS        Status;

	PAGED_CODE();

    if(KeyName)
    {
        RtlInitUnicodeString(&NameString,KeyName);
        Status = WdfRegistryQueryULong(Key,
                                       &NameString,
                                       &Value);
        if(NT_SUCCESS(Status))
        {
            *retVal = Value;
        }
        return Status;
    }
    else
    {
        return STATUS_INVALID_PARAMETER;
    }
}

NTSTATUS TpmQueryFlag(WDFKEY  Key,PCWSTR KeyName,PULONG pulVal,PBOOLEAN pbVal)
{
    NTSTATUS Status;

	PAGED_CODE();

	Status = TpmQueryDWORD(Key,KeyName,wcslen(KeyName),pulVal);

    if(NT_SUCCESS(Status))
    {
        if(*pulVal)
            *pbVal = TRUE;
        else
            *pbVal = FALSE;
    }
    return Status;
}

VOID TpmGetRegistryFlags(WDFDEVICE device,PTPM_CONTEXT TpmContext)
{
    WDFKEY  Key = 0;
    NTSTATUS Status;
    ULONG    ulVal;

	PAGED_CODE();

    Status = WdfDeviceOpenRegistryKey(device,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      GENERIC_READ,
                                      WDF_NO_OBJECT_ATTRIBUTES,
                                      &Key);
    if(NT_SUCCESS(Status))
    {
        ulVal = sizeof("SkipInitCommands");
        TpmQueryFlag(Key,L"SkipInitCommands",
                     &ulVal,
                     &TpmContext->SkipInitCommands);

        ulVal = sizeof("SkipUnderflowCheck");
        TpmQueryFlag(Key,L"SkipUnderflowCheck",
                     &ulVal,
                     &TpmContext->SkipUnderflowCheck);

        ulVal = sizeof("SkipOverflowCheck");
        TpmQueryFlag(Key,L"SkipOverflowCheck",
                     &ulVal,
                     &TpmContext->SkipOverflowCheck);

        ulVal = sizeof("SkipAccessRegisterCheck");
        TpmQueryFlag(Key,L"SkipAccessRegisterCheck",
                     &ulVal,
                     &TpmContext->SkipAccessRegisterCheck);

        ulVal = sizeof("UsePortBasedIO");
        TpmQueryDWORD(Key,L"UsePortBasedIO",ulVal,
                      &TpmContext->UsePortBasedIO);

        ulVal = sizeof("DelayExpectBitCheck");
        TpmQueryFlag(Key,L"DelayExpectBitCheck",
                     &ulVal,
                     &TpmContext->DelayExpectBitCheck);

        ulVal = sizeof("TPMVersion");
        TpmQueryDWORD(Key,L"TPMVersion",ulVal,
                      &TpmContext->TPMVersion);

		if(TpmContext->TPMVersion == 0x11)
			TpmContext->UsePortBasedIO = 1;
		else if(TpmContext->TPMVersion >= 0x12)
			TpmContext->UsePortBasedIO = 0;

        WdfRegistryClose(Key);
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
            "TpmGetRegistryFlags: WdfDeviceOpenRegistryKey failed with Status 0x%x \n",
            Status));
    }
}
