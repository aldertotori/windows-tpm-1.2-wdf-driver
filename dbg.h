
#ifndef _DBG_H_
#define _DBG_H_

ULONG TpmDumpBuffer(PUCHAR buf,ULONG len,ULONG Level);
VOID TpmLogEvent(ULONG SymbolNameValue,PDEVICE_OBJECT Device,NTSTATUS Info,ULONG Site);
const char* TpmDbgGetTpmStateString(TPM_STATE State);
const char* TpmDbgGetTpmThreadIDString(TPM_THREAD_ID ThreadId);
const char* TpmDbgGetPhysPresFuncString(ULONG MinorFunsIndex);

#endif
