
#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, TpmDbgGetPhysPresFuncString)
#pragma alloc_text (PAGE, TpmDbgGetTpmStateString)
#pragma alloc_text (PAGE, TpmDbgGetTpmThreadIDString)
#endif

VOID TpmLogEvent(ULONG SymbolNameValue,PDEVICE_OBJECT Device,NTSTATUS Info,ULONG Site)
{
    PIO_ERROR_LOG_PACKET  LogPacket;

    KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_INFO_LEVEL,
            "TpmLogEvent: SymbolNameValue = 0x%x, Info = 0x%x, Site = 0x%d\n",
            SymbolNameValue,
            Info,
            Site));
    if(Device)
    {
        LogPacket = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(Device,
                                                                  sizeof(IO_ERROR_LOG_PACKET)+sizeof(UCHAR));
        if(LogPacket)
        {
            RtlZeroMemory(LogPacket,sizeof(IO_ERROR_LOG_PACKET));
            LogPacket->DumpDataSize = (USHORT)sizeof(ULONG);
            LogPacket->ErrorCode    = SymbolNameValue;
            LogPacket->UniqueErrorValue = Site;
            LogPacket->DumpData[0] = Info;
            IoWriteErrorLogEntry(LogPacket);
        }
    }
    else
    {
        KdPrintEx((DPFLTR_PNPMEM_ID, DPFLTR_WARNING_LEVEL,
                "Object is NULL - unable to log event!\n"));
    }
}


const char* TpmDbgGetPhysPresFuncString(ULONG index)
{
	
	PAGED_CODE();

    switch(index)
    {
    case 1:
        return "GET_PHYS_PRES_IFC_VER";
    case 2:
        return "SET_PHYS_PRES_RQST";
    case 3:
        return "GET_PHYS_PRES_RQST";
    case 4:
        return "GET_PHYS_PRES_TRANS";
    case 5:
        return "GET_PHYS_PRES_RESP";
    default:
        return "UNKNOWN";
    }
}

const char* TpmDbgGetTpmStateString(TPM_STATE State)
{
	
	PAGED_CODE();

    switch(State)
    {
	case StAvailable:
		return "StAvailable";

    case StBusy:
        return "StBusy";

    case StAborting:
        return "StAborting";

    case StSuspendPending:
        return "StSuspendPending";

    case StSuspend:
        return "StSuspend";

    default:
        return "UNKNOWN";
    }
}

const char* TpmDbgGetTpmThreadIDString(TPM_THREAD_ID ThreadId)
{

	PAGED_CODE();

	switch ( ThreadId )
	{
	case 0:
		return "IdCommand";
		
	case 1:
		return "IdAbort";
		
	case 2:
		return "IdPowerdown";
		
	case 3:
		return "IdPowerup";
		
	case 4:
		return "IdEntropy";
		
	default:
		return "UNKNOWN";

	}
    
}

ULONG TpmDumpBuffer(PUCHAR buf,ULONG len,ULONG Level)
{
    ULONG  index;
    ULONG  size = len;

	for(index = 0; index < len; index++)
	{
		KdPrintEx((DPFLTR_PNPMEM_ID, Level,"0x%02x ",buf[index]));
	}
	KdPrintEx((DPFLTR_PNPMEM_ID, Level,"\n"));

	return 0;

}

