
#include "precomp.h"
#include "defs.h"

#ifdef ALLOC_PRAGMA

#endif

#define ATMEL_DEFAULT_PORT		 0x4E
#define ATMEL_DATA_PORT          0x400   /* PIO data port */
#define ATMEL_STATUS_PORT        0x401   /* PIO status port */

#define ATMEL_STATUS_ABORT       0x01    /* ABORT command (W) */
#define ATMEL_STATUS_BUSY        0x01    /* device BUSY (R) */
#define ATMEL_STATUS_DATA_AVAIL  0x02    /* Data available (R) */

int atmel_rdx(UCHAR index)
{
//	outb(index, 0x4E);
//	return inb(0x4F) & 0xFF;
	WRITE_PORT_UCHAR((PUCHAR)ATMEL_DEFAULT_PORT,index);
	return READ_PORT_UCHAR((PUCHAR)ATMEL_DEFAULT_PORT + 1);
}

int atmel_v11_init()
{
	/* verify that it is an Atmel part */
	if (atmel_rdx(4) != 'A' || atmel_rdx(5) != 'T' ||
		atmel_rdx(6) != 'M' || atmel_rdx(7) != 'L')
		return 0;
	return 1;
}

VOID outb(UCHAR Value,PUCHAR Port)
{
	WRITE_PORT_UCHAR(Port,Value);
}

UCHAR inb(PUCHAR Port)
{
	return READ_PORT_UCHAR(Port);
}

NTSTATUS wait_for_status(UCHAR StsValue,UCHAR Off)
{



	return STATUS_SUCCESS;
}

NTSTATUS wait_for_not_status(UCHAR StsValue,UCHAR Off)
{

	return STATUS_SUCCESS;
}

int recv(unsigned char *buf, int count)
{
	PTPM_RESULT_BUFFER hdr = (PTPM_RESULT_BUFFER)buf;
	unsigned long size;
	int i;
	/* wait while the TPM to respond */
	if(!wait_for_status(ATMEL_STATUS_DATA_AVAIL|
		ATMEL_STATUS_BUSY,
		ATMEL_STATUS_DATA_AVAIL))
		return 0;
	/* start reading header */
	for (i = 0; i < 6; i++) {
		if ((inb(ATMEL_STATUS_PORT) &
			ATMEL_STATUS_DATA_AVAIL)
			== 0)
			return 0;
		*buf++ = inb(ATMEL_DATA_PORT);
	}
	/* size of the data received */
	size = BIG_ENDIAN_UINT(hdr->paramSize);
	if (count < size)
		return 0;
	/* read all the data available */
	for ( ; i < size; i++) {
		if ((inb(ATMEL_STATUS_PORT) &
			ATMEL_STATUS_DATA_AVAIL)
			== 0)
			return 0;
		*buf++ = inb(ATMEL_DATA_PORT);
	}
	
	/* sanity check: make sure data available is gone */
	if (inb(ATMEL_STATUS_PORT) & ATMEL_STATUS_DATA_AVAIL)
		return 0;
	
	return size;
}

int send(PTPM_CONTEXT TpmContext,PUCHAR buf, int count)
{
	int i;
	
	/* send abort and check for busy bit to go away */
	outb(ATMEL_STATUS_ABORT, ATMEL_STATUS_PORT);
	if (!wait_for_status(ATMEL_STATUS_BUSY, 0))
		return 0;
	
	/* write n bytes */
	for (i = 0; i < count; i++)
		outb(buf[i], ATMEL_DATA_PORT);
	
	/* wait for TPM to go BUSY or have data available */
	if (!wait_for_not_status(ATMEL_STATUS_BUSY|
		ATMEL_STATUS_DATA_AVAIL, 0))
		return 0;
	
	return count;
}

void write8(PTPM_CONTEXT TpmContext,UCHAR Value,UCHAR Off)
{
	WRITE_PORT_UCHAR(TpmContext->PortAddr + Off,Value);
}

UINT32 read32(PTPM_CONTEXT TpmContext,UINT32 Off)
{

	return 0;
}

#define ACCESS(l)                       (0x0000 | ((l) << 12))
#define STS(l)                          (0x0018 | ((l) << 12))
#define DATA_FIFO(l)                    (0x0024 | ((l) << 12))
#define DID_VID(l)                      (0x0F00 | ((l) << 12))
/* access bits */
#define ACCESS_ACTIVE_LOCALITY          0x20 /* (R)*/
#define ACCESS_RELINQUISH_LOCALITY      0x20 /* (W) */
#define ACCESS_REQUEST_USE              0x02 /* (W) */
/* status bits */
#define STS_VALID                       0x80 /* (R) */
#define STS_COMMAND_READY               0x40 /* (R) */
#define STS_DATA_AVAIL                  0x10 /* (R) */
#define STS_DATA_EXPECT                 0x08 /* (R) */
#define STS_GO                          0x20 /* (W) */

UCHAR read8(PTPM_CONTEXT TpmContext,UINT32 PortOff)
{
	return TpmContext->MemAddr[PortOff];
}

int request_locality(PTPM_CONTEXT TpmContext,UCHAR l)
{
	UCHAR locality;
	write8(TpmContext,ACCESS_RELINQUISH_LOCALITY, ACCESS(l));
	
	write8(TpmContext,ACCESS_REQUEST_USE, ACCESS(l));
	/* wait for locality to be granted */
	locality = read8(TpmContext,ACCESS(l) & ACCESS_ACTIVE_LOCALITY);
	if (locality)
		return locality = l;
	
	return -1;
}

int atmel_v12_init(PTPM_CONTEXT TpmContext)
{
	UINT32 vendor;
	int i;
	
	for (i = 0 ; i < 5 ; i++)
		write8(TpmContext,ACCESS_RELINQUISH_LOCALITY, ACCESS(i));
	if (request_locality(TpmContext,0) < 0)
		return 0;
	
	vendor = read32(TpmContext,DID_VID(0));
	if ((vendor & 0xFFFF) == 0xFFFF)
		return 0;
	
	return 1;
}
