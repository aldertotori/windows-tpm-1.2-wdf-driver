
#include <ntddk.h>
#include "cng.h"



NTSTATUS EntropyRegisterSource(OUT ENTROPY_SOURCE_HANDLE * phEntropySource,
							   IN ENTROPY_SOURCE_TYPE entropySourceType,
							   IN PCWSTR entropySourceName)
{
	return STATUS_SUCCESS;
}

NTSTATUS EntropyUnregisterSource(ENTROPY_SOURCE_HANDLE hEntropySource)
{
	return STATUS_SUCCESS;
}

NTSTATUS EntropyProvideData(ENTROPY_SOURCE_HANDLE hEntropySource,
							PCBYTE pbData,
							SIZE_T cbData,
							ULONG entropyEstimateInMilliBits )
{
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING Regstry)
{
	return STATUS_SUCCESS;
}


