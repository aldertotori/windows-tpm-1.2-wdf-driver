
#ifndef _CNG_H
#define _CNG_H

#pragma once

#define CNG_ENTROPY_NAME     L"Microsoft Windows TPM driver"

typedef HANDLE ENTROPY_SOURCE_HANDLE;
typedef enum
{
    ENTROPY_SOURCE_TYPE_UNKNOWN = 0,
    ENTROPY_SOURCE_TYPE_TPM,
} ENTROPY_SOURCE_TYPE;
typedef CCHAR PCBYTE;

NTSTATUS EntropyRegisterSource(OUT ENTROPY_SOURCE_HANDLE * phEntropySource,
							   IN ENTROPY_SOURCE_TYPE entropySourceType,
							   IN PCWSTR entropySourceName);

NTSTATUS EntropyUnregisterSource(ENTROPY_SOURCE_HANDLE hEntropySource);

NTSTATUS EntropyProvideData(ENTROPY_SOURCE_HANDLE hEntropySource,
							PCBYTE pbData,
							SIZE_T cbData,
							ULONG entropyEstimateInMilliBits );

#endif //
