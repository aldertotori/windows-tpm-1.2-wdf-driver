
#ifdef _WIN32
#include <windows.h>
#else
#include <ntddk.h>
#endif
#include "CmdBuf.h"

#pragma pack(push,1)

typedef struct _TPM_COMMAND_BUFFER
{
    USHORT      tag;
    ULONG       paramSize;
    ULONG       ordinal;
} TPM_COMMAND_BUFFER,*PTPM_COMMAND_BUFFER;

typedef struct _TPM_RESULT_BUFFER
{
    USHORT      tag;
    ULONG       paramSize;
    ULONG       returnCode;
} TPM_RESULT_BUFFER,*PTPM_RESULT_BUFFER;

#pragma pack(pop)

VOID  CmdBufInit(PUCHAR Buf,UINT32 Ordinal)
{
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	Cmd->tag = BIG_ENDIAN_WORD(TPM_TAG_RQU_COMMAND);
	Cmd->paramSize = BIG_ENDIAN_UINT(sizeof(TPM_COMMAND_BUFFER));
	Cmd->ordinal = BIG_ENDIAN_UINT(Ordinal);
}

VOID CmdBufAddUINT32(PUCHAR Buf,UINT32 data)
{
	UINT32 Offset;
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	Offset = BIG_ENDIAN_UINT(Cmd->paramSize);
	*(PUINT32)(Buf + Offset) = BIG_ENDIAN_UINT(data);
	Cmd->paramSize = BIG_ENDIAN_UINT(Offset + sizeof(UINT32));
}

VOID CmdBufAddUSHORT(PUCHAR Buf,USHORT data)
{
	UINT32 Offset;
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	Offset = BIG_ENDIAN_UINT(Cmd->paramSize);
	*(PUSHORT)(Buf + Offset) = BIG_ENDIAN_WORD(data);
	Cmd->paramSize = BIG_ENDIAN_UINT(Offset + sizeof(USHORT));
}

VOID CmdBufAddUCHAR(PUCHAR Buf,UCHAR data)
{
	UINT32 Offset;
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	Offset = BIG_ENDIAN_UINT(Cmd->paramSize);
	*(Buf + Offset) = data;
	Cmd->paramSize = BIG_ENDIAN_UINT(Offset + sizeof(UCHAR));
}

VOID CmdBufAddBuf(PUCHAR Buf,PUCHAR Data,UINT32 DataLen)
{
	UINT32 Offset;
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	Offset = BIG_ENDIAN_UINT(Cmd->paramSize);
	RtlCopyMemory(Buf+Offset,Data,DataLen);
	Cmd->paramSize = BIG_ENDIAN_UINT(Offset + DataLen);
}

UINT CmdBufGetLen(PUCHAR Buf)
{
	PTPM_COMMAND_BUFFER Cmd = (PTPM_COMMAND_BUFFER)Buf;
	return BIG_ENDIAN_UINT(Cmd->paramSize);;
}

VOID ResultInit(PUCHAR Buf,UINT32 RetLen)
{
	PTPM_RESULT_BUFFER Result = (PTPM_RESULT_BUFFER)Buf;
	Result->tag = BIG_ENDIAN_WORD(TPM_TAG_RSP_COMMAND);
	Result->paramSize = BIG_ENDIAN_UINT(RetLen);
}

TSS_RESULT GetResultStatus(PUCHAR Buf)
{
	PTPM_RESULT_BUFFER Ret = (PTPM_RESULT_BUFFER)Buf;
	return BIG_ENDIAN_UINT(Ret->returnCode);
}