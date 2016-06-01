
/* drvAbDf1.c */
/* 	$Id: drvAbDf1.c,v 1.2 2002/08/23 13:21:07 pedro Exp $ */
/*
 *      EPICS Driver Support for the Allen Bradley DF1 serial protocol
 *
 *      Author:         
 *			Jeffrey O. Hill (LANL)
 *			johill@lanl.gov
 *			(505) 665 1831
 *			(for the KECK Astronomy Center)
 *
 *      Date:           6-6-95
 *
 * Modification Log:
 * -----------------
 * 	Revision 1.9  1999/12/23 16:55:37  hill
 * 	fixed warnings drvAbdf1.c
 *
 * 	Revision 1.8  1998/09/17 22:47:05  aptdvl
 * 	added support for signed 16 bit words in PLC
 *
 * 	Revision 1.7  1998/07/21 22:12:25  hill
 * 	doc
 *
 * Revision 1.4  1995/09/01  07:45:13  jhill
 * header comments
 *
 * Revision 1.3  1995/08/28  02:38:16  jhill
 * use VX_FP_TASK
 *
 * Revision 1.2  1995/08/24  07:15:11  jhill
 * changed comments
 *
 * Revision 1.1  1995/08/24  07:12:37  jhill
 * installed into cvs
 *
 *
 *	
 * NOTES:
 *
 * 1) The AB node (station) number for the host that is running this code
 * usually must be specified. The default station number for this code
 * is octal 012, and is KECK site specific. To change the default node
 * issue the following command from the vxWorks shell.
 *
 * drvAbDf1SrcStationNumber = NNN 
 *
 * For example, to set the local node number to 4 issue the following
 * command prior to calling "iocInit()"
 *
 * drvAbDf1SrcStationNumber = 4
 *
 * The local node number can also be specified independently for each 
 * serial port by typing the following.
 *
 * drvAbDf1SetLocalNodeNo ("PORT", NODENO)
 *
 * For example, to set the local node number of port /dlpc/0 to octal 10
 * we would enter the following command at the vxWorks shell
 *
 * drvAbDf1SetLocalNodeNo ("/dlpc/0", 010)
 * 
 * For example, to set the local node number of port /dlpc/0 to hex 10
 * we would enter the following command at the vxWorks shell
 *
 * drvAbDf1SetLocalNodeNo ("/dlpc/0", 0x10)
 * 
 * For example, to set the local node number of port /dlpc/0 to decimal 10
 * we would enter the following command at the vxWorks shell
 *
 * drvAbDf1SetLocalNodeNo ("/dlpc/0", 10)
 *
 * Records with node addresses that match drvAbDf1SrcStationNumber
 * will create local PLC5 style variables (elements in numbered files) 
 * in the local IOC that can be read and written by other PLC stations 
 * via the DF1 protocol(frequently over data highway). In this situation 
 * the driver acts as a server which will process DF1 read/write requests 
 * from other PLC clients on the network (frequently data highway). If an 
 * input record specifies a local PLC5 style variable and it is I/O interrupt
 * scanned then the interrupt will occur whenever a new value is written
 * into the local variable. The driver will accept only PLC5 addresses in
 * when creating IOC local variables, and will ignore PLC2 addresses that 
 * specify a node number that matches drvAbDf1SrcStationNumber. However,
 * it _is_ possible to create a PLC5 style local variable which will
 * be accessed by PLC2 protocol in PLC2 compatibility mode
 * (the station number of the PLC2 specifies the PLC5 file that is to be 
 * addressed).
 *
 * 2) Typically all EPICS input records using this driver have their 
 * SCAN field set to "I/O Interrupt" so that the record is not processed 
 * unless the element in the PLC's file is actually changing. The variable
 * drvAbDf1DefaultScanPeriod_mS specifies the default for how often the
 * PLCs files are interrogated with DF1 block transfer requests
 * (how often the cache in this program is updated), and
 * therefore how often this driver will check to see if a particular signal 
 * has changed. The default value of drvAbDf1DefaultScanPeriod_mS is
 * 100 mS. The value of drvAbDf1DefaultScanPeriod_mS can be changed from the
 * vxWorks shell as follows. 
 *
 * drvAbDf1DefaultScanPeriod_mS = DELAY
 *
 * The value specified is the unsigned integer number of milli seconds 
 * between scans. For example the following will specify a default of
 * 10 mS between scans. Of course the value of this parameter must be
 * initialized before it is used in "iocInit()".
 *
 * drvAbDf1DefaultScanPeriod_mS = 10
 *
 * It is also possible to set the interrogation period for a particular file in 
 * a particular PLC on a particular serial port by typing the following.
 *
 * drvAbDf1SetScanPeriod ("PORT", PLCNODENO, FILENO, PERIOD_mS)
 *
 * For example, to set the scan period of port /dlpc/0, PLC node 8, file 7
 * to be 10 mS enter the following command at the vxWorks shell
 *
 * drvAbDf1SetScanPeriod ("/dlpc/0", 8, 7, 10)
 *
 * 3) Outputs are automatically scanned at the rate determined by (2)
 * above. Outputs can be changed by many AB sources that are outside of 
 * the control of EPICS. When this driver sees that an output has been
 * changed by another source besides this program, then the output record is
 * processed. IO to the PLC is suppressed in this situation, but all other
 * normal record processing will take place including processing of
 * forward links. Database designers need to take this special behavior into 
 * consideration when interconnecting their function blocks. Note that this output
 * change record processing is accomplished by artificially executing the end of 
 * asynchronous record processing, and therefore will be invisible to the 
 * "trace record processing" option in the EPICS database.
 *
 * 4) detailed diagnostics are available by typing "dbior, 0, <level>"
 * from the vxWorks shell. Increasing magnitude in the level argument
 * produces a more detailed report.
 *
 * 5) A detailed dump of the DF1 serial protocol as it passes in/out of
 * the IOC can be obtained by setting drvAbDf1Debug to a non-zero value
 * from the vxWorks shell. Increasing magnitude in drvAbDf1Debug will
 * produce increasing detail in the protocol dump. Turning on this
 * option will usually cause a dramatic change in the execution speed
 * and efficiency of the driver.
 *
 */

/*
 * ANSI C
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>

/*
 * The station number for the host that is running this code
 *
 * !! Default here is Keck Site Specific !!
 *
 * This is in the global symbol table so you can change it from
 * the vxWorks shell by typing:
 *
 * drvAbDf1SrcStationNumber = NNN 
 *
 * The node number can also be specified independently for each 
 * serial port (see notes at the top o this file)
 */
unsigned drvAbDf1SrcStationNumber = 012;

/*
 * The default read block IO scan period
 *
 * This is in the global symbol table so you can change it from
 * the vxWorks shell by typing:
 * drvAbDf1DefaultScanPeriod_mS = NNN 
 * 
 * scan rate can also be specified independently for each file
 * (see notes at the top o this file)
 */
unsigned drvAbDf1DefaultScanPeriod_mS = 100; /* milli sec */

/*
 * turn on/off debug messages
 *
 * increasing detail with increasing magnitude of drvAbDf1Debug
 */
unsigned drvAbDf1Debug = 0;

/*
 * The maximum outstanding requests (the maximum simultaneous
 * unanswered requests allowed)
 *
 * This is in the global symbol table so you can change it from
 * the vxWorks shell by typing:
 * drvAbDf1MaxOutstandingRequest = NNN 
 *
 * It should only occasionally be necessary to change this 
 * parameter
 */
unsigned drvAbDf1MaxOutstandingRequest = 16u;

/*
 * Set the local node number for a specified port
 */
unsigned long drvAbDf1SetLocalNodeNo (const char *pPort, unsigned localNodeNo);

/*
 * Set the scan period for the specified port, nodeNo, fileNo
 */
unsigned long drvAbDf1SetScanPeriod (const char *pPort, 
					unsigned nodeNo, unsigned fileNo, unsigned period_mS);

/*
 * vxWorks
 */
#include <semLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <tickLib.h>

/*
 * EPICS
 */
#include <epicsAssert.h>
#include <ellLib.h>		/* requires stdlib.h */
#include <devLib.h>		/* requires stdlib.h */
#include <bucketLib.h>
#include <epicsPrint.h>
#include <dbScan.h>
#include <freeList.h>

#include <drvSerial.h>

#define REFERENCE_abDataTypeCvrt
#include <drvAbDf1.h>
#include <df1.h>


#define STRINGOF(A) #A

#if defined(__GNUC__) && 0
#define INLINE inline
#else
#define INLINE
#endif

/*
 * work with old versions of bucketLib.c
 */
#ifndef S_bucket_success
#define S_bucket_success BUCKET_SUCCESS
#endif

#define TICKS_PER_SEC (sysClkRateGet())
#define WAIT_N_SEC(N) {taskDelay(TICKS_PER_SEC*(ULONG)(N));}

/*
 * task creation options
 */
#define tp_drvAbDf1Priority 60 	/* higher than network activity */
#define tp_drvAbDf1Options VX_FP_TASK 	/* none */
#define tp_drvAbDf1StackSize 0x1000	/* 4096 */

#define drvAbDf1MaxNegAck 6U
#define drvAbDf1MaxXmitTMO 6U


#define ENQ_TMO (TICKS_PER_SEC/4ul) /* 1/4 sec */
#define NAK_BASE_TMO (TICKS_PER_SEC/4ul) /* 1/4 sec */
#define RESPONSE_TMO (10UL*TICKS_PER_SEC) /* 10 sec */
#define MUTEX_TMO (TICKS_PER_SEC) /* 1 sec */

typedef struct drvAbDf1ElemIO drvAbDf1ElemIO;
typedef struct absBlockIO absBlockIO;

struct drvAbDf1ElemIO {
	drvAbDf1ElemIO	*pNext;				/* Link to next structure (pvt to drv support) */
	absBlockIO		*pBIO;				/* pointer to the associated IO block */
	uint16_t		elemNo;             /* Desired element number (init by drv support) */
	uint16_t		subElemNo;          /* Desired sub-element number (init by drv support) */
	abDf1ElemIO		dev;
};

/*
 * convert from a device support element IO structure to 
 * to the driver private structure
 */
#define devToDrvElemIOPtr(pDevElemIO) \
	( (drvAbDf1ElemIO *) ( ((char *)pDevElemIO) - offsetof (drvAbDf1ElemIO, dev)) )

typedef struct drvAbDf1Parm drvAbDf1Parm;
typedef drvAbDf1InputFunc (FILE *, drvSerialResponse *, drvAbDf1Parm *);

/*
 * FILE pointers not stored here so that we will not be
 * tempted to use the non thread safe vxWorks stdio facility from
 * more than one task
 */
struct drvAbDf1Parm {
	ELLNODE node;
	char pName[512];
	ELLLIST transLimboList; /* absTransactions are here after they are created */
	ELLLIST transDSList; /* absTransactions pending transmission here */
	ELLLIST transACKList; /* absTransactions pending transmission here */
	ELLLIST transResList; /* absTransactions pending response here */
	ELLLIST scanList; /* absBlockIO awaiting scan here */
	ELLLIST localList; /* absBlockIO local PVs here until hashed */
	ELLLIST pendingList; /* absBlockIO io with scan pending here */
	ELLLIST plcIOList; /* list of PLCs in use */
	SEM_ID mutexSem;
	SEM_ID ackSem;
	SEM_ID scanSem;
	drvSerialLinkId id;
	BUCKET *pTransBucket;
	ELLLIST *pInitOutIO;
	ELLLIST *pLocalHashList;
	void *pTransFreeListPVT;
	devAbDf1ParseAddressFunc *pParseAddress;
	drvAbDf1InputFunc *pReadCharFunc;
	drvAbDf1InputFunc *inHandlerList[df1dlLAST+1u];
	ULONG smoothedDelayToResp;
	ULONG maxDelayToResp;
	ULONG smoothedDelayToReadSend;
	ULONG maxDelayToReadSend;
	ULONG smoothedDelayToWriteSend;
	ULONG maxDelayToWriteSend;
	int taskId;
	unsigned nakSendCount;
	unsigned damagedFrameCount;
	unsigned nakRecvCount;
	unsigned badControlRecvCount;
	unsigned enqTimeoutCount;
	unsigned nakTimeoutCount;
	unsigned responseTimeoutCount;
	unsigned dupResponseCount;
	unsigned totFramesSent;
	unsigned totFramesRecv;
	unsigned localHashSize;
	uint16_t nextTransId; /* must be an unsugned 16 bit integer */
	uint8_t lastAckSent;
	uint8_t nextAckSent;
	uint8_t nodeNo; /* node number */
	unsigned ackRecv:1;
	unsigned negAckRecv:1;
	unsigned scanInit:1;
};


/*
 * Notes on maxDataBytes field:
 * The maximum data bytes is selected to
 * be evenly divisable by 6 for typed reads
 * and typed writes so that we will be 
 * on a 6 byte natural boundary. This prevents
 * the 3 word AB compound values such as timers
 * from crossing cache boundaries. These values 
 * for maxDataBytes are of course also constrained 
 * by the DF1 250 bytes/frame limit and the protocol
 * element involved.
 */
#define absLocalElemCount 0x100

typedef enum {absBlockWrite, absBlockRead, absTypedWrite, 
				absTypedRead, absLocal, absIOCount} absIOType;

typedef enum {	
		absQueNone,
		absQueTransLimbo, /* recently allocated transaction */
		absQueTransRes, /* PLC response is pending */
		absQueTransACK, /* request waiting for acknowledged by PLC */
		absQueTransDS, /* request waiting to be accepted by drive serial */
		absQueScan,
		absQuePending,
		absQueLocal
} absQueType;

typedef struct {
	ELLNODE node;
	ELLLIST fileList;
	uint8_t nodeNo; /* station number */
	drvAbDf1Parm *pDev;
	unsigned ioOutstandingCount;
} absPLCIO;

enum dtClass {dtcUnknown, dtcUntyped, dtcTyped, dtcLoopBack};

/*
 * indexed by enum dtClass
 */
LOCAL const char *dtClassLabel[] = {
					"Unknown",
					"Untyped",
					"Typed",
					"Loop Back"
};

typedef struct {
	ELLNODE node;
	absPLCIO *pPLC;
	ULONG scanPeriod; /* ticks between scans */
	uint16_t fileNo; /* only for PLC5 IO */
	uint8_t dataType; /* AB data type (one of df1PrimTypes) */
	uint8_t dataTypeClass; /* one from enum type dtClass */
	uint8_t effectiveDataType; /* AB effective data type (one of df1PrimTypes) jws 1/31/2000 */
} absFileIO;

struct absBlockIO {
	ELLNODE node;
	long lastReadStatus; /* the last IO exception */
	long lastWriteStatus; /* the last IO exception */
	ULONG ticksAtScanCompletion; /* ticks at scan completion */
	absFileIO *pFile; /* pointer to the per file info */
	void *pData; /* pointer to the cache */
	drvAbDf1ElemIO **pElemIOTbl;
	unsigned localId; /* hash id (for local variables) */
	unsigned ioCount; /* number of successful io operations */
	uint16_t elemNo; /* element number of first item in cache */
	uint8_t elemCount; /* number of elements in cache*/
	uint8_t queue; /* the queue on which this block is installed */
};

typedef struct {
	ELLNODE node;
	ULONG ticksAtReq;
	unsigned transId; /* bucket lib requires unsigned here */
	absBlockIO *pIO; /* for all IO */
	drvAbDf1ElemIO *pElemIO; /* only for writes (NULL for reads) */
	ELLLIST *pTransList; /* the list that the transaction is stored on */
	uint8_t read; /* true if its a read operation */
	uint8_t protoType; /* one of df1ProtoType goes here */
	uint8_t respPending; /* response to this transaction pending */
}absTransaction;

typedef enum  {
		dptUnknown,
		dptBlockRead,
		dptBlockWrite, 
		dptTypedBlockRead,
		dptTypedBlockWrite,
		dptBitWrite,
		dptReadModifyWrite,
		dptLoopBack,
		dptFileBlockWrite,
		dptFileBlockRead
} df1ProtoType;

/*
 * indexed by enum dtClass
 */
LOCAL const unsigned dtClassReadProto[] = {
					dptUnknown,
					dptBlockRead,		/* PLC2 */
					dptTypedBlockRead,	/* PLC5 only */
					dptLoopBack
};
			
typedef void (*reqProcFunc) (absTransaction *pTrans, 
		abDf1ReqProto *pReq, unsigned *pByteCount);

LOCAL void doNothingReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void blockWriteReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void typedBlockWriteReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void fileBlockWriteReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void bitWriteReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void readModifyWriteReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void blockReadReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void typedBlockReadReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void fileBlockReadReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);
LOCAL void loopBackReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount);

/*
 * indexed by df1ProtoType
 */
LOCAL reqProcFunc reqJumpTable[] = {
		doNothingReqProc,
		blockReadReqProc,
		blockWriteReqProc,
		typedBlockReadReqProc,
		typedBlockWriteReqProc,
		bitWriteReqProc,
		readModifyWriteReqProc,
		loopBackReqProc,
		fileBlockWriteReqProc,
		fileBlockReadReqProc
};

typedef unsigned long (*resProcFunc) (absTransaction *pTrans, 
				abDf1ResProto *pProto, unsigned byteCount); 

LOCAL unsigned long doNothingResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long blockWriteResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long readModifyWriteResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long typedBlockWriteResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long fileBlockWriteResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long bitWriteResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long blockReadResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long typedBlockReadResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long fileBlockReadResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);
LOCAL unsigned long loopBackResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount);

/*
 * indexed by df1ProtoType
 */
LOCAL resProcFunc resJumpTable[] = {
		doNothingResProc,
		blockReadResProc,
		blockWriteResProc,
		typedBlockReadResProc,
		typedBlockWriteResProc,
		bitWriteResProc,
		readModifyWriteResProc,
		loopBackResProc,
		fileBlockWriteResProc,
		fileBlockReadResProc
};

LOCAL int drvAbDf1ParseInput (FILE *fp, drvSerialResponse *pResp, void *pPrivate);
LOCAL void drvAbDf1SetNoMsgMode (drvAbDf1Parm *pDev, drvSerialResponse *pResp);
LOCAL void drvAbDf1SetMsgMode (drvAbDf1Parm *pDev, drvSerialResponse *pResp);
LOCAL int drvAbDf1Ack (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1Enq (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1NegAck (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1UnexpectedProto (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1ReadNextMsgChar (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1DiscardInput (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1ExpectedSTX (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1UnexpectedSTX (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1ExpectedETX (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1UnexpectedETX (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1ExpectedDLE (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1UnexpectedDLE (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev);

typedef int32_t drvAbDf1Status;

/*
 * jump table for commands accepted by the DF1 driver
 * when it is a server
 */
typedef void (*processCMD)(drvAbDf1Parm *pDev, drvSerialResponse *pResp);

/* 
 * commands to this driver when it is a server
 */
LOCAL void cmdGeneralIO (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdBlockWrite (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdBitWrite (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdBlockRead (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdInvalid (drvAbDf1Parm *, drvSerialResponse *pCmd);


LOCAL processCMD cmdJumpTable[] =
{
	cmdBlockWrite, /* Block Write, Limited (All PLCs)      */
	cmdBlockRead, /* Block Read (All PLCs)                */
	cmdBitWrite, /* Bit-Masked Write, Limited (All PLCs) */
	cmdInvalid, /* Physical Write (PLC-2 & 1774)        */
	cmdInvalid, /* Physical Read (PLC-2 & 1774)         */
	cmdBitWrite, /* Bit-Masked Write, All Areas (All PLCs)*/
	cmdInvalid, /* Diagnostic Commands (All PLCs)       */
	cmdInvalid, /* Mode Enable/Disable (PLC-2 & 1774)   */
	cmdBlockWrite, /* Block Write, All Areas (All PLCs)    */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* General I/O Command (PLC-4)          */
	cmdGeneralIO, /* General I/O Command (PLC-3 & PLC-5)  */
};

/*
 * general commands to this driver when it is a server
 */
LOCAL void cmdFileBlockWrite (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdFileBlockRead (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdTypedBlockWrite (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdTypedBlockRead (drvAbDf1Parm *, drvSerialResponse *pCmd);
LOCAL void cmdReadModifyWrite (drvAbDf1Parm *, drvSerialResponse *pCmd);

LOCAL processCMD plc5GeneralCmdJumpTable[] =
{
	/*
	 * 0
	 */
	cmdFileBlockWrite, /* Write Range of Words (PLC-3 & PLC-5) */
	cmdFileBlockRead, /* Read Range of Words (PLC-3 & PLC-5)  */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 1
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 2
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdReadModifyWrite, /* Read/Modify/Write Specified Bits */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 3
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 4
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 5
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 6
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdTypedBlockWrite, /* Write Typed Data (Check File Type) */
	cmdTypedBlockRead, /* Read Data Words & Type Information */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 7
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 8
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * 9
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * a
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * b
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * c
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * d
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * e
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid, /* f */
	/*
	 * f
	 */
	cmdInvalid, /* 0 */
	cmdInvalid, /* 1 */
	cmdInvalid, /* 2 */
	cmdInvalid, /* 3 */
	cmdInvalid, /* 4 */
	cmdInvalid, /* 5 */
	cmdInvalid, /* 6 */
	cmdInvalid, /* 7 */
	cmdInvalid, /* 8 */
	cmdInvalid, /* 9 */
	cmdInvalid, /* a */
	cmdInvalid, /* b */
	cmdInvalid, /* c */
	cmdInvalid, /* d */
	cmdInvalid, /* e */
	cmdInvalid  /* f */
};

typedef struct {
	absBlockIO *pIO;
	unsigned elemNo;
	unsigned elemCount;
	unsigned elemType; /* allows subelement type to be different from file type */
}localIODescriptor;


/*
 * drvAbDf1.c globals
 */
LOCAL struct {
	ELLLIST links;
	SEM_ID lock;
}drvAbDf1Global;

/*
 * driver support entry table
 */
typedef long    drvInitFunc_t (void);
typedef long    drvReportFunc_t (int level);
LOCAL drvReportFunc_t drvAbDf1Report;
LOCAL drvInitFunc_t drvAbDf1Init;
struct
{
	long            number;
	drvReportFunc_t *report;
	drvInitFunc_t  *init;
}               drvAbDf1 =
{
	2L,
	drvAbDf1Report,
	drvAbDf1Init
};

/* 
 * initialize (or at least clear) 
 * all bytes in the an allocated element io structure 
 * (this allows the driver to add a driver specific
 * preamble to the element IO structure)
 */
LOCAL abDf1ElemIO *drvAbDf1NewElemIO (void);

/*
 * To be called after all calls to "setupScan()" and
 * setupWriteCache()" above have been made. This 
 * initiates scan and begins sending write cache updates. 
 */
LOCAL void drvAbDf1InitiateAll (void); /* start all links */

/*
 * the pBitNo argument should be supplied NULL if this
 * is not bit field IO
 */
LOCAL long drvAbDf1SetupIO (const struct link *, abDf1ElemIO *, unsigned *pBitNo);

LOCAL void drvAbDf1SetupScan (const int, abDf1ElemIO*);

/*
 * initiate write operation
 * (currently integer words, bit strings, or floating point supported)
 * (see pCurrentWriteValue, pWriteCompletion call backs above)
 */
LOCAL long drvAbDf1InitiateWrite (abDf1ElemIO *);

/*
 * reads current value in the cache
 * 
 * o data type in the value union will match the
 * data type flag in the element io structure
 */
LOCAL long drvAbDf1ReadWord (abDf1ElemIO *, uint16_t *pVal);
LOCAL long drvAbDf1ReadBitString (abDf1ElemIO *pWordIO, uint16_t mask, uint16_t *pVal);
LOCAL long drvAbDf1ReadReal (abDf1ElemIO *pWordIO, float *pVal);

drvAbDf1FuncTable drvAbDf1Func = {
	drvAbDf1NewElemIO,
	drvAbDf1InitiateAll,
	drvAbDf1SetupIO,
	drvAbDf1SetupScan,
	drvAbDf1InitiateWrite,
	drvAbDf1ReadWord,
	drvAbDf1ReadBitString,
	drvAbDf1ReadReal
};

LOCAL INLINE unsigned computeCheckSum (const uint8_t *pFrame, unsigned byteCount);
#ifndef VALIDATE_FRAME_WITH_BCC
LOCAL INLINE unsigned computeByteCRC (unsigned crc, unsigned c);
#endif
LOCAL uint8_t *pushPLC5AddrLevel (uint8_t *pb, unsigned addr);

LOCAL uint8_t *pushPLC5Addr (uint8_t *pb, unsigned fileNo, 
								unsigned elemNo, unsigned subElemNo);
LOCAL uint8_t *pushPLC5TypeDataParam (uint8_t *pb, unsigned dataType);
LOCAL unsigned long fetchPLC5Addr (devAbDf1ParseAddressFunc *pParseAddress, 
				const uint8_t **ppb, unsigned *pFileNo, unsigned *pElemNo, 
				unsigned *pSubElemNo);
LOCAL const uint8_t *fetchPLC5AddrLevel(const uint8_t *pb, unsigned *pAddr);
LOCAL const uint8_t *fetchPLC5TypeDataParam (unsigned *pDataType,
								unsigned *pDataSize, const uint8_t *pb);
LOCAL drvSerialSendCB drvAbDf1SendRequest;
LOCAL drvSerialSendCB drvAbDf1SendRespCodeFromRequest;
LOCAL int drvAbDf1SendRespCode (FILE *pf, drvAbDf1Parm *pDev);
LOCAL int drvAbDf1QueueRespCode (drvAbDf1Parm *pDev, int code);
LOCAL long drvAbDf1QueueRequest (absTransaction *pTrans);
LOCAL void drvAbDf1Read (drvAbDf1Parm *pDev);
LOCAL void drvAbDf1ProcessResp (drvAbDf1Parm *pDev, drvSerialResponse *);
LOCAL void drvAbDf1ReportIOBlocks (drvAbDf1Parm *pDev, ELLLIST *pList, 
										const char *pName, unsigned level);
LOCAL void drvAbDf1ReportTransactions (drvAbDf1Parm *pDev, ELLLIST *pList, 
										const char *pName);
LOCAL void retireTransaction (drvAbDf1Parm *pDev, unsigned id, long status);
LOCAL int drvAbDf1Scan (drvAbDf1Parm * pDev);
LOCAL void insertExpiredReadIO (absBlockIO *pIOIn);
LOCAL absTransaction *drvAbDf1NewTrans (drvAbDf1Parm *pDev);
LOCAL absTransaction *drvAbDf1NewReadTrans (absBlockIO *pIO);
LOCAL absTransaction *drvAbDf1NewWriteTrans (drvAbDf1ElemIO *pElemIO);
LOCAL drvAbDf1Status absDisposeTransaction (absTransaction *pTrans);
LOCAL void drvAbDf1MoveTrans (absTransaction *pTrans, ELLLIST *pNewList);
LOCAL long drvAbDf1CreateCache (drvAbDf1Parm *pDev, drvAbDf1ElemIO *pIO, unsigned nodeNumber, 
				unsigned fileType, unsigned fileNo, ELLLIST *pAddList, absQueType queType, 
				unsigned maxElem, unsigned dataTypeClass);
LOCAL void setLastStatus (absBlockIO *pIO, long epicsStatus, int read);
LOCAL drvAbDf1Status drvAbDf1SendFrame (FILE *fp, const drvSerialRequest *pRequest, 
								drvAbDf1Parm *pDev, absPLCIO *pPLC);
LOCAL int drvAbDf1SendReply (FILE *fp, drvSerialRequest *pRequest);
LOCAL void reflectDF1Frame (const drvSerialResponse *pCmd, unsigned long status);
LOCAL unsigned long drvAbDf1WriteBlockRaw (absBlockIO *pIO, unsigned df1DT, 
			unsigned elemNo, unsigned subElemNo, unsigned nElem, const uint8_t *pBuf,
			unsigned typed);
LOCAL unsigned long drvAbDf1WriteBitsRaw (absBlockIO *pIO, 
			unsigned elemNo, unsigned subElemNo, uint8_t *pVal, uint8_t *pMask);
LOCAL long drvAbDf1CreateLink (const char *pName, drvAbDf1Parm **ppDev);
LOCAL void drvAbDf1InitiateLink (drvAbDf1Parm *pDev); /* start specified link */
LOCAL long parseAbDf1Address (const char *pAddr, drvAbDf1Parm **ppDev, 
				unsigned *pNodeNumber, drvAbDf1ElemIO *pIO, unsigned *pBitNo, 
				unsigned *pFileType, unsigned *pFileNo, unsigned *pDataTypeClass);
LOCAL drvAbDf1Status readLocalBlock (const localIODescriptor *pFirst, 
			const localIODescriptor *pSecond, unsigned subElemNo, uint8_t *pBytes, unsigned nBytes);
LOCAL drvAbDf1Status writeLocalBlock (drvAbDf1Parm *pDev, unsigned fileNo, 
		unsigned elemNo, unsigned subElemNo, unsigned nElem, 
		unsigned elemType, const uint8_t *pBytes);
LOCAL drvAbDf1Status locateBlockIO (const drvAbDf1Parm *pDev, unsigned fileNo,  
		unsigned elemNo, unsigned subElemNo, unsigned elemCount, 
		unsigned elemCountIsInBytes, unsigned elemType,
		localIODescriptor *pFirst, localIODescriptor *pSecond);
LOCAL unsigned long drvAbDf1WriteCompletion (drvAbDf1ElemIO *pEIO);
LOCAL int drvAbDf1TransTMO (ULONG ticksAtReq);
LOCAL absPLCIO *drvAbDf1CreatePLCIO (drvAbDf1Parm *pDev, unsigned nodeNumber);
LOCAL absFileIO *drbAbDf1CreateFileIO (drvAbDf1Parm *pDev, unsigned nodeNumber, 
				unsigned fileNo, unsigned scanPeriod, unsigned dataType, unsigned dataTypeClass);
LOCAL void drvAbDf1ReportPLCs (drvAbDf1Parm *pDev);
LOCAL void showLocalHashList (drvAbDf1Parm *pDev, unsigned level);
LOCAL void drvAbDf1PrintElemValue (unsigned dataType, void *pDataIn);
LOCAL unsigned long drvAbDf1VerifyTypeAndRange (absBlockIO *pIO, unsigned df1DT, 
			unsigned elemNo, unsigned subElemNo, unsigned nElem, void **ppData);

#define mkmask(NBITS) ((1<<(NBITS))-1)

unsigned drvAbDf1LoopBackStationNumber = UINT_MAX; /* loopback test run aginst this node if no UINT_MAX */

/*
 * Set the local node number for a specified port
 */
unsigned long drvAbDf1SetLocalNodeNo (const char *pPort, unsigned localNodeNo)
{
	long status;
	drvAbDf1Parm *pLink;

	if (localNodeNo>0xfe) {
		return S_drvAbDf1_badNodeNumber;
	}

	status = drvAbDf1Init();
	if (status) {
		errMessage (status, "unable to initialize DF1 driver");
		return status;
	}

	status = drvAbDf1CreateLink (pPort, &pLink);
	if (status) {
		errMessage (status, "unable to attach to serial port");
		return status;
	}

	/*
	 * force the node number (replace the default)
	 */
	pLink->nodeNo = localNodeNo;

	return status;
}

/*
 * Set the scan period for the specified port, nodeNo, fileNo
 */
unsigned long drvAbDf1SetScanPeriod (const char *pPort, 
					unsigned nodeNo, unsigned fileNo, unsigned period_mS)
{
	long status;
	drvAbDf1Parm *pLink;
	absFileIO *pFile;

	status = drvAbDf1Init();
	if (status) {
		errMessage (status, "unable to initialize DF1 driver");
		return status;
	}

	status = drvAbDf1CreateLink (pPort, &pLink);
	if (status) {
		errMessage (status, "unable to attach to serial port");
		return status;
	}

	pFile = drbAbDf1CreateFileIO (pLink, nodeNo, fileNo, period_mS, 
				df1DTNone, dtcUnknown);
	if (!pFile) {
		return S_drvAbDf1_noMemory;
	}

	/*
	 * force the scan period (replace the default)
	 */
	pFile->scanPeriod = (sysClkRateGet() *  period_mS) / 1000ul;

	return S_drvAbDf1_OK;
}

LOCAL int drvAbDf1DebugPrintf (unsigned level, char *pformat, ...)
{
	va_list	args;
	int	status;

	if (drvAbDf1Debug<level) {
		return 0;
	}

    va_start(args, pformat);

    status = epicsVprintf(pformat, args);

    va_end(args);

	return status;
}


/*
 * drvAbDf1Init()
 */
LOCAL long drvAbDf1Init(void)
{
	drvAbDf1Status status;
	int	opt;

	/*
	 * dont init twice
	 */
	if (drvAbDf1Global.lock) {
		return S_drvAbDf1_OK;
	}

	/*
	 * create a hash table for the file 
	 * name strings and associated MUTEX
	 */
	opt = SEM_Q_PRIORITY | SEM_DELETE_SAFE |
		SEM_INVERSION_SAFE;
	drvAbDf1Global.lock = semMCreate(opt);
	if (!drvAbDf1Global.lock)
	{
		status = S_dev_noMemory;
		errMessage(status,
			   ":drvAbDf1Init() semMCreate ()");
		return status;
	}

	ellInit(&drvAbDf1Global.links);

	return S_drvAbDf1_OK;
}


/*
 * drvAbDf1Report()
 */
LOCAL long drvAbDf1Report(int level)
{
	drvAbDf1Parm  *pDev;
	drvAbDf1Status status;

	/*
	 * check for init
	 */
	if (drvAbDf1Global.lock)
	{
		status = semTake(drvAbDf1Global.lock, WAIT_FOREVER);
		assert(status == OK);

		pDev = (drvAbDf1Parm *) ellFirst(&drvAbDf1Global.links);
		while (pDev)
		{
			double delay;
			printf("\tAB DF1 Link over %s\n",
				pDev->pName);
			if (level > 0) {
				printf("\t\tneg acks sent=%u (damaged frame or lcl buf full)\n",
					pDev->nakSendCount);
				printf("\t\tdamaged frames received=%u\n",
					pDev->damagedFrameCount);
				printf("\t\tneg acks recv=%u (damaged frame or rmt buf full)\n",
					pDev->nakRecvCount);
				printf("\t\tRequests that timed out with no ACK/NAK response %u\n",
					pDev->enqTimeoutCount);
				printf("\t\tRequests that timed out with too many NAKs %u\n",
					pDev->nakTimeoutCount);
				printf("\t\tRequests that timed out lacking a response %u\n",
					pDev->responseTimeoutCount);
				printf("\t\tDuplicate responses %u\n",
					pDev->dupResponseCount);
				printf("\t\tBad control char received %u\n",
					pDev->badControlRecvCount);
				delay = pDev->smoothedDelayToReadSend;
				delay /= sysClkRateGet();
				printf("\t\tSmoothed delay to read send begin %f sec\n",
					delay);
				delay = pDev->maxDelayToReadSend;
				delay /= sysClkRateGet();
				printf("\t\tMax delay to read send begin %f sec\n",
					delay);
				delay = pDev->smoothedDelayToWriteSend;
				delay /= sysClkRateGet();
				printf("\t\tSmoothed delay to write send begin %f sec\n",
					delay);
				delay = pDev->maxDelayToWriteSend;
				delay /= sysClkRateGet();
				printf("\t\tMax delay to write send begin %f sec\n",
					delay);
				delay = pDev->smoothedDelayToResp;
				delay /= sysClkRateGet();
				printf("\t\tSmoothed delay to resp %f sec\n",
					delay);
				delay = pDev->maxDelayToResp;
				delay /= sysClkRateGet();
				printf("\t\tMax delay to resp %f sec\n",
					delay);
				printf ("\t\tTotal frames transmitted %u\n",
					pDev->totFramesSent);
				printf ("\t\tTotal frames received %u\n",
					pDev->totFramesRecv);
			}
			if (level > 3) {
				printf("\t\tmutexSem ");
				semShow(pDev->mutexSem, level);
				printf("\t\tackSem");
				semShow(pDev->ackSem, level);
				printf("\t\tscanSem");
				semShow(pDev->scanSem, level);
				printf("\t\ttransaction id hash table");
				bucketShow(pDev->pTransBucket);
				if (pDev->pLocalHashList) {
					showLocalHashList (pDev, level);
				}
			}
			if (level > 2) {
				drvAbDf1ReportTransactions (pDev, 
					&pDev->transLimboList, "limbo");
				drvAbDf1ReportTransactions (pDev, 
					&pDev->transDSList, "pending delivery to drive serial");
				drvAbDf1ReportTransactions (pDev, 
					&pDev->transACKList, "pending PLC request acknowledge");
				drvAbDf1ReportTransactions (pDev, 
					&pDev->transResList, "pending PLC response acknowledge");
			}
			if (level > 1) {
				drvAbDf1ReportIOBlocks (pDev,
					&pDev->scanList, "idle", level);
				drvAbDf1ReportIOBlocks (pDev,
					&pDev->pendingList, "io pending", level);
				if (pDev->pLocalHashList) {
					showLocalHashList (pDev, level);
				}
				drvAbDf1ReportPLCs (pDev);
			}
			pDev = (drvAbDf1Parm *) ellNext(&pDev->node);

		}
		status = semGive(drvAbDf1Global.lock);
		assert(status == OK);
	}

	return S_drvAbDf1_OK;
}

/*
 * showLocalHashList()
 */
LOCAL void showLocalHashList (drvAbDf1Parm *pDev, unsigned level)
{
	unsigned i;
	unsigned maxEntries = 0u;

	printf("\t\tlocal variable hash table\n");

	for (i=0u; i<pDev->localHashSize; i++) {
		unsigned listCount = (unsigned)
			ellCount(&pDev->pLocalHashList[i]);
		maxEntries = max (maxEntries, listCount);
		if (ellCount(&pDev->pLocalHashList[i])) {
			drvAbDf1ReportIOBlocks (pDev, 
				&pDev->pLocalHashList[i], "local", level);
		}
	}
	printf("\t\tlocal variable hash table has max=%u entries id\n",
		maxEntries);
}
					
/*
 * drvAbDf1ReportTransactions
 */
LOCAL void drvAbDf1ReportTransactions (drvAbDf1Parm *pDev, ELLLIST *pList, const char *pName)
{
	absTransaction *pTrans;
	double delay;
	int status;

	printf ("\t\ttransactions in %s list (count=%u):\n", pName, ellCount(pList));
	status = semTake (pDev->mutexSem, MUTEX_TMO);
	if (status) {
		printf ("\t\ttimed out\n");
		return;
	}
	for (pTrans = (absTransaction *)ellFirst (pList);
		pTrans; pTrans = (absTransaction *) ellNext (&pTrans->node)) {

		delay = tickGet() - pTrans->ticksAtReq;
		delay /= sysClkRateGet();
		
		switch (pTrans->pIO->pFile->dataTypeClass) {
		case dtcLoopBack:
			printf(
		"\t\tloop back transaction id=%u delay=%f node=%u\n",
					pTrans->transId,
					delay,
					pTrans->pIO->pFile->pPLC->nodeNo);
			break;

		case dtcTyped:
			printf(
		"\t\ttyped %s transaction id=%u delay=%f node=%u file=%u\n",
					pTrans->read?"read":"write",
					pTrans->transId,
					delay,
					pTrans->pIO->pFile->pPLC->nodeNo,
					pTrans->pIO->pFile->fileNo);
			break;

		case dtcUntyped:
			printf(
		"\t\tuntyped %s transaction id=%u delay=%f node=%u\n",
					pTrans->read?"read":"write",
					pTrans->transId,
					delay,
					pTrans->pIO->pFile->pPLC->nodeNo);
			break;

		default:
		case dtcUnknown:
			printf(
		"\t\tunknown purpose transaction id=%u delay=%f node=%u\n",
					pTrans->transId,
					delay,
					pTrans->pIO->pFile->pPLC->nodeNo);
			break;
		}
		printf(
	"\t\t\telem=%u N elem=%u data type=\"%s\"\n",
				pTrans->pElemIO ? pTrans->pElemIO->elemNo : pTrans->pIO->elemNo,
				pTrans->pElemIO ? 1u : pTrans->pIO->elemCount,
				abDataTypeToString (pTrans->pIO->pFile->dataType));
	}
	status = semGive (pDev->mutexSem);
	assert (status==OK);
}

/*
 * drvAbDf1ReportPLCs
 */
LOCAL void drvAbDf1ReportPLCs (drvAbDf1Parm *pDev)
{
	absPLCIO *pPLC;
	absFileIO *pFileIO;
	int status;

	printf ("\t\tPLC List:\n");
	status = semTake (pDev->mutexSem, MUTEX_TMO);
	if (status) {
		printf ("\t\ttimed out\n");
		return;
	}
	
	for (pPLC = (absPLCIO *) ellFirst (&pDev->plcIOList);
		pPLC; pPLC = (absPLCIO *) ellNext (&pPLC->node)) {

		printf(
	"\t\tPLC at address %u oustanding IO count %u \n",
				pPLC->nodeNo, pPLC->ioOutstandingCount);
		for (pFileIO = (absFileIO *) ellFirst (&pPLC->fileList);
			pFileIO; pFileIO = (absFileIO *) ellNext (&pFileIO->node)) {

			printf(
	"\t\t\tFile %u, period %g, %s, %s\n",
					pFileIO->fileNo, ((double)pFileIO->scanPeriod)/sysClkRateGet(),
					abDataTypeToString(pFileIO->dataType), 
					dtClassLabel[pFileIO->dataTypeClass]);
		}
	}

	status = semGive (pDev->mutexSem);
	assert (status==OK);
}

/*
 * drvAbDf1ReportIOBlocks
 */
LOCAL void drvAbDf1ReportIOBlocks (drvAbDf1Parm *pDev, ELLLIST *pList, 
								   const char *pName, unsigned level)
{
	absBlockIO *pIO;
	int status;
	double delay, period;
	ULONG current;
	unsigned i, j;
	
	printf("\t\tIO cache list with state = \"%s\" (count=%u):\n", 
				pName, ellCount(pList));
	status = semTake (pDev->mutexSem, MUTEX_TMO);
	if (status) {
		printf ("\ttimed out\n");
		return;
	}
	for (pIO = (absBlockIO *)ellFirst (pList);
		pIO; pIO = (absBlockIO *)ellNext (&pIO->node)) {

		switch (pIO->pFile->dataTypeClass) {
		case dtcLoopBack:
			printf(
	"\t\tnode=%d\n",
				pIO->pFile->pPLC->nodeNo);
			break;
		case dtcTyped:
			printf(
	"\t\tnode=%d file=%u elem=%d N elem=%u data type=\"%s\"\n",
				pIO->pFile->pPLC->nodeNo,
				pIO->pFile->fileNo,
				pIO->elemNo,
				pIO->elemCount,
				abDataTypeToString(pIO->pFile->dataType));
			break;
		case dtcUntyped:
			printf(
	"\t\tnode=%d octal word no=%o n words=%u\n",
				pIO->pFile->pPLC->nodeNo,
				pIO->elemNo,
				pIO->elemCount);
			break;
		default:
			printf ("unknown purpose");
			break;
		}
		printf(
	"\t\t\tIO OP count=%u\n", pIO->ioCount);

		current = tickGet();
		if (pIO->ticksAtScanCompletion==0ul) {
			delay = 0.0;
		}
		else {
			if (current>=pIO->ticksAtScanCompletion) {
				delay = current - pIO->ticksAtScanCompletion;
			}
			else {
				delay = (ULONG_MAX - pIO->ticksAtScanCompletion) + current;
			}
		}

		switch (pIO->queue) {
		case absQuePending:
			delay -= pIO->pFile->scanPeriod;
			delay /= sysClkRateGet();
			printf(
		"\t\t\tio pending %f sec\n",
				delay);
			break;

		case absQueScan:
			delay /= sysClkRateGet();
			period = pIO->pFile->scanPeriod;
			period /= sysClkRateGet();
			printf(
		"\t\t\tidle %f sec out of %f sec\n",
				delay, period);
			break;

		default:
			break;
		}

		if (pIO->lastReadStatus) {
			char buf[512];
			long errlkupStatus;

			errlkupStatus = errSymFind (pIO->lastReadStatus, buf);
			if (errlkupStatus) {
				sprintf(buf, "Unknown error code=%lx", pIO->lastReadStatus);
			}
			printf ("\t\t\tRIO \"%s\"\n", buf);
		}
		if (pIO->lastWriteStatus) {
			char buf[512];
			long errlkupStatus;

			errlkupStatus = errSymFind (pIO->lastWriteStatus, buf);
			if (errlkupStatus) {
				sprintf(buf, "Unknown error code=%lx", pIO->lastWriteStatus);
			}
			printf ("\t\t\tWIO \"%s\"\n", buf);
		}
		if (level>2u) {
			char *pData = pIO->pData;

			for (i=pIO->elemNo; i<pIO->elemNo+pIO->elemCount; i++) {

				if (abDataStructure(pIO->pFile->dataType)) {
					const subelementSymbol *pSym = abSubElemTable(pIO->pFile->dataType);
					
					printf ("\t\t\telement = %4u\n", i);
					for (j=0u; j<abDataSize(pIO->pFile->dataType)/sizeof(df1Word); j++) {
						if (pSym[j].symbol[0] == '\0') {
							continue;
						}
						printf ("\t\t\t\t%s = ", pSym[j].symbol);
						drvAbDf1PrintElemValue (pSym[j].type, 
									pData + abDataSize(df1DTInt)*j);
					}
				}
				else {
					printf ("\t\t\telem =%4u value = ", i);
					drvAbDf1PrintElemValue (pIO->pFile->effectiveDataType, pData);
				}
				pData += abDataSize(pIO->pFile->dataType);
			}
		}
	}
	status = semGive(pDev->mutexSem);
	assert(status==OK);
}

/*
 * drvAbDf1PrintElemValue()
 */
LOCAL void drvAbDf1PrintElemValue (unsigned dataType, void *pDataIn)
{
	switch (dataType) {
		case df1DTInt:
		{
			uint16_t *pData = (uint16_t *) pDataIn;
			printf ("0x%08x\n", *pData);
			break;
		}
		case df1DTFP:
		{
			float *pData = (float *) pDataIn;
			printf ("%g\n", *pData);
			break;
		}
		default:
		{
			printf ("\n");
			break;
		}
	}
}


/*
 * drvAbDf1CreateLink()
 */
LOCAL long
drvAbDf1CreateLink (const char *pName, drvAbDf1Parm **ppLink)
{
	drvAbDf1Parm  *pDev;
	drvAbDf1Status status;
	unsigned i;

	if (!drvAbDf1Global.lock)
	{
		errMessage(S_drvAbDf1_noInit, NULL);
		return S_drvAbDf1_noInit;
	}

	/*
	 * see if another task has attached to this link 
	 * for AB serial already
	 */
	status = drvSerialAttachLink (pName, 
				drvAbDf1ParseInput, (void **)ppLink);
	if (status == S_drvSerial_OK) {
		return S_drvAbDf1_OK;
	}
	else if (status != S_drvSerial_noneAttached) {
		return status;
	}

	pDev = (drvAbDf1Parm *) calloc (1, sizeof(*pDev));
	if (!pDev)
	{
		status = S_dev_noMemory;
		errMessage(status,
			   ":drvAbDf1CreateLink() calloc()");
		return status;
	}

	/*
	 * create free list for DF1 transactions
	 */
	freeListInitPvt (&pDev->pTransFreeListPVT, sizeof(absTransaction), 1024);

	/*
	 * there should to be a way to specify the node number for
	 * each link independently
	 */
	pDev->nodeNo = drvAbDf1SrcStationNumber;
	
	pDev->mutexSem = semMCreate (SEM_Q_PRIORITY|
				SEM_DELETE_SAFE|SEM_INVERSION_SAFE);
	if (pDev->mutexSem == NULL)
	{
		free (pDev);
		status = S_dev_noMemory;
		errMessage (status,
			   ":drvAbDf1CreateLink() semMCreate()");
		return status;
	}
	pDev->ackSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	if (!pDev->ackSem)
	{
		semDelete (pDev->mutexSem);
		free (pDev);
		status = S_dev_noMemory;
		errMessage (status,
			   ":drvAbDf1Init() semBCreate ()");
		return status;
	}
	pDev->scanSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	if (!pDev->scanSem)
	{
		semDelete (pDev->ackSem);
		semDelete (pDev->mutexSem);
		free (pDev);
		status = S_dev_noMemory;
		errMessage (status,
			   ":drvAbDf1Init() semBCreate ()");
		return status;
	}

	/*
	 * MUTEX around use of hash table 
	 */
	status = semTake (drvAbDf1Global.lock, WAIT_FOREVER);
	assert (status == OK);

	ellAdd (&drvAbDf1Global.links, &pDev->node);

	/*
	 * MUTEX off around use of hash table 
	 */
	status = semGive (drvAbDf1Global.lock);
	assert(status == OK);

	pDev->pTransBucket = bucketCreate(256);
	if (!pDev->pTransBucket) {
		semDelete (pDev->scanSem);
		semDelete (pDev->mutexSem);
		semDelete (pDev->ackSem);
		free (pDev);
		status = S_dev_noMemory;
		errMessage (status, "semMCreate()");
		return status;
	}

	pDev->lastAckSent = df1dlNAK;
	ellInit (&pDev->pendingList);
	ellInit (&pDev->scanList);
	ellInit (&pDev->localList);
	ellInit (&pDev->transACKList);
	ellInit (&pDev->transResList);
	ellInit (&pDev->transLimboList);
	ellInit (&pDev->transDSList);
	ellInit (&pDev->plcIOList);

	assert (strlen(pName) < sizeof(pDev->pName) - 1);
	strncpy (pDev->pName, pName, sizeof(pDev->pName)-1);
	pDev->pName[sizeof(pDev->pName)-1]='\0';

	/*
	 * next fill in a handler for each of the
	 * expected control characters
	 */
	for (i=0; i<NELEMENTS(pDev->inHandlerList); i++) {
		pDev->inHandlerList[i] = drvAbDf1UnexpectedProto;
	}
	drvAbDf1SetNoMsgMode (pDev, NULL);
	pDev->inHandlerList[df1dlACK] = drvAbDf1Ack;
	pDev->inHandlerList[df1dlNAK] = drvAbDf1NegAck;
	pDev->inHandlerList[df1dlENQ] = drvAbDf1Enq;

	/*
	 * start up link only after handler stubs are installe above
	 */
	status = drvSerialCreateLink(
			pName, drvAbDf1ParseInput, pDev, &pDev->id);
	if (status) {
		semDelete (pDev->scanSem);
		semDelete (pDev->mutexSem);
		semDelete (pDev->ackSem);
		free (pDev);
		errMessage (status,
			   ":drvAbDf1Init() drvSerialCreateLink ()");
		return status;
	}

	/*
	 * set their handle to this serial link
	 */
	*ppLink = pDev;

	return S_drvAbDf1_OK;
}

/*
 * drvAbDf1InitiateAll()
 * (start all links)
 */
void drvAbDf1InitiateAll(void)
{
	drvAbDf1Parm  *pDev;

	/*
	 * check for init
	 */
	if (!drvAbDf1Global.lock)
	{
		printf("drvAbDf1 not installed\n");
		return;
	}

	/*
	 * start up all devices
	 */
	pDev = (drvAbDf1Parm *) ellFirst(&drvAbDf1Global.links);
	while (pDev)
	{
		drvAbDf1InitiateLink (pDev);
		pDev = (drvAbDf1Parm *) ellNext(&pDev->node);
	}
}

/*
 * drvAbDf1InitiateLink()
 *
 * after all blocks are configured move them
 * to the scan list where they will become active
 */
LOCAL void drvAbDf1InitiateLink (drvAbDf1Parm *pDev)
{
	long status;
	absBlockIO *pIO;
	absBlockIO *pIIO;
	ELLLIST *pList;
	unsigned localHashSize;
	ULONG current;

	status = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (status==OK);
	
	if (!pDev->scanInit) {

		/*
		 * build hash table for local variables
		 * (now that we know how many there are)
		 */
		localHashSize = ellCount(&pDev->localList);

		if (localHashSize) {

			pDev->pLocalHashList = (ELLLIST *) 
				calloc (localHashSize, sizeof(ELLLIST));
			if (pDev->pLocalHashList) {

				pDev->localHashSize = localHashSize;

				/*
				 * place blocks in local PV hash table
				 */
				while ( (pIO = (absBlockIO *) ellGet(&pDev->localList)) ) {

					pIO->localId = pIO->pFile->fileNo ^ (pIO->elemNo/absLocalElemCount); 
					pIO->localId %= pDev->localHashSize;

					/*
					 * There isnt an undefined status code in the DF1
					 * error set, and we dont have a mechanism
					 * that allows each element to be tagged when it
					 * has been initialized.
					 */
					pIO->lastReadStatus = S_drvAbDf1_OK;
					pIO->lastWriteStatus = S_drvAbDf1_OK;

					pList = &pDev->pLocalHashList[pIO->localId];
					for (pIIO = (absBlockIO *) ellFirst(pList); 
							pIIO; pIIO = (absBlockIO *) ellNext(&pIIO->node)) {
						if (pIIO->elemNo/absLocalElemCount == pIO->elemNo/absLocalElemCount 
								&& pIIO->pFile->fileNo == pIO->pFile->fileNo) {
							errPrintf(status, __FILE__, __LINE__,
								"Unable to export local DF1 variable cache file=%u elem=%u",
									pIO->pFile->fileNo, pIO->elemNo);
							break;
						}
					}
					
					if (!pIIO) {
						ellAdd (pList, &pIO->node);
					}
				}
			}
		}

		/*
		 * set up fake IO block for loop back tests
		 */
		if (drvAbDf1LoopBackStationNumber!=UINT_MAX) {
			absFileIO *pFileIO;

			pFileIO = drbAbDf1CreateFileIO (pDev, drvAbDf1LoopBackStationNumber, 0, 
				(drvAbDf1DefaultScanPeriod_mS * sysClkRateGet()) / 1000ul, df1DTNone, dtcLoopBack);
			if (pFileIO) {
				pIO = (absBlockIO *) calloc (1, sizeof(*pIO));

				if (pIO) {
					pIO->pFile = pFileIO; 
					pIO->queue = absQueScan;
					ellAdd (&pDev->scanList, &pIO->node);
				}
				else {
					errMessage (S_drvAbDf1_noMemory, "unable to initialize loop back IO");
				}
			}
			else {
				errMessage (S_drvAbDf1_noMemory, "unable to initialize loop back IO");
			}
		}

		/*
		 * initialize the scan timers
		 * so that we scan everything once at startup
		 */
		current = tickGet();
		for (pIO = (absBlockIO *) ellFirst(&pDev->scanList); 
					pIO; pIO = (absBlockIO *) ellNext(&pIO->node)) {

			/* 
			 * set ticks at next scan 
			 * (fully expect underflow - and react to it see 
			 * delay calc in drvAbDf1Scan())
			 */
			pIO->ticksAtScanCompletion = (current - pIO->pFile->scanPeriod) - 1ul;

			/*
			 * this staggers the initial scan requests in time
			 * (delay between requests is 0.1 sec)
			 */
			current += sysClkRateGet()/10ul;
		}

		/*
		 * start the scan task
		 */
		pDev->taskId = taskSpawn(
			"AB DF1",		/* task name */
			tp_drvAbDf1Priority,	/* priority */
			tp_drvAbDf1Options,	/* options */
			tp_drvAbDf1StackSize,/* stack size */
			drvAbDf1Scan,	/* task entry point */
			(int) pDev,		/* args (1-10) of int */
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0);
		if (!pDev->taskId) {
			errMessage (S_dev_noMemory,
				   ":drvABInitiateScan() taskSpawn()");
			status = semGive(pDev->mutexSem);
			assert (status==OK);
			return;
		}

		/*
		 * notify the scan task
		 */
		status = semFlush (pDev->scanSem);
		assert (status==OK);

		pDev->scanInit = TRUE;
	}

	status = semGive (pDev->mutexSem);
	assert (status==OK);
}

/*
 * drvAbDf1Scan()
 *
 * this routine does not take the mutex lock 
 * at the start of the scan timer loop so that
 * it will not have the mutex lock when call backs
 * are called (and potentilly cause a deadlock)
 */
LOCAL int drvAbDf1Scan (drvAbDf1Parm *pDev)
{
	absBlockIO *pIO;
	int vxStatus;
	long status;
	ULONG delay = 0ul;
	ULONG current;

	while (TRUE) {

		semTake (pDev->scanSem, delay);

		/*
		 * process all responses pending in drvSerial
		 */
		drvAbDf1Read (pDev);

		vxStatus = semTake (pDev->mutexSem, MUTEX_TMO);
		if (vxStatus!=OK) {
			/*
			 * we will delay at least MUTEX_TMO if
			 * the mutex is unavailable
			 */
			delay = 0ul;
			continue;
		}

		/*
		 * initiate scan for all blocks in the scan queue
		 * whose scan timers have expired
		 */
		current = tickGet();
		while (TRUE) {
			absTransaction *pTrans;

			pIO = (absBlockIO *) ellFirst (&pDev->scanList);

			if (!pIO) {

				vxStatus = semGive (pDev->mutexSem);
				assert (vxStatus==OK);

				delay = RESPONSE_TMO/2UL;
				break;
			}

			/*
			 * expect that overflow is possible
			 */
			if (current>=pIO->ticksAtScanCompletion) {
				delay = current - pIO->ticksAtScanCompletion;
			}
			else {
				delay = current + (ULONG_MAX - pIO->ticksAtScanCompletion);
			}

			/*
			 * scan blocks are stored in expiration order
			 */
			if (delay<pIO->pFile->scanPeriod) {

				delay = min (RESPONSE_TMO/2UL, pIO->pFile->scanPeriod - delay);

				vxStatus = semGive (pDev->mutexSem);
				assert (vxStatus==OK);

				break;
			}

			pTrans = drvAbDf1NewReadTrans (pIO);
			if (!pTrans) {

				vxStatus = semGive (pDev->mutexSem);
				assert (vxStatus==OK);
				
				/*
				 * place records in alarm state
				 *
				 * care is taken to not have the mutex when
				 * this is called so we will not dead lock with the
				 * EPICS database
				 */
				setLastStatus (pIO, S_drvAbDf1_noBuf, TRUE);

				delay = RESPONSE_TMO/2UL;

				break;
			}

			pTrans->protoType = dtClassReadProto[pIO->pFile->dataTypeClass];

			status = drvAbDf1QueueRequest (pTrans);
			if (status==S_drvAbDf1_OK) {
				assert (pIO->queue==absQueScan);
				ellDelete (&pDev->scanList, &pIO->node);
				pIO->queue = absQuePending;
				ellAdd (&pDev->pendingList, &pIO->node);
			}
			else {
				/*
				 * the only failure is a mutex tmo
				 * here which should never occur
				 * (because we already have the mutex)
				 */
				vxStatus = semGive (pDev->mutexSem);
				assert (vxStatus==OK);

				delay = RESPONSE_TMO/2UL;
				break;
			}
		}
	}

	/*
	 * never here
	 */
}

/*
 * insertExpiredReadIO
 */
LOCAL void insertExpiredReadIO (absBlockIO *pIOIn)
{
	ULONG delay;
	int vwStatus;
	absBlockIO *pIO;

	vwStatus = semTake (pIOIn->pFile->pPLC->pDev->mutexSem, WAIT_FOREVER);
	assert (vwStatus==OK);

	/*
	 * the scan delay is counted from IO
	 * completion so that over time nothing
	 * is synchronized, and we hopefully end
	 * up with demand driven load balancing
	 */
	pIOIn->ticksAtScanCompletion = tickGet();

	/*
	 * insert into the list in scan order
	 * BINARY SEARCH HERE COULD BE SLOW
	 */
	for ( pIO = (absBlockIO *) ellLast (&pIOIn->pFile->pPLC->pDev->scanList);
			pIO; pIO = (absBlockIO *) ellPrevious (&pIO->node)) {
		/*
		 * expect that overflow is possible
		 */
		if (pIOIn->ticksAtScanCompletion>=pIO->ticksAtScanCompletion) {
			delay = pIOIn->ticksAtScanCompletion - pIO->ticksAtScanCompletion;
		}
		else {
			delay = pIOIn->ticksAtScanCompletion + 
					(ULONG_MAX - pIO->ticksAtScanCompletion);
		}

		if (delay<=pIOIn->pFile->scanPeriod) {
			ellInsert (&pIOIn->pFile->pPLC->pDev->scanList, &pIO->node, &pIOIn->node);
			break;
		}
	}

	if (pIO==NULL) {
		ellInsert (&pIOIn->pFile->pPLC->pDev->scanList, NULL, &pIOIn->node);
	}

	pIOIn->queue = absQueScan;

	vwStatus = semGive (pIOIn->pFile->pPLC->pDev->mutexSem);
	assert (vwStatus==OK);

	if (pIO==NULL) {
		/*
		 * we added an item to the front of the scan queue
		 * so fill the scan tasks sem
		 */
		vwStatus = semFlush (pIOIn->pFile->pPLC->pDev->scanSem);
		assert (vwStatus==OK);
	}
}

/*
 * NOOP req specific processing
 */
LOCAL void doNothingReqProc (absTransaction *pTrans,
				abDf1ReqProto *pReq, unsigned *pByteCount)
{
}

/*
 * write req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void blockWriteReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pData;
	abDf1Value val;

	/*
	 * fetch the current value to be written
	 */
	pTrans->pElemIO->dev.devFunc->pCurrentWriteValue (&pTrans->pElemIO->dev, &val);

	/*
	 * block writes always assume element size 
	 * is a df1 word
	 */
	pReq->hdr.cmd = df1CmdProtectedBlockWrite;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->blockWrite.lsAddr = pTrans->pElemIO->elemNo<<1u;
	pReq->blockWrite.msAddr = pTrans->pElemIO->elemNo>>(NBBY-1u);
	*pByteCount = sizeof(pReq->hdr)
		+ sizeof(pReq->blockWrite.lsAddr)
		+ sizeof(pReq->blockWrite.msAddr)
		+ sizeof(df1Word); 
	pData = pReq->blockWrite.data.bytes;
	*(pData++) = (uint8_t) val.word;
	*(pData++) = (uint8_t) (val.word>>NBBY);
}

/*
 * bit write req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void bitWriteReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	unsigned i, j, clrMask, setMask;
	abDf1Value val;

	/*
	 * fetch the current value to be written
	 */
	pTrans->pElemIO->dev.devFunc->pCurrentWriteValue (&pTrans->pElemIO->dev, &val);

	/*
	 * block writes always assume element size 
	 * is a df1 word
	 */
	pReq->hdr.cmd = df1CmdProtectedBitWrite;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;

	/* 
	 * this is complicated by the fact that the masks are stored in PLC5
	 * and/or mask format and is very dependent on the PLC5 bit AND op first
	 * followed by bit OR op order
	 */
	i=0u;
	clrMask = ~val.bitString.value & val.bitString.mask;
	setMask = val.bitString.value & val.bitString.mask;
	for (j=0; j<sizeof(df1Word); j++) {
		if ( (clrMask^setMask)&0xff ) {
			pReq->bitWrite.bits[i].lsAddr = 
				(uint8_t) ((pTrans->pElemIO->elemNo<<1u)+j);
			pReq->bitWrite.bits[i].msAddr = 
				(uint8_t) (pTrans->pElemIO->elemNo>>(NBBY-1u));
			pReq->bitWrite.bits[i].setBits = (uint8_t) setMask;
			pReq->bitWrite.bits[i].clrBits = (uint8_t) clrMask;
			i++;
		}
		clrMask >>= NBBY;
		setMask >>= NBBY;
	}
	if (i==0u) {
		*pByteCount = 0u;
	}
	else {
		*pByteCount = sizeof(pReq->hdr) + sizeof(pReq->bitWrite.bits[0])*i;
	}
}

/*
 * typed write req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void typedBlockWriteReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pb;
	abDf1Value val;

	/*
	 * fetch the current value to be written
	 */
	pTrans->pElemIO->dev.devFunc->pCurrentWriteValue (&pTrans->pElemIO->dev, &val);

	pReq->hdr.cmd = df1CmdGeneral;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->typedWrite.fnc = df1FncTypedWrite;
	pReq->typedWrite.lsPO = 0u;
	pReq->typedWrite.msPO = 0u;
	pReq->typedWrite.lsTT = 1u;
	pReq->typedWrite.msTT = 0u;

	/*
	 * PLC5 system address is the first part 
	 * of the data
	 */
	pb = pushPLC5Addr (pReq->typedWrite.data.bytes, 
			pTrans->pIO->pFile->fileNo, pTrans->pElemIO->elemNo,
			pTrans->pElemIO->subElemNo);

	/*
	 * next we place the type/data parameter into
	 * the stream
	 */
	pb = pushPLC5TypeDataParam (pb, pTrans->pElemIO->dev.dataType);

	/*
	 * followed by the data (one word)
	 */
	switch (pTrans->pElemIO->dev.dataType) {
	case df1DTInt:
		*(pb++) = (uint8_t) val.word;
		*(pb++) = (uint8_t) (val.word>>NBBY);
		break;

	case df1DTFP:
	{
		uint32_t *plw = (uint32_t *) &val;
		assert (sizeof(val.real==sizeof(*plw)));
		*(pb++) = (uint8_t) (*plw>>(0*NBBY));
		*(pb++) = (uint8_t) (*plw>>(1*NBBY));
		*(pb++) = (uint8_t) (*plw>>(2*NBBY));
		*(pb++) = (uint8_t) (*plw>>(3*NBBY));
		break;
	}
	default:
		break;
	}

	*pByteCount = pb - pReq->buf;
}

/*
 * file block write req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void fileBlockWriteReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pb;
	abDf1Value val;

	/*
	 * fetch the current value to be written
	 */
	pTrans->pElemIO->dev.devFunc->pCurrentWriteValue (&pTrans->pElemIO->dev, &val);

	pReq->hdr.cmd = df1CmdGeneral;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->fileBlockWrite.fnc = df1FncWordRangeWrite;
	pReq->fileBlockWrite.lsPO = 0u;
	pReq->fileBlockWrite.msPO = 0u;
	pReq->fileBlockWrite.lsTT = abDataSize(pTrans->pElemIO->dev.dataType)/sizeof(df1Word);
	pReq->fileBlockWrite.msTT = 0u;

	/*
	 * PLC5 system address is the first part 
	 * of the data
	 */
	pb = pushPLC5Addr (pReq->typedWrite.data.bytes, 
			pTrans->pIO->pFile->fileNo, pTrans->pElemIO->elemNo,
			pTrans->pElemIO->subElemNo);

	/*
	 * followed by the data (one word)
	 */
	switch (pTrans->pElemIO->dev.dataType) {
	case df1DTInt:
		*(pb++) = (uint8_t) val.word;
		*(pb++) = (uint8_t) (val.word>>NBBY);
		break;

	case df1DTFP:
	{
		/*
		 * typeless block IO has bizarre byte order 
		 * (bytes within a floating value are in little endian order
		 * while at the same time the words within a floating 
		 * point value are in big endian order)
		 */
		uint32_t *plw = (uint32_t *) &val;
		assert (sizeof(val.real==sizeof(*plw)));
		*(pb++) = (uint8_t) (*plw>>(2*NBBY));
		*(pb++) = (uint8_t) (*plw>>(3*NBBY));
		*(pb++) = (uint8_t) (*plw>>(0*NBBY));
		*(pb++) = (uint8_t) (*plw>>(1*NBBY));
		break;
	}
	default:
		break;
	}

	*pByteCount = pb - pReq->buf;
}
/*
 * typed bit write req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void readModifyWriteReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pb;
	abDf1Value val;
	unsigned andMask, orMask;

	/*
	 * fetch the current value to be written
	 */
	pTrans->pElemIO->dev.devFunc->pCurrentWriteValue (&pTrans->pElemIO->dev, &val);

	pReq->hdr.cmd = df1CmdGeneral;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->typedBitWrite.fnc = df1FncWriteBit;

	/*
	 * PLC5 system address is the first part 
	 * of the data
	 */
	pb = pushPLC5Addr (pReq->typedBitWrite.data.bytes, 
			pTrans->pIO->pFile->fileNo, pTrans->pElemIO->elemNo,
			pTrans->pElemIO->subElemNo);

	/*
	 * followed by the and mask (one word)
	 */
	andMask = ~val.bitString.mask;
	*(pb++) = (uint8_t) andMask;
	*(pb++) = (uint8_t) (andMask >> NBBY);

	/*
	 * followed by the or mask (one word)
	 */
	orMask = val.bitString.value & val.bitString.mask;
	*(pb++) = (uint8_t) orMask;
	*(pb++) = (uint8_t) (orMask >> NBBY);

	*pByteCount = pb - pReq->buf;
}

/*
 * pushPLC5TypeDataParam()
 */
LOCAL uint8_t *pushPLC5TypeDataParam (uint8_t *pb, unsigned dataType)
{
	uint8_t *pTypeByte = pb++;
	unsigned dataSize;

	/*
	 * insert the data type id
	 */
	if (dataType<=tdfMaxEmbedded) {
		*pTypeByte = (dataType<<tdfFieldWidth);
	}
	else {
		*pTypeByte = tdfOneByteFollows<<tdfFieldWidth;
		assert(dataType<=UCHAR_MAX);
		*(pb++) = dataType;
	}

	/*
	 * insert the data size
	 */
	dataSize = abDataSize(dataType);
	if (dataSize<=tdfMaxEmbedded) {
		*pTypeByte |= dataSize;
	}
	else {
		*pTypeByte |= tdfOneByteFollows;
		assert(dataSize<=UCHAR_MAX);
		*(pb++) = dataSize;
	}

	return pb;
}

/*
 * pushPLC5Addr()
 *
 * NOTE: AB appear to require that at least one 
 * address level is supplied here.
 */
LOCAL uint8_t *pushPLC5Addr (uint8_t *pb, unsigned fileNo, 
					unsigned elemNo, unsigned subElemNo)
{
	uint8_t *pFirstB = pb;

	/* 
	 * flag byte initialized to zero
	 * indicating defaults for all fields
	 */
	*(pb++) = '\0'; 

	/*
	 * do we have a non-default file number?
	 */
	if (fileNo!=lbaDefaultFileNo) {
		*pFirstB |= lbaMaskFile;
		pb = pushPLC5AddrLevel(pb, fileNo);
	}

	/*
	 * do we have a non-default element number
	 */
	if (elemNo!=lbaDefaultElemNo) {
		*pFirstB |= lbaMaskElement;
		pb = pushPLC5AddrLevel(pb, elemNo);
	}

	/*
	 * do we have a non-default sub-element number
	 *
	 * NOTE: AB requires that at least one 
	 * address level is supplied here even
	 * if it is the default.
	 */
	if (subElemNo!=lbaDefaultSubElemNo || pb==(pFirstB+1u)) {
		*pFirstB |= lbaMaskSubElem;
		pb = pushPLC5AddrLevel(pb, subElemNo);
	}

	return pb;
}

/*
 * pushPLC5AddressLevel()
 */
LOCAL uint8_t *pushPLC5AddrLevel(uint8_t *pb, unsigned addr)
{
	if (addr>=lbaTwoByteAddr) {
		*(pb++) = lbaTwoByteAddr;
		*(pb++) = addr;
		*(pb++) = addr>>NBBY;
	}
	else {
		*(pb++) = addr;
	}
	return pb;
}

/*
 * fetchPLC5Addr()
 * When entering this routine *ppb contains a pointer
 * to the address string. When leaving this routine
 * *ppb contains a pointer to what is after the address 
 * string if a valid address was located.
 */
LOCAL unsigned long fetchPLC5Addr (devAbDf1ParseAddressFunc *pParseAddress, 
				const uint8_t **ppb, unsigned *pFileNo, unsigned *pElemNo, unsigned *pSubElemNo)
{
	const uint8_t *pb = *ppb;
	uint8_t flag = *(pb++);

	/*
	 * check for ascii logical address
	 */
	if (flag==0) {
		int fileType, dataType, fileNo, elemNo, subElemNo,
				bitNo, elementSize, structured;
		unsigned long status;

		if (*pb!='$') {
			return S_df1_LoAddrLvl;
		}

		/*
		 * this would happen if no IO is registered
		 * from device support
		 */
		if (!pParseAddress) {
			return S_df1_BadAddr;
		}

		/*
		 * look for a PLC5 style address
		 */
		status = (unsigned long) (*pParseAddress) (
						(char *) (pb+1), &fileType, &dataType, &fileNo, 
						&elemNo, &subElemNo, &bitNo, 
						&elementSize, &structured);
		if (status) {
			return status;
		}
		/*
		 * bit level addresing not allowed in a DF1
		 * request message
		 */
		if (dataType == df1DTBit || dataType == df1DTBitStr) {
			return S_drvAbDf1_badBitNumber;
		}			
		if (fileNo>0xffff || fileNo<0) {
			return S_drvAbDf1_badFile;
		}
		if (elemNo>0xffff || elemNo<0) {
			return S_drvAbDf1_badElement;
		}
		if (subElemNo>0xffff || subElemNo<0) {
			return S_drvAbDf1_badSubelement;
		}
		*pFileNo = (unsigned) fileNo;
		*pElemNo = (unsigned) elemNo;
		*pSubElemNo = (unsigned) subElemNo;
		/*
		 * logical ascii address is terminated by a null
		 * character
		 */
		*ppb = pb + strlen((char *)pb) + 1;
		return S_drvAbDf1_OK;
	}

	/*
	 * check for too many address levels
	 * (in particular if an area is specified then reject address)
	 */
	if (flag & ~(lbaMaskFile|lbaMaskElement|lbaMaskSubElem)) {
		return S_df1_HiAddrLvl;
	}

	/*
	 * fetch the file number
	 */
	if (flag&lbaMaskFile) {
		pb = fetchPLC5AddrLevel (pb, pFileNo);
	}
	else {
		*pFileNo = lbaDefaultFileNo;
	}

	/*
	 * fetch the element number
	 */
	if (flag&lbaMaskElement) {
		pb = fetchPLC5AddrLevel (pb, pElemNo);
	}
	else {
		*pElemNo = lbaDefaultElemNo;
	}

	/*
	 * fetch the sub-element number
	 */
	if (flag&lbaMaskSubElem) {
		pb = fetchPLC5AddrLevel (pb, pSubElemNo);
	}
	else {
		*pSubElemNo = lbaDefaultSubElemNo;
	}

	*ppb = pb;
	return S_drvAbDf1_OK;
}

/*
 * fetchPLC5AddressLevel()
 */
LOCAL const uint8_t *fetchPLC5AddrLevel (const uint8_t *pb, unsigned *pAddr)
{
	if (*pb==lbaTwoByteAddr) {
		pb++;
		*pAddr = *(pb++);
		*pAddr |= ((unsigned)(*(pb++)))<<NBBY;
	}
	else {
		*pAddr = *(pb++);
	}
	return pb;
}

/*
 * read req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void blockReadReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	unsigned addr;
	unsigned size;

	/*
	 * block reads always read words
	 */
	addr = pTrans->pIO->elemNo*sizeof(df1Word);
	size = pTrans->pIO->elemCount*sizeof(df1Word);

	pReq->hdr.cmd = df1CmdBlockRead;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->blockRead.lsAddr = addr;
	pReq->blockRead.msAddr = addr>>NBBY;
	pReq->blockRead.byteCount = size;

	/*
	 * unable to use structure size due to
	 * 1 byte pad
	 */
	*pByteCount = sizeof(pReq->hdr)
		+ sizeof(pReq->blockRead.lsAddr)
		+ sizeof(pReq->blockRead.msAddr)
		+ sizeof(pReq->blockRead.byteCount);
}

/*
 * typed block read req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void typedBlockReadReqProc(absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pb;

	pReq->hdr.cmd = df1CmdGeneral;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->typedRead.fnc = df1FncTypedRead;
	pReq->typedRead.lsPO = 0u;
	pReq->typedRead.msPO = 0u;
	pReq->typedRead.lsTT = pTrans->pIO->elemCount; 
	pReq->typedRead.msTT = pTrans->pIO->elemCount>>NBBY;

	/*
	 * PLC5 system address is the first part 
	 * of the data
	 * 
	 * sub element is always zero here since this code
	 * always reads blocks of elements (structured
	 * or otherwise)
	 */
	pb = pushPLC5Addr (pReq->typedWrite.data.bytes, 
			pTrans->pIO->pFile->fileNo, pTrans->pIO->elemNo, 0u);

	/*
	 * followed by the size
	 */
	*(pb++) = pTrans->pIO->elemCount;
	*(pb++) = pTrans->pIO->elemCount>>NBBY;

	*pByteCount = pb - pReq->buf;
}

/*
 * file block read req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void fileBlockReadReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	uint8_t *pb;
	unsigned wordSize;

	wordSize = abDataSize(pTrans->pIO->pFile->dataType)/sizeof(df1Word);
	wordSize *= pTrans->pIO->elemCount;

	pReq->hdr.cmd = df1CmdGeneral;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->fileBlockRead.fnc = df1FncWordRangeRead;
	pReq->fileBlockRead.lsPO = 0u;
	pReq->fileBlockRead.msPO = 0u;
	pReq->fileBlockRead.lsTT = wordSize; 
	pReq->fileBlockRead.msTT = wordSize>>NBBY;

	/*
	 * PLC5 system address is the first part 
	 * of the data
	 * 
	 * sub element is always zero here since this code
	 * always reads blocks of elements (structured
	 * or otherwise)
	 */
	pb = pushPLC5Addr (pReq->typedWrite.data.bytes, 
			pTrans->pIO->pFile->fileNo, pTrans->pIO->elemNo, 0u);

	/*
	 * followed by the size
	 */
	*(pb++) = wordSize*sizeof(df1Word);

	*pByteCount = pb - pReq->buf;
}

/*
 * loop back req specific processing
 * (LOCK needs to be applied here)
 */
LOCAL void loopBackReqProc (absTransaction *pTrans, abDf1ReqProto *pReq,
	unsigned *pByteCount)
{
	pReq->hdr.cmd = df1CmdDiagnostic;
	pReq->hdr.dst = pTrans->pIO->pFile->pPLC->nodeNo;
	pReq->general.fnc = df1FncEcho;

	strcpy ((char *)pReq->general.data.bytes, "echo");

	*pByteCount = sizeof(pReq->general)-sizeof(pReq->general.data)
		+ strlen ((char *)pReq->general.data.bytes) + 1; 
}

/*
 * drvAbDf1Read ()
 *
 * read incoming frames & time out stale requests
 *
 * care is taken to not hold the mutex when callbacks
 * into the device support are called (indirectly
 * calling EPICS record support)
 */
LOCAL void drvAbDf1Read (drvAbDf1Parm *pDev)
{
	int status;
	drvSerialResponse resp;
	absTransaction *pTrans;
	absTransaction *pNext;
	ELLLIST tmpList;

	/*
	 * if a client sends messages as fast or faster than this 
	 * task can process them then periodically drop out
	 * and take care of other duties
	 */
	while (TRUE) {
		status = drvSerialNextResponse (pDev->id, &resp);
		if (status) {
			break;
		}

		drvAbDf1ProcessResp (pDev, &resp);
	}

	ellInit(&tmpList);

	/*
	 * check for timed out requests
	 */
	status = semTake (pDev->mutexSem, MUTEX_TMO);
	if (status!=OK) {
		return;
	}

	/*
	 * check requests pending response for time out
	 */
	for (pTrans = (absTransaction *) ellFirst(&pDev->transResList);
			pTrans; pTrans = pNext) {

		pNext = (absTransaction *) ellNext (&pTrans->node);

		if (drvAbDf1TransTMO (pTrans->ticksAtReq)) {
			drvAbDf1MoveTrans (pTrans, &tmpList);
		}
	}

	/*
	 * check transactions pending transmission to see if
	 * they will be accepted by drvSerial
	 *
	 * note that requests waiting too be sent are not
	 * checked for TMO here because they will time out once they
	 * reach the top of the queue if the PLC does not 
	 * respond. The driver should just queue the requests
	 * if the PLC is scanned faster than throughput of the
	 * IO system allows.
	 */
	for (pTrans = (absTransaction *) ellFirst(&pDev->transDSList); 
				pTrans; pTrans = pNext) {

		pNext = (absTransaction *) ellNext (&pTrans->node);

		if (drvAbDf1QueueRequest (pTrans)) {
			break;
		}
		if (pTrans->pTransList==&pDev->transDSList) {
			break;
		}
	}

	/*
	 * !! we must own the mutex when accessing tmpList
	 * !! even when it is a local variable because a pointer
	 * !! the tmpList is stored in the transaction 
	 */
	while ( (pTrans = (absTransaction *) ellFirst(&tmpList)) ) {
		unsigned copyOfId;

		/*
		 * some archs require a lock around increment
		 */
		pDev->responseTimeoutCount++;
		copyOfId = pTrans->transId;

		status = semGive (pDev->mutexSem);
		assert (status==OK);

		/*
		 * special care is taken to not have the lock
		 * applied here (in order to avoid dead locks with the EPICS
		 * database)
		 */
		retireTransaction (pDev, copyOfId, S_drvAbDf1_respTMO);

		status = semTake (pDev->mutexSem, WAIT_FOREVER);
		assert (status==OK);
	}

	status = semGive (pDev->mutexSem);
	assert (status==OK);
}

/*
 * drvAbDf1TransTMO()
 *
 * returns true if a transaction has timed out
 */
LOCAL int  
drvAbDf1TransTMO (ULONG ticksAtReq)
{
	ULONG delay;
	ULONG current;

	current = tickGet();
	if (current>=ticksAtReq) {
		delay = current-ticksAtReq;
	}
	else {
		delay = ULONG_MAX-ticksAtReq;
		delay += current;
	}

	if (delay > RESPONSE_TMO) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*
 * drvAbDf1ProcessResp()
 *
 * process incoming frame
 *
 * care is taken to not hold the lock when the transaction
 * is retired (so we dont deadlock with the EPICS database)
 */
LOCAL void  
drvAbDf1ProcessResp (drvAbDf1Parm *pDev, drvSerialResponse *pResp)
{
	abDf1Proto *pProto = (abDf1Proto *) pResp->buf;
	unsigned frameLength = pResp->bufCount;
	int status;
	absTransaction *pTrans;
	unsigned id;

	/*
	 * reject messages that are not addressed to this station
	 */
	if (pProto->hdr.dst != pDev->nodeNo) {
		errPrintf(S_drvAbDf1_badNodeNumber, __FILE__, __LINE__,
			" (received DF1 message addressed to another node=%d?) ", pProto->hdr.dst);
		return;
	}

	/*
	 * another node sent us a command
	 * (this is not a response to our command)
	 */
	if (!(pProto->hdr.cmd&df1RespMask)) {
		(*cmdJumpTable[pProto->hdr.cmd&df1CmdMask]) (pDev, pResp);
		return;
	}

	/*
	 * verify that this transaction hasnt been canceled 
	 * or duplicated
	 */
	id = pProto->hdr.msTns;
	id <<= NBBY;
	id |= pProto->hdr.lsTns;

	status = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (status==OK);

	pTrans = (absTransaction *) 
		bucketLookupItemUnsignedId (pDev->pTransBucket, &id);

	if (pTrans) {
		status = semGive (pDev->mutexSem);
		assert (status==OK);
	}
	else {
		/*
		 * some archs require lock around increment
		 */
		pDev->dupResponseCount++;
		status = semGive (pDev->mutexSem);
		assert (status==OK);
		return;
	}

	/*
	 * If there is bad status in the header then we log the status
	 * received, and otherwise ignore the frame
	 */
	if (pProto->hdr.sts) {
		long epicsStatus;

		if (pProto->hdr.sts == df1RemoteMask) {
			epicsStatus = pProto->res.hdrstx.stx | M_df1e;
		} 
		else {
			epicsStatus = pProto->hdr.sts | M_df1;
		}

		if (drvAbDf1Debug>=1) {
			char buf[512];
			long errlkupStatus;
			errlkupStatus = errSymFind (epicsStatus, buf);
			if (errlkupStatus) {
				sprintf(buf, "Unknown error code=%lx", epicsStatus);
			}
			drvAbDf1DebugPrintf(1,"recv cmd=%x resp from node %d w bad status \"%s\"\n", 
				pProto->hdr.cmd&df1CmdMask, pProto->hdr.src, buf);
		}

		retireTransaction (pDev, id, epicsStatus);
		return;
	}

	/*
	 * when its not from who we sent it to then something
	 * is very wrong
	 *
	 * (Note that all frames that are not addressed
	 * to this node are discarded
	 * prior to performing the CRC check and min/max
	 * frame length checks)
	 */
	if (pProto->hdr.src!=pTrans->pIO->pFile->pPLC->nodeNo) {
		retireTransaction (pDev, id, S_drvAbDf1_badNodeNumber);
		return;
	}
	status = (*resJumpTable[pTrans->protoType])
				(pTrans, &pProto->res, frameLength);
	retireTransaction (pDev, id, status);
}

/*
 * setLastStatus()
 *
 * care is taken to not hold the lock
 */
LOCAL void setLastStatus (absBlockIO *pIO, long epicsStatus, int read)
{
	unsigned i;
	long oldStatus;
	long *pStatus;
	int status;

	status = semTake (pIO->pFile->pPLC->pDev->mutexSem, WAIT_FOREVER);
	assert (status==OK);

	if (!epicsStatus) {

		/*
		 * increment requires a mutex lock on some architectures
		 */
		pIO->ioCount++;
	}

	if (read) {
		pStatus = &pIO->lastReadStatus;
	}
	else {
		pStatus = &pIO->lastWriteStatus;
	}

	/*
	 * lock required to prevent possible race condition when
	 * updating status
	 */
	oldStatus = *pStatus;
	*pStatus = epicsStatus;

	status = semGive (pIO->pFile->pPLC->pDev->mutexSem);
	assert (status==OK);

	if (oldStatus!=epicsStatus) {

		/*
		 * scan records if we go in and out of alarm state
		 *
		 * no lock applied here because we dont add to or
		 * delete from this list after initialization completes
		 *
		 * if locking was used here we would need to be carefull
		 * about deadlocks with the EPICS database
		 */
		if (oldStatus==0 || epicsStatus==0) {
			for (i=0u; i<pIO->elemCount; i++) {
				drvAbDf1ElemIO *pElemIO = pIO->pElemIOTbl[i];
				while (pElemIO) {
					(*pElemIO->dev.devFunc->pNewCacheValue)(&pElemIO->dev);
					pElemIO = pElemIO->pNext;
				}
			}
		}
	}
}

/*
 * retireTransaction()
 */
LOCAL void retireTransaction (drvAbDf1Parm *pDev, unsigned id, long retireStatus)
{
	drvAbDf1Status drvStatus;
	absTransaction copyOfTrans;
	int status;

	{
		absTransaction *pTrans;

		status = semTake (pDev->mutexSem, WAIT_FOREVER);
		assert (status==OK);

		/*
		 * dont use a transaction that has already been 
		 * deleted (only use a pTrans if we have found it
		 * in the resource table and we have the lock)
		 */
		pTrans = pTrans = (absTransaction *) 
				bucketLookupItemUnsignedId (pDev->pTransBucket, &id);
		if (!pTrans) {
			status = semGive (pDev->mutexSem);
			assert (status==OK);
			return;
		}

		copyOfTrans = *pTrans;

		/*
		 * dont use a transaction that has already been 
		 * deleted (since lock is applied this will not occur)
		 */
		drvStatus = absDisposeTransaction (pTrans);
		if (drvStatus) {
			status = semGive (pDev->mutexSem);
			assert (status==OK);
			return;
		}

		/*
		 * if its a read transaction
		 */
		if (copyOfTrans.read) {
			if (copyOfTrans.pIO->queue==absQuePending) {
				ellDelete (&pDev->pendingList, &copyOfTrans.pIO->node);
				insertExpiredReadIO (copyOfTrans.pIO);
			}
		}

		status = semGive (pDev->mutexSem);
		assert (status==OK);
	}

	setLastStatus (copyOfTrans.pIO, retireStatus, copyOfTrans.read);

	/*
	 * if its a write transaction then post write completion
	 *
	 * care is taken to not have lock applied here so that
	 * we dont deadlock with the EPICS database
	 */
	if (copyOfTrans.pElemIO) {
		(*copyOfTrans.pElemIO->dev.devFunc->pWriteCompletion) 
							(&copyOfTrans.pElemIO->dev, retireStatus); 
	}
}

/*
 * protocol specific processing of the read response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long blockReadResProc (absTransaction *pTrans, 
		abDf1ResProto *pProto, unsigned byteCount)
{
	unsigned dataByteCount;
	unsigned elemSize;

	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask) != df1CmdBlockRead) {
		return S_drvAbDf1_badFrame;
	}
	
	/*
	 * if we dont get the number of bytes asked for
	 * then its a bad response (block reads always
	 * assume a 16 bit element size)
	 */
	dataByteCount = byteCount - sizeof(pProto->hdr);
	elemSize = abDataSize(pTrans->pIO->pFile->dataType);
	if (dataByteCount != pTrans->pIO->elemCount*elemSize) {
		return S_drvAbDf1_badFrame;	
	}

	/*
	 * move the bytes into the IO block
	 */
 	return drvAbDf1WriteBlockRaw (pTrans->pIO, pTrans->pIO->pFile->effectiveDataType, 
			pTrans->pIO->elemNo, 0u, pTrans->pIO->elemCount, 
			pProto->blockRead.data.bytes, FALSE);
}

/*
 * protocol specific processing of the typed block read response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long typedBlockReadResProc (absTransaction *pTrans, 
		abDf1ResProto *pProto, unsigned byteCount)
{
	unsigned dataType;
	unsigned dataSize;
	const uint8_t *pb;

	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask) != df1CmdGeneral) {
		return S_drvAbDf1_badFrame;
	}

	/* 
	 * fetch data type and size from the message
	 */
	pb = fetchPLC5TypeDataParam (&dataType, &dataSize, 
				pProto->typedRead.data.bytes);
	if (!pb) {
		drvAbDf1DebugPrintf (1, "typedBlockReadResProc: invalid type/data parameter");
		return S_df1_BadParm;
	}

	/* 
	 * verify that data type matches
	 */
	if (pTrans->pIO->pFile->effectiveDataType!=dataType) {
		return S_drvAbDf1_wrongType;
    }

	/*
	 * verify that we got the data size
	 * we asked for
	 */
	if (abDataSize(dataType)!=dataSize) {
		return S_df1_IllField;
	}

	/* 
	 * verify that frame will hold this data
	 */
	if (byteCount != (dataSize*pTrans->pIO->elemCount)+(pb-pProto->buf)) {
		return S_df1_IllField;
	}

	/*
	 * move the bytes into the IO block
	 */
	return drvAbDf1WriteBlockRaw (pTrans->pIO, dataType,
			pTrans->pIO->elemNo, 0u, 
			pTrans->pIO->elemCount, pb, TRUE);
}

/*
 * protocol specific processing of the file block read response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long fileBlockReadResProc (absTransaction *pTrans, 
		abDf1ResProto *pProto, unsigned byteCount)
{
	const uint8_t *pb = pProto->fileBlockRead.data.bytes;

	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask) != df1CmdGeneral) {
		return S_drvAbDf1_badFrame;
	}

	/* 
	 * verify that frame will hold this data
	 */
	if (byteCount != (abDataSize(pTrans->pIO->pFile->dataType)*
						pTrans->pIO->elemCount)+
						(unsigned) (pb-pProto->buf)) {
		return S_df1_IllField;
	}

	/*
	 * move the bytes into the IO block
	 */
	return drvAbDf1WriteBlockRaw (pTrans->pIO, pTrans->pIO->pFile->effectiveDataType,
			pTrans->pIO->elemNo, 0u, pTrans->pIO->elemCount, pb, FALSE);
}

/*
 * protocol specific processing of the loop back response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long loopBackResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount)
{
	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask) != df1CmdDiagnostic) {
		return S_drvAbDf1_badFrame;
	}

	if (strcmp((char *)pProto->blockRead.data.bytes,"echo")) {
		epicsPrintf("DF1 echo response failed\n");
		return S_drvAbDf1_badFrame;
	}
	else {
		return S_drvAbDf1_OK;
	}
}

/*
 * fetchPLC5TypeDataParam()
 * (returns NULL if type/element parameter is invalid
 * and otherwise a pointer to the data bytes)
 */
LOCAL const uint8_t *fetchPLC5TypeDataParam(unsigned *pDataType, 
							unsigned *pDataSize, const uint8_t *pb)
{
	uint8_t typeByte= *(pb++);
	unsigned dataSize;
	unsigned dataType;

	/*
	 * extract and verify the data type id
	 */
	dataType = (typeByte&tdfTypeMask) >> tdfFieldWidth;
	if (typeByte&tdfTypeFollowsMask) {
		/*
		 * we only handle the case where the
		 * number of bytes required to specify
		 * the data type is one or two
		 */
		if (dataType==1u) {
			dataType = *(pb++);
		}
		else if (dataType==2u) {
			dataType = pb[1];
			dataType <<= NBBY;
			dataType |= pb[0]; 
			pb += 2u;
		}
		else {
			return NULL;
		}
	}

	/*
	 * extract and verify the data size
	 */
	dataSize = typeByte&tdfSizeMask;
	if (typeByte&tdfSizeFollowsMask) {
		/*
		 * we only handle the case where the
		 * number of bytes required to specify
		 * the data size is one or two
		 */
		if (dataSize==1u) {
			dataSize = *(pb++);
		}
		else if (dataSize==2u) {
			dataSize = pb[1];
			dataSize <<= NBBY;
			dataSize |= pb[0]; 
			pb += 2u;
		}
		else {
			return NULL;
		}
	}

	if (dataType==df1DTArray) {
		return fetchPLC5TypeDataParam (pDataType, pDataSize, pb);
	}

	*pDataType = dataType;
	*pDataSize = dataSize;

	return pb;
}

/*
 * NOOP response stub
 */
LOCAL unsigned long doNothingResProc (absTransaction *pTrans,
				abDf1ResProto *pProto, unsigned byteCount)
{
	return S_drvAbDf1_OK;
}

/*
 * protocol specific processing of the write response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long blockWriteResProc (absTransaction *pTrans, 
	abDf1ResProto *pProto, unsigned byteCount)
{
	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask)!=df1CmdProtectedBlockWrite) {
		return S_drvAbDf1_badFrame;	
	}

	if (byteCount!=sizeof(pProto->blockWrite)) {
		return S_drvAbDf1_badFrame;
	}

	return drvAbDf1WriteCompletion (pTrans->pElemIO);
}

/*
 * protocol specific processing of the bit write response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long bitWriteResProc (absTransaction *pTrans, 
	abDf1ResProto *pProto, unsigned byteCount)
{
	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask)!=df1CmdProtectedBitWrite) {
		return S_drvAbDf1_badFrame;	
	}

	if (byteCount!=sizeof(pProto->bitWrite)) {
		return S_drvAbDf1_badFrame;
	}

	return drvAbDf1WriteCompletion (pTrans->pElemIO);
}

/*
 * protocol specific processing of the typed write response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long typedBlockWriteResProc (absTransaction *pTrans, 
	abDf1ResProto *pProto, unsigned byteCount)
{

	/*
	 * this will occur if the transaction id is wrong,
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask)!=df1CmdGeneral) {
		return S_drvAbDf1_badFrame;	
	}

	if (byteCount!=sizeof(pProto->typedWrite)) {
		return S_drvAbDf1_badFrame;
	}

	return drvAbDf1WriteCompletion (pTrans->pElemIO);
}

/*
 * protocol specific processing of the file block write response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long fileBlockWriteResProc (absTransaction *pTrans, 
	abDf1ResProto *pProto, unsigned byteCount)
{

	/*
	 * this will occur if the transaction id is wrong,
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask)!=df1CmdGeneral) {
		return S_drvAbDf1_badFrame;	
	}

	if (byteCount!=sizeof(pProto->fileBlockWrite)) {
		return S_drvAbDf1_badFrame;
	}

	return drvAbDf1WriteCompletion (pTrans->pElemIO);
}
/*
 * protocol specific processing of the bit write response messages
 * (lock must be applied when calling this function)
 */
LOCAL unsigned long readModifyWriteResProc (absTransaction *pTrans, 
	abDf1ResProto *pProto, unsigned byteCount)
{
	/*
	 * this will occur if the transaction id is wrong
	 * but happens to match an existing transaction id
	 */
	if ((pProto->hdr.cmd&df1CmdMask)!=df1CmdGeneral) {
		return S_drvAbDf1_badFrame;	
	}

	if (byteCount!=sizeof(pProto->typedBitWrite)) {
		return S_drvAbDf1_badFrame;
	}

	return drvAbDf1WriteCompletion (pTrans->pElemIO);
}

/*
 * drvAbDf1ParseInput()
 */
LOCAL int
drvAbDf1ParseInput (FILE *fp, drvSerialResponse *pResp, void *pPrivate)
{
	drvAbDf1Parm *pDev = (drvAbDf1Parm *) pPrivate;
	drvAbDf1Status status;

	while (TRUE) {
		status = (*pDev->pReadCharFunc) (fp, pResp, pDev);
		if (status) {
			return status;
		}
	}
}

/*
 * drvAbDf1SetMsgMode()
 */
LOCAL void drvAbDf1SetMsgMode (drvAbDf1Parm *pDev, drvSerialResponse *pResp)
{
	/*
	 * reset to looking for STX mode
	 */
	pDev->pReadCharFunc = drvAbDf1ReadNextMsgChar;
	pResp->bufCount = 0;

	pDev->inHandlerList[df1dlSTX] = drvAbDf1UnexpectedSTX;
	pDev->inHandlerList[df1dlETX] = drvAbDf1ExpectedETX;
	pDev->inHandlerList[df1dlDLE] = drvAbDf1ExpectedDLE;
}

/*
 * drvAbDf1SetNoMsgMode()
 */
LOCAL void drvAbDf1SetNoMsgMode (drvAbDf1Parm *pDev, drvSerialResponse *pResp)
{
	/*
	 * initialize the receiver to discard characters 
	 * until an STX is seen
	 */
	pDev->pReadCharFunc = drvAbDf1DiscardInput;
	pDev->inHandlerList[df1dlSTX] = drvAbDf1ExpectedSTX;
	pDev->inHandlerList[df1dlDLE] = drvAbDf1UnexpectedDLE;
	pDev->inHandlerList[df1dlETX] = drvAbDf1UnexpectedETX;

	if (pResp) {
		pResp->bufCount = 0;
	}
}

/*
 * drvAbDf1DiscardInput()
 */
LOCAL int drvAbDf1DiscardInput (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	int	nc;
	int status;

	nc = getc(fp);
	if (nc < 0) {
		return EOF;
	}

	if (nc==df1dlDLE) {

		/*
		 * next character is a control character
		 */
		nc = getc (fp);
		if (nc < 0) {
			return EOF;
		}

		if (nc<(int)NELEMENTS(pDev->inHandlerList)) {
			status = (*pDev->inHandlerList[nc]) (fp, pResp, pDev);
		}
		else {
			status = drvAbDf1UnexpectedProto (fp, pResp, pDev);
		}
		return status;
	}

	return 0;
}

/*
 * drvAbDf1ReadNextMsgChar()
 */
LOCAL int drvAbDf1ReadNextMsgChar (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	int	nc;
	int status;

	nc = getc(fp);
	if (nc < 0) {
		return EOF;
	}

	if (nc==df1dlDLE) {
		/*
		 * next character is a control character
		 */
		nc = getc (fp);
		if (nc < 0) {
			return EOF;
		}

		/*
		 * a DLE followed by a DLE is inserted into
		 * the message as a single DLE character
		 */
		if (nc<(int)NELEMENTS(pDev->inHandlerList)) {
			status = (*pDev->inHandlerList[nc]) (fp, pResp, pDev);
		}
		else {
			status = drvAbDf1UnexpectedProto (fp, pResp, pDev);
		}
		return status;
	}

	if (pResp->bufCount>=NELEMENTS(pResp->buf)) {
		drvAbDf1DebugPrintf (1,"<-Msg too large (discarded)\n");
		/*
		 * too many characters in the message 
		 *
		 * reset to looking for STX mode
		 */
		pDev->lastAckSent = df1dlNAK;
		drvAbDf1SetNoMsgMode (pDev, pResp);
	}
	else {
		pResp->buf[pResp->bufCount++] = (char) nc;
	}

	return 0;
}

/*
 * drvAbDf1UnexpectedProto()
 */
LOCAL int drvAbDf1UnexpectedProto (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	pDev->badControlRecvCount++;
	pDev->lastAckSent = df1dlNAK;

	/*
	 * reset to looking for STX mode
	 */
	drvAbDf1SetNoMsgMode (pDev, pResp);

	drvAbDf1DebugPrintf (1,"<-Bad control char\n");

	return 0;
}

/*
 * drvAbDf1ExpectedDLE()
 */
LOCAL int drvAbDf1ExpectedDLE (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	if (pResp->bufCount>=NELEMENTS(pResp->buf)) {
		/*
		 * too many characters in the message 
		 *
		 * return code intentionally ignored
		 * (if the queue is full we will wait
		 * for the other side to time out)
		 *
		 * reset to looking for STX mode
		 */
		pDev->pReadCharFunc = drvAbDf1DiscardInput;
		pDev->lastAckSent = df1dlNAK;
		pResp->bufCount = 0;
	}
	else {
		pResp->buf[pResp->bufCount++] = (char) df1dlDLE;
	}
	return 0;
}

/*
 * drvAbDf1UnexpectedDLE()
 */
LOCAL int drvAbDf1UnexpectedDLE (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	drvAbDf1DebugPrintf (1,"<-DLE DLE between messages ?\n");

	pDev->badControlRecvCount++;
	pDev->lastAckSent = df1dlNAK;
	return 0;
}

/*
 * drvAbDf1UnexpectedETX()
 */
LOCAL int drvAbDf1UnexpectedETX (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	drvAbDf1DebugPrintf (1,"<-Unexpected ETX ?\n");

	drvAbDf1QueueRespCode (pDev, df1dlNAK);
	pDev->badControlRecvCount++;
	return 0;
}

/*
 * drvAbDf1ExpectedETX()
 */
LOCAL int drvAbDf1ExpectedETX (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	drvSerialRequest *pReq;
	abDf1ProtoHdr *pHdr;
	unsigned checkSum;
	int c;

	drvAbDf1DebugPrintf (3,"<-ETX\n");

	/*
	 * reject messages less than the minimum size
	 */
	if (pResp->bufCount<df1dlMinFrameLen) {
		/*
		 * error code intentionally ignored
		 * (if the queue is full we will wait
		 * for the other side to time out)
		 */
		drvAbDf1QueueRespCode (pDev, df1dlNAK);
		pDev->damagedFrameCount++;
		drvAbDf1SetNoMsgMode (pDev, pResp);
		drvAbDf1DebugPrintf (1,"<-Msg was too small?\n");
		return 0;
	}

	/*
	 * reject messages greater than the maximum size
	 */
	if (pResp->bufCount>df1dlMaxFrameLen) {
		/*
		 * error code intentionally ignored
		 * (if the queue is full we will wait
		 * for the other side to time out)
		 */
		drvAbDf1QueueRespCode (pDev, df1dlNAK);
		pDev->damagedFrameCount++;
		drvAbDf1SetNoMsgMode (pDev, pResp);
		drvAbDf1DebugPrintf (1,"<-Msg was too large?\n");
		return 0;
	}
 
#	ifdef VALIDATE_FRAME_WITH_BCC
		c = getc(fp);
		if (c==EOF) {
			return c;	
		}
		checkSum = c;
#	else
		c = getc(fp);
		if (c==EOF) {
			return c;	
		}
		checkSum = c;
		c = getc(fp);
		if (c==EOF) {
			return c;	
		}
		checkSum |= c << NBBY;
#	endif

	if (checkSum != computeCheckSum(pResp->buf, pResp->bufCount)) {
		drvAbDf1QueueRespCode (pDev, df1dlNAK);
		pDev->damagedFrameCount++;
		drvAbDf1SetNoMsgMode (pDev, pResp);
		drvAbDf1DebugPrintf (1,"<-damaged frame?\n");
		return 0;
	}

	if (drvSerialInputQueueIsFull(pDev->id)) {
		/*
		 * no queue spoace so we just discard incoming frames until
		 * the input queue unblocks (this matches exactly brain
		 * dead DF1 behavior as specified by AB)
		 */
		drvAbDf1QueueRespCode (pDev, df1dlNAK);
		drvAbDf1SetNoMsgMode (pDev, pResp);
		drvAbDf1DebugPrintf (1,"<-no queue space for incoming frame?\n");
		return 0;
	}

	/*
	 * AP DF1 protocol spec specifies that if this is a request
	 * and we cant guarantee a response then we send a NAK
	 */
	pHdr = (abDf1ProtoHdr *) pResp->buf;
	if (!(pHdr->cmd&df1RespMask)) {
		pReq = drvSerialCreateReservedRequest (pDev->id, dspHigh);

		if (!pReq) {
			drvAbDf1QueueRespCode(pDev, df1dlNAK);
			drvAbDf1SetNoMsgMode (pDev, pResp);
			drvAbDf1DebugPrintf (1,"<-no queue space for response to incoming frame?\n");
			return 0;
		}

		pResp->pAppPrivate = pReq;
	}
	else {
		pResp->pAppPrivate = NULL;
	}


	/*
	 * also notifies the send task that a message was received and 
	 * therefore this might be a good time to try to retry
	 * the send (when we have received a NAK most likely 
	 * because the recv queue on the destination node is
	 * full)
	 */
	drvAbDf1QueueRespCode (pDev, df1dlACK);

	/*
	 * notify the scan task that a message is in the input queue
	 */
	semFlush (pDev->scanSem);

	/*
	 * notifification is also sent to the write task that 
	 * a frame has been received
	 * and that in a NAK situation resulting from over run
	 * it is a good time to try sending the frame again
	 * indirectly via the ACK post above (via the ack sem)
	 */
	pDev->totFramesRecv++;

	drvAbDf1SetNoMsgMode (pDev, NULL);

	return pResp->bufCount;
}


/*
 * drvAbDf1UnexpectedSTX()
 */
LOCAL int drvAbDf1UnexpectedSTX (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{
	pDev->badControlRecvCount++;
	/*
	 * if they sent an STX when an ETX is expected, then 
	 * they must have misinterpreted a damaged character stream
	 * as a NAK/ACK lostreceived last NAK/ACK 
	 * we sent and we are experiencing a lost ETX
	 * if they send an ENQ
	 */
	pDev->lastAckSent = df1dlNAK;
	drvAbDf1DebugPrintf (1,"<-DLE STX prior to DLE ETX ? - reseting\n");
	pResp->bufCount = 0;

	return 0;
}

/*
 * drvAbDf1ExpectedSTX()
 */
LOCAL int drvAbDf1ExpectedSTX (FILE *fp, drvSerialResponse *pResp, 
		drvAbDf1Parm *pDev)
{

	drvAbDf1DebugPrintf (3,"<-STX\n");
	/*
	 * if they sent an STX, then they received last NAK/ACK 
	 * we sent and we are experiencing a lost ETX
	 * if they send an ENQ
	 */
	pDev->lastAckSent = df1dlNAK;
	drvAbDf1SetMsgMode (pDev, pResp);

	return 0;
}


#ifdef VALIDATE_FRAME_WITH_BCC
/*
 * computeCheckSum() BCC version
 *
 * An 8 bit value is attached to the end of the frame
 */
LOCAL INLINE unsigned computeCheckSum(const uint8_t *pFrame, unsigned byteCount)
{
	uint8_t *pC;
	int bcc= 0;
 
	for (pC = pFrame; pC<&pFrame[byteCount]; pC++) {
		bcc += *pC;
	}

	bcc = -bcc;
	bcc = bcc & mkmask(NBBY);
	return bcc;
}
#else /*VALIDATE_FRAME_WITH_BCC*/
/* 
 * computeByteCRC()
 */
LOCAL INLINE unsigned computeByteCRC(unsigned crc, unsigned c)
{
	unsigned i;

	crc ^= c;
	for (i=0; i<NBBY; i++) {
		if (crc&1) {
			crc >>= 1;
			/*
			 * CRC magic for polynomial
			 * x**16+x**15+x**2+x**0
			 *
			 * see sec 6.3.3.2 in
			 * Users's Manual for
			 * PLC-2-Family/RS-232-C
			 * Interface Module
			 * AB Cat No. 1771-KG)
			 */
			crc ^= 0xa001;
		}
		else {
			crc >>= 1;
		}
	}

	return crc;
}
/*
 * computeCheckSum() CRC version
 *
 * DLE DLE is not counted twice (not expanded here either)
 *
 * A 16 bit value is attached to the end of the frame
 */
LOCAL INLINE unsigned computeCheckSum(const uint8_t *pFrame, unsigned byteCount)
{
	unsigned	crc = 0;
	const uint8_t	*pC;

	for (pC = pFrame; pC<&pFrame[byteCount]; pC++) {
		crc = computeByteCRC(crc, *pC);
	}
	crc = computeByteCRC(crc, df1dlETX);
	return crc;
}
#endif /*VALIDATE_FRAME_WITH_BCC*/
 
/*
 * drvAbDf1Ack()
 */
LOCAL int drvAbDf1Ack(FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev)
{
	drvAbDf1Status status;

	drvAbDf1DebugPrintf(3,"<-ACK\n");
	pDev->ackRecv = TRUE;
	status = semFlush(pDev->ackSem);
	assert(status==OK);

	return 0;
}
 
/*
 * drvAbDf1NegAck()
 */
LOCAL int drvAbDf1NegAck (FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev)
{
	drvAbDf1Status status;

	drvAbDf1DebugPrintf(3,"<-NAK\n");
	pDev->nakRecvCount++;
	pDev->negAckRecv = TRUE;
	status = semFlush (pDev->ackSem);
	assert (status==OK);

	return 0;
}

/*
 * drvAbDf1Enq()
 */
LOCAL int drvAbDf1Enq(FILE *fp, drvSerialResponse *pResp, drvAbDf1Parm *pDev)
{
	drvAbDf1DebugPrintf (2,"<-ENQ\n");

	/*
	 * error code intentionally ignored
	 * (if the queue is full we will send when the ENQ comes)
	 */
	drvAbDf1QueueRespCode (pDev, pDev->lastAckSent);

	return 0;
}

/*
 * drvAbDf1QueueRespCode()
 * (offload ACK or NAK to the send task)
 */
LOCAL int drvAbDf1QueueRespCode (drvAbDf1Parm *pDev, int code)
{
	drvSerialRequest req;
	int status;

	pDev->lastAckSent = code;
	pDev->nextAckSent = code;

	/*
	 * wake the send task if it is waiting for an
	 * ACK (or for max outstanding packets to decrease)
	 */
	status = semFlush (pDev->ackSem);
	assert (status==OK);

	if (code==df1dlNAK) {
		pDev->nakSendCount++;
	}

	req.bufCount = 0;
	req.pCB = drvAbDf1SendRespCodeFromRequest;
	req.pAppPrivate = pDev;

	/*
	 * error code intentionally ignored
	 * (if the queue is full we will send when the ENQ comes)
	 */
	status = drvSerialSendRequest (pDev->id, dspHigh, &req);
	if (status) {
		if (status !=S_drvSerial_queueFull && status!=S_dev_noMemory) {
			errMessage(status, "drvAbDf1QueueRespCode()");
			return status;
		}
	}

	return S_drvAbDf1_OK;
}
/*
 * drvAbDf1SendRespCodeFromRequest()
 * (send ACK or NAK from a request item read by the send task)
 */
LOCAL int drvAbDf1SendRespCodeFromRequest(FILE *pf, drvSerialRequest *pReq)
{
	drvAbDf1Parm *pDev = pReq->pAppPrivate;

	return drvAbDf1SendRespCode(pf, pDev);
}
/*
 * (send ACK or NAK from the send task)
 */
LOCAL int drvAbDf1SendRespCode(FILE *pf, drvAbDf1Parm *pDev)
{
	int status;

	/*
	 * dont send twice if we sent it while waiting
	 * for a request to be acked
	 */
	if (pDev->nextAckSent==df1dlNONE) {
		return 0;
	}

	status = putc (df1dlDLE, pf);
	if (status==EOF) {
		return status;
	}
	status = putc (pDev->nextAckSent, pf);
	if (status==EOF) {
		return status;
	}

	/*
	 * force the request out to the serial port 
	 */
	status = fflush (pf);
	if (status == EOF) {
		return status;
	}

	if (pDev->nextAckSent==df1dlACK) {
		drvAbDf1DebugPrintf(3,"ACK->\n");
	}
	else if (pDev->nextAckSent==df1dlNAK) {
		drvAbDf1DebugPrintf(3,"NAK->\n");
	}
	else {
		assert(0);
	}

	/*
	 * disable sending this again in case we sent it
	 * early (while waiting for a request to be acked)
	 */
	pDev->nextAckSent = df1dlNONE;

	return 0;
}

/*
 * drvAbDf1QueueRequest()
 * (place request on the queue)
 * 
 * lock must be applied when calling this routine
 */
LOCAL long drvAbDf1QueueRequest (absTransaction *pTrans)
{
	long status;
	int vxStatus;
	unsigned reqSize;
	drvSerialRequest drvSReq;
	abDf1ReqProto *pABReq = (abDf1ReqProto *) drvSReq.buf;
	drvSerialPriority pri;
	ELLLIST *pList;

	(*reqJumpTable[pTrans->protoType])(pTrans, pABReq, &reqSize);

	/*
	 * check for application programmer error
	 */
	assert (reqSize<=df1dlMaxFrameLen);
	assert (reqSize>=df1dlMinFrameLen);
	assert (reqSize<=sizeof(drvSReq.buf));

	/*
	 * common protocol fields
	 */
	pABReq->hdr.src = pTrans->pIO->pFile->pPLC->pDev->nodeNo;
	pABReq->hdr.sts = 0;
	pABReq->hdr.lsTns = pTrans->transId;
	pABReq->hdr.msTns = pTrans->transId>>NBBY;

	drvSReq.bufCount = reqSize;
	drvSReq.pCB = drvAbDf1SendRequest;
	drvSReq.pAppPrivate = pTrans;

	/*
	 * medium priority if its a write
	 * (high priority is for command responses and ACKs)
	 */
	pri = pTrans->pElemIO?dspMed:dspLow;

	vxStatus = semTake (pTrans->pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	status = drvSerialSendRequest (pTrans->pIO->pFile->pPLC->pDev->id, pri, &drvSReq);
	if (status==S_drvAbDf1_OK) {
		pList = &pTrans->pIO->pFile->pPLC->pDev->transACKList;
	}
	else {
		pList = &pTrans->pIO->pFile->pPLC->pDev->transDSList;
	}

	drvAbDf1MoveTrans (pTrans, pList);

	vxStatus = semGive (pTrans->pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	return S_drvAbDf1_OK;
}


/*
 * drvAbDf1SendRequest()
 * xmit request to the device and wait for an ACK
 */
LOCAL int drvAbDf1SendRequest (FILE *fp, drvSerialRequest *pRequest)
{
	absTransaction *pTrans = (absTransaction *) pRequest->pAppPrivate;
	drvAbDf1Parm *pDev = pTrans->pIO->pFile->pPLC->pDev;
	drvAbDf1Status status;
	ULONG current;
	ULONG delay;
	unsigned copyOfId;
	int vxStatus;

	/*
	 * dont allow too many requests to be outstanding at once
	 * (and potentially overrun the input queue)
	 *
	 * if the outstanding requests take too long to complete then 
	 * they will time out and this will unblock
	 *
	 * must NOT hold lock while waiting
	 */
	while (ellCount(&pDev->transResList)>=(int)drvAbDf1MaxOutstandingRequest) {
		drvAbDf1DebugPrintf(2,"Wait for max outstanding requests to decrease\n");
		semTake (pDev->ackSem, ENQ_TMO);
		if (pDev->nextAckSent!=df1dlNONE) {
			drvAbDf1SendRespCode(fp, pDev);
		}
	}


	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (vxStatus==OK);

	current = tickGet();

	/*
	 * update the delay to send counters
	 */
	if (current>= pTrans->ticksAtReq) {
		delay = current - pTrans->ticksAtReq;
	}
	else {
		delay = (ULONG_MAX - pTrans->ticksAtReq) + current;
	}

	if (pTrans->read) {
		pDev->smoothedDelayToReadSend = 
			(delay+pDev->smoothedDelayToReadSend*3)/4U;
		pDev->maxDelayToReadSend = 
			max(pDev->maxDelayToReadSend, delay);
	}
	else {
		pDev->smoothedDelayToWriteSend = 
			(delay+pDev->smoothedDelayToWriteSend*3)/4U;
		pDev->maxDelayToWriteSend = 
			max(pDev->maxDelayToWriteSend, delay);
	}

	/*
	 * record the time that we started sending a request to
	 * the PLC (so we can measure the response time)
	 */
	pTrans->ticksAtReq = current;

	/*
	 * any time after the transaction is sent it may be deleted
	 * (we dont have the mutex here)
	 */
	copyOfId = pTrans->transId;

	/*
	 * o inc requires lock
	 * o continued existence of transaction requires lock
	 */
	pTrans->pIO->pFile->pPLC->ioOutstandingCount++;

	/*
	 * we dont test the queue address here because
	 * the transaction may be in a temporary list
	 * when it is deleted
	 */
	pTrans->respPending = TRUE;

	/*
	 * list access requires lock
	 */
	drvAbDf1MoveTrans (pTrans, &pDev->transResList);

	/*
	 * once the transaction is in the resp list and the
	 * lock is removed it can be timed out (and deleted
	 * so pTrans is no longer safe)
	 */
	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);


	/*
	 * must NOT hold lock while waiting
	 */
	status = drvAbDf1SendFrame (fp, pRequest, pDev, pTrans->pIO->pFile->pPLC);

	/*
	 * safe to access the transaction here because
	 * it will not be timed out (and potentially deleted)
	 * until it is on the transaction response list
	 */
	if (status) {

		/*
		 * we never delete the transaction with the lock on
		 * (this avoids dead locks with the EPICS database)
		 */
		retireTransaction (pDev, copyOfId, status);
	}

	/*
	 * if there are transactions waiting to go into drvSerial
	 * then we wake up the AB DF1 task so that it can take
	 * care of this
	 */
	if (ellCount(&pDev->transDSList)>0) {
		vxStatus = semFlush (pDev->scanSem);
		assert (vxStatus==OK);
	}

	if (status==S_drvAbDf1_linkDown) {
		return EOF;
	}
	else {
		return 0; /* success */
	}
}

/*
 * drvAbDf1SendReply()
 * xmit reply to the device and wait for an ACK
 *
 */
LOCAL int drvAbDf1SendReply (FILE *fp, drvSerialRequest *pRequest)
{
	drvAbDf1Parm *pDev = (drvAbDf1Parm *) pRequest->pAppPrivate;
	drvAbDf1Status status;
	int vxStatus;

	status = drvAbDf1SendFrame (fp, pRequest, pDev, NULL);
	if (status==S_drvAbDf1_linkDown) {
		return EOF;
	}

	/*
	 * if there are transactions waiting to go into drvSerial
	 * then we wake up the AB DF1 task so that it can take
	 * care of this
	 */
	if (ellCount (&pDev->transDSList)>0) {
		vxStatus = semFlush (pDev->scanSem);
		assert (vxStatus==OK);
	}

	return 0; /* success */
}

/*
 * drvAbDf1SendFrame()
 * xmit frame to the device and wait for an ACK
 *
 * NOTE:
 * pPLC is NULL when this is a reply to a request,
 * and is a valid pointer when this is a request
 */
LOCAL drvAbDf1Status 
drvAbDf1SendFrame (FILE *fp, const drvSerialRequest *pRequest, 
				   drvAbDf1Parm *pDev, absPLCIO *pPLC)
{
	drvAbDf1Status status;
	unsigned negAckRecvCount;
	unsigned tmoCount;
	const uint8_t *pC;
	unsigned checkSum;
	unsigned copyOfCount;
	ULONG current;
	ULONG begin;
	ULONG enqBegin;
	ULONG ticksLeft;
	ULONG delay;
	ULONG nakTMO;

	negAckRecvCount = 0u;
	tmoCount = 0u;
	pDev->ackRecv = FALSE;
	while (TRUE) {

		if (pDev->nextAckSent!=df1dlNONE) {
			drvAbDf1SendRespCode(fp, pDev);
		}

		/*
		 * DLE STX header
		 */
		pDev->negAckRecv = FALSE;
		status = putc (df1dlDLE, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}
		status = putc(df1dlSTX, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}

		/*
		 * Body (all DLE sent twice)
		 */
		for (pC = pRequest->buf; 
			pC<&pRequest->buf[pRequest->bufCount]; pC++) {
			if (*pC == df1dlDLE) {
				status = putc(*pC, fp);
				if (status == EOF) {
					return S_drvAbDf1_linkDown;
				}
			}
			status = putc(*pC, fp);
			if (status == EOF) {
				return S_drvAbDf1_linkDown;
			}
			/*
			 * embedded ACK in this message if it is a long one
			 */
			if (pDev->nextAckSent!=df1dlNONE) {
				drvAbDf1DebugPrintf(3,"sent ACK inside msg\n");
				drvAbDf1SendRespCode(fp, pDev);
			}
		}

		/*
		 * DLE ETC BCC/CRC trailer
		 */
		status = putc(df1dlDLE, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}
		status = putc(df1dlETX, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}

		checkSum = computeCheckSum(pRequest->buf, pRequest->bufCount);

#ifdef VALIDATE_FRAME_WITH_BCC
		status = putc(checkSum, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}
#else
		status = putc(checkSum, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}
		status = putc(checkSum>>NBBY, fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}
#endif
		/*
		 * force the request out to the serial port 
		 */
		status = fflush (fp);
		if (status == EOF) {
			return S_drvAbDf1_linkDown;
		}

		drvAbDf1DebugPrintf(3,"REQ->\n");

		if (pDev->nextAckSent!=df1dlNONE) {
			drvAbDf1SendRespCode(fp, pDev);
		}

		ticksLeft = ENQ_TMO;
		enqBegin = begin = tickGet();
		 
		if (pPLC) {
			copyOfCount = pPLC->ioOutstandingCount;
		}
		else {
			copyOfCount = 0u;
		}
		while (TRUE) {

			/*
			 * status ignored because we cant tell difference
			 * btw bad sem id and a time out
			 */
			semTake (pDev->ackSem, ticksLeft);

			if (pDev->ackRecv) {
				pDev->totFramesSent++;
				return S_drvAbDf1_OK; /* success */
			}

			if (pDev->negAckRecv) {
				break;
			}
			
			/*
			 * you never know when the tick cntr will wrap 
			 * around or someone will reset it
			 */
			current = tickGet();
			if (current>=enqBegin) {
				delay = current - enqBegin;
			}
			else {
				delay = ULONG_MAX - enqBegin;
				delay += current;
			}

			if (delay >= ENQ_TMO) {
				
				/*
				 * here if it timed out
				 */
				tmoCount++;
				if (tmoCount>drvAbDf1MaxXmitTMO) {
					/*
					 * here if too many time out retries
					 */
					pDev->enqTimeoutCount++;
					drvAbDf1DebugPrintf (2,
						"Gave up waiting for NAK/ACK after %u ENQ attempts\n",
						drvAbDf1MaxXmitTMO);
					return S_drvAbDf1_ackTMO; 
				}

				/*
				 * if we time out then ask the
				 * other side to re xmit the ACK
				 */
				status = putc(df1dlDLE, fp);
				if (status==EOF) {
					return S_drvAbDf1_linkDown;
				}
				status = putc(df1dlENQ, fp);
				if (status==EOF) {
					return S_drvAbDf1_linkDown;
				}
				status = fflush (fp);
				if (status == EOF) {
					return S_drvAbDf1_linkDown;
				}

				drvAbDf1DebugPrintf(2,"ENQ->\n");
				
				/*
				 * reset the time out
				 */
				enqBegin = tickGet();
				delay = 0ul;
			}
			
			ticksLeft = ENQ_TMO-delay;

			/*
			 * if we were awakened for this purpose then
			 * we need to recompute the timeout and
			 * drop back into semTake()
			 */
			if (pDev->nextAckSent!=df1dlNONE) {
				drvAbDf1SendRespCode(fp, pDev);
			}
		}

		/*
		 * here if its a neg ack (need appropriate delay
		 * and then a re xmit)
		 */
		 negAckRecvCount++;

		if (negAckRecvCount>=drvAbDf1MaxNegAck) {
			/*
			 * here if to many neg ack retries
			 */
			pDev->nakTimeoutCount++;
			drvAbDf1DebugPrintf(1,
				"Gave up after %u negative ACK retries\n",
				drvAbDf1MaxNegAck);
			return S_drvAbDf1_negAckTMO; 
		}

		/*
		 * we can receive NAKs for two reasons:
		 * 1) destination node does not have buffer space 
		 *		to store request frame (overrun)
		 * 2) request frame arrives damaged at destination 
		 *		node
		 *
		 * therefore we dont want to resend immediately if
		 * this is the 2nd NAK in a row because the cause
		 * is most likely a queue overrun and we would
		 * just get an immediate NAK and use up our NAK quota
		 * prematurely
		 *
		 * count was incremented before the ACK so we are
		 * looking at really one less than the current value
		 * of copyOfCount requests outstanding below
		 */
		if (negAckRecvCount<=1u) {
			nakTMO=0u;
		}
		else {
			nakTMO = NAK_BASE_TMO << (negAckRecvCount-2u);
			nakTMO = min (nakTMO, NAK_BASE_TMO*100ul);
		}
		
		while (TRUE) {
			current = tickGet();
			if (current>=begin) {
				delay = current - begin;
			}
			else {
				delay = ULONG_MAX - begin;
				delay += current;
			}

			if (delay>=nakTMO) {
				break;
			}

			/*
			 * status ignored because we cant tell difference
			 * btw bad sem id and a time out
			 */
			semTake (pDev->ackSem, nakTMO-delay);

			/*
			 * if we were awakened for this purpose then
			 * we need to recompute the timeout and
			 * drop back into semTake()
			 */
			if (pDev->nextAckSent!=df1dlNONE) {
				drvAbDf1SendRespCode (fp, pDev);
			}

			if (pPLC) {
				/*
				 * if we received a new frame then the queue
				 * overrun condition has most likely gone away
				 * so it is hopefully a good time to retry
				 */
				if (pPLC->ioOutstandingCount<copyOfCount) {
					break;
				}
			}
		}
	}
	/*
	 * never here
	 */
}

/*
 * drvAbDf1ReadWord()
 */
long drvAbDf1ReadWord (abDf1ElemIO *pDevElemIO, uint16_t *pVal)
{
	drvAbDf1ElemIO *pElemIO = devToDrvElemIOPtr (pDevElemIO);
	absBlockIO	*pIO = pElemIO->pBIO;
	uint16_t work;
	df1Word *pWords;
	int vxStatus;
	long status;

	if (!pIO) {
		return S_drvAbDf1_notSubscribed;
	}
	if (pIO->lastReadStatus) {
		return pIO->lastReadStatus;
	}

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DTInt, pElemIO->elemNo, 
					pElemIO->subElemNo, 1u, (void **)&pWords);
	if (status) {
		return status;
	}

	/*
	 * MUTEX used here to force ls and ms bytes
	 * to be consistent
	 */
	vxStatus = semTake (pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	work = pWords->msData;
	work <<= NBBY;
	work |= pWords->lsData;
	*pVal = work;

	/*
	 * MUTEX off
	 */
	vxStatus = semGive (pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	return S_drvAbDf1_OK;
}

/*
 * drvAbDf1ReadReal()
 */
long drvAbDf1ReadReal (abDf1ElemIO *pDevElemIO, float *pVal)
{
	drvAbDf1ElemIO *pElemIO = devToDrvElemIOPtr (pDevElemIO);
	absBlockIO	*pIO = pElemIO->pBIO;
	uint8_t *pElem;
	union {
		float fval;
		uint32_t ival;
	}work;
	int vxStatus;
	unsigned long status;

	if (!pIO) {
		return S_drvAbDf1_notSubscribed;
	}
	if (pIO->lastReadStatus) {
		return pIO->lastReadStatus;
	}

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DTFP, pElemIO->elemNo, 
						pElemIO->subElemNo, 1u, (void **) &pElem);
	if (status) {
		return status;
	}

	assert (sizeof(work.fval)==abDataSize(df1DTFP));
	assert (sizeof(work.fval)==sizeof(work.ival));

	/*
	 * MUTEX used here to force ls and ms bytes
	 * to be consistent
	 */
	vxStatus = semTake (pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	work.ival = pElem[3];
	work.ival <<= NBBY;
	work.ival |= pElem[2];
	work.ival <<= NBBY;
	work.ival |= pElem[1];
	work.ival <<= NBBY;
	work.ival |= pElem[0];
	*pVal = work.fval;

	/*
	 * MUTEX off
	 */
	vxStatus = semGive(pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	return S_drvAbDf1_OK;
}

/*
 * drvAbDf1ReadBlock() 
 * (bytes returned in here are in DF1 (little endian byte order)
 * this returns a PLC5 STX error code
 */
LOCAL unsigned long drvAbDf1ReadBlock (absBlockIO *pIO, unsigned df1DT, 
			unsigned elemNo, unsigned subElemNo, unsigned nElem, uint8_t *pBuf)
{
	int vxStatus;
	unsigned long status;
	uint8_t *pElem;

	if (!pIO) {
		return S_drvAbDf1_notSubscribed;
	}
	if (pIO->lastReadStatus) {
		return pIO->lastReadStatus;
	}

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DT, 
								elemNo, subElemNo, nElem, (void **) &pElem);
	if (status) {
		return status;
	}

	/*
	 * MUTEX used here to force entire modification
	 * to be consistent
	 */
	vxStatus = semTake(pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	/*
	 * copy out the data in little endian format
	 */
	memcpy (pBuf, pElem, nElem*abDataSize(df1DT));

	/*
	 * MUTEX off
	 */
	vxStatus = semGive(pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	return 0; /* success */
}

/*
 * drvAbDf1InitiateWrite () 
 */
long drvAbDf1InitiateWrite (abDf1ElemIO *pDevElemIO)
{
	drvAbDf1ElemIO *pElemIO = devToDrvElemIOPtr (pDevElemIO);
	absBlockIO	*pIO = pElemIO->pBIO;
	absTransaction *pTrans;
	long status;

	/*
	 * other data types cant be written
	 */
	if (pDevElemIO->dataType != df1DTFP && 
		pDevElemIO->dataType != df1DTInt &&
		pDevElemIO->dataType != df1DTBit) {
		return S_drvAbDf1_uknDataType;
	}

	/*
	 * verify that the request is valid
	 */
	if (!pIO) {
		return S_drvAbDf1_notSubscribed;
	}

	/*
	 * if its for this node then no IO is performed
	 * (we just update the cache)
	 */
	if (pIO->pFile->pPLC->nodeNo==pIO->pFile->pPLC->pDev->nodeNo) {
		return drvAbDf1WriteCompletion (pElemIO);
	}
	else {
		/*
		 * verify data type and range of the request
		 */
		status = drvAbDf1VerifyTypeAndRange (pIO, pDevElemIO->dataType, 
									pElemIO->elemNo, pElemIO->subElemNo, 
									1u, NULL);
		if (status) {
			return status;
		}
	}

	pTrans = drvAbDf1NewWriteTrans (pElemIO);
	if (!pTrans) {
		return S_drvAbDf1_noMemory;
	}

	/*
	 * determine the proper IO type
	 */
	if (pIO->pFile->dataTypeClass==dtcTyped) {
		if (pDevElemIO->dataType==df1DTBit) {
			pTrans->protoType = dptReadModifyWrite;
		}
		else {
			pTrans->protoType = dptTypedBlockWrite;
		}
	}
	else if (pIO->pFile->dataTypeClass==dtcUntyped) {
		if (pDevElemIO->dataType==df1DTBit) {
			pTrans->protoType = dptBitWrite;
		}
		else {
			pTrans->protoType = dptBlockWrite;
		}
	}
	else {
		return S_drvAbDf1_notSubscribed;
	}

	status = drvAbDf1QueueRequest (pTrans);
	if (status) {
		return status;
	}
	else {
		return S_drvAbDf1_asyncCompletion;
	}
}

/*
 * drvAbDf1WriteCompletion() 
 */
LOCAL unsigned long drvAbDf1WriteCompletion (drvAbDf1ElemIO *pEIO)
{
	absBlockIO *pIO = pEIO->pBIO;
	abDf1Value val;
	uint8_t valBytes[4], maskBytes[4];


	/*
	 * fetch the current value to be written
	 */
	pEIO->dev.devFunc->pCurrentWriteValue (&pEIO->dev, &val);

	switch (pEIO->dev.dataType) {
	case df1DTInt:
		valBytes[0] = (uint8_t) val.word;
		valBytes[1] = (uint8_t) (val.word>>NBBY);
		/*
		 * update the cache
		 */
		return drvAbDf1WriteBlockRaw (pIO, df1DTInt, pEIO->elemNo,
						pEIO->subElemNo, 1u, valBytes, TRUE);

	case df1DTBit:
		valBytes[0] = (uint8_t) val.bitString.value;
		valBytes[1] = (uint8_t) (val.bitString.value>>NBBY);
		maskBytes[0] = (uint8_t) val.bitString.mask;
		maskBytes[1] = (uint8_t) (val.bitString.mask>>NBBY);
		/*
		 * update the cache
		 */
		return drvAbDf1WriteBitsRaw (pIO, pEIO->elemNo, 
						pEIO->subElemNo, valBytes, maskBytes);

	case df1DTFP:
		{
		uint32_t *plw = (uint32_t *) &val;
		assert (sizeof(val.real==sizeof(*plw)));
		valBytes[0] = (uint8_t) (*plw>>(0*NBBY));
		valBytes[1] = (uint8_t) (*plw>>(1*NBBY));
		valBytes[2] = (uint8_t) (*plw>>(2*NBBY));
		valBytes[3] = (uint8_t) (*plw>>(3*NBBY));
		/*
		 * update the cache
		 */
		return drvAbDf1WriteBlockRaw (pIO, df1DTFP, pEIO->elemNo,
						pEIO->subElemNo, 1u, valBytes, TRUE);
		}
	default:
		return S_drvAbDf1_uknDataType;
	}
}

/*
 * drvAbDf1VerifyTypeAndRange() 
 *
 * verify that the request is valid
 */
LOCAL unsigned long drvAbDf1VerifyTypeAndRange (absBlockIO *pIO, unsigned df1DT, 
			unsigned elemNo, unsigned subElemNo, unsigned nElem, void **ppData)
{
	int8_t *pData = (int8_t *) pIO->pData;

	if (nElem==0u) {
		return S_df1_BadAddr;
	}

	if (elemNo<pIO->elemNo) {
		return S_df1_BadAddr;
	}

	if ((elemNo-pIO->elemNo)+nElem>pIO->elemCount) {
		return S_df1_EOF;
	}
	
	/*
	 * index into the data
	 */
	pData += (elemNo-pIO->elemNo)*abDataSize(pIO->pFile->dataType);

	/*
	 * bit fields can only be extracted from types
	 * that are integers
	 */
	if (df1DT==df1DTBit) {
		df1DT = df1DTInt;
	}

	if (df1DT!=pIO->pFile->dataType) {

		/*
		 * if the types dont match then it must 
		 * be a structured type
		 */
		if (!abDataStructure(pIO->pFile->dataType)) {
			return S_df1_BadType;
		}
	
		/*
		 * type of field in structure must match
		 */
		if (df1DT != abSubElemTable(pIO->pFile->dataType)[subElemNo].type) {
			return S_df1_BadType;
		}

		/*
		 * verify that the subelement address is within the
		 * element
		 */
		if (subElemNo>=abDataSize(pIO->pFile->dataType)/sizeof(df1Word)) {
			return S_df1_BadAddr;
		}

		/*
		 * if addressing a subelement within an element then
		 * multiple elements is not allowed
		 */
		if (nElem>1u) {
			return S_df1_BadAddr;
		}

		/*
		 * subElemNo is really a word index
		 */
		pData += subElemNo*sizeof(df1Word);
	}

	if (ppData) {
		*ppData = (void *) pData;
	}

	return 0ul; /* success */
}

/*
 * drvAbDf1WriteBlockRaw() 
 * (bytes passed in here are in DF1 (little endian byte order)
 * this returns a PLC5 STX error code)
 */
LOCAL unsigned long drvAbDf1WriteBlockRaw (absBlockIO *pIO, unsigned df1DT, 
			unsigned elemNo, unsigned subElemNo, unsigned nElem, 
			const uint8_t *pBuf, unsigned typed)
{
	uint8_t *pCache;
	drvAbDf1ElemIO **ppElemIO;
	unsigned i;
	unsigned long status;
	int vxStatus;

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DT, 
								elemNo, subElemNo, nElem, (void **) &pCache);
	if (status) {
		return status;
	}

	/*
	 * MUTEX used here to force ls and ms bytes
	 * to be consistent
	 */
	vxStatus = semTake (pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	if (typed) {
		memcpy (pCache, pBuf, nElem*abDataSize(df1DT));
	}
	else {

		/*
		 * typeless block IO has bizarre byte order 
		 * (bytes within a floating value are in little endian order
		 * while at the same time the words within a floating 
		 * point value are in big endian order)
		 */
		if (pIO->pFile->dataType==df1DTFP) {
			for (i=0u; i<nElem*4; i += 4) {
				pCache[i+0] = pBuf[i+2];
				pCache[i+1] = pBuf[i+3];
				pCache[i+2] = pBuf[i+0];
				pCache[i+3] = pBuf[i+1];
			}
		} 
		/*
		 * correct difference in the data size
		 * between typed block R/W and typeless block R/W for 
		 * timer elements
		 */
		else if (pIO->pFile->dataType==df1DTTmr) {
			union bizarreTimer {
				uint16_t	w[3];
			} *pbt = (union bizarreTimer *) pBuf;
			union normalTimer {
				uint16_t	w[5];
			} *pnt = (union normalTimer *) pCache;

			i = nElem;
			while (i > 0) {

				i--;

				pnt[i].w[4] = 0; /* reserved */
				pnt[i].w[3] = 0; /* reserved */
				pnt[i].w[2] = pbt[i].w[2];
				pnt[i].w[1] = pbt[i].w[1];
				pnt[i].w[0] = pbt[i].w[0];
			}
		}
		else {
			memcpy (pCache, pBuf, nElem*abDataSize(pIO->pFile->dataType));
		}
	}

	/*
	 * MUTEX off
	 */
	vxStatus = semGive (pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	/*
	 * scan records when we update the cache
	 *
	 * no lock applied here because we dont add to or
	 * delete from this list after initialization completes
	 *
	 * if locking was used here we would need to be carefull
	 * about deadlocks with the EPICS database
	 */
	ppElemIO = &pIO->pElemIOTbl[elemNo-pIO->elemNo];
	for (i=0u; i<nElem; i++) {

		drvAbDf1ElemIO *pElemIO = ppElemIO[i];
		while (pElemIO) {
			(*pElemIO->dev.devFunc->pNewCacheValue)(&pElemIO->dev);
			pElemIO = pElemIO->pNext;
		}
	}

	return 0; /* success */
}


/*
 * drvAbDf1WriteBitsRaw() 
 * (this works exactly the same way as the AB DF1 Bit write Command)
 * this returns a PLC5 STX error code
 */
LOCAL unsigned long drvAbDf1WriteBitsRaw (absBlockIO *pIO, 
			unsigned elemNo, unsigned subElemNo, uint8_t *pVal, uint8_t *pMask)
{
	drvAbDf1ElemIO *pElemIO;
	uint8_t *pData;
	unsigned netChange;
	unsigned i;
	int vxStatus;
	long status;

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DTBit, 
								elemNo, subElemNo, 1u, (void **) &pData);
	if (status) {
		return status;
	}

	/*
	 * MUTEX used here to force ls and ms bytes
	 * to be consistent
	 */
	vxStatus = semTake (pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	/*
	 * update the specified bits
	 */
	netChange = FALSE;
	for (i=0u; i<sizeof(df1Word); i++) {

		uint8_t work = pData[i];
		work = (work & ~pMask[i]) | (pVal[i] & pMask[i]);
		if (pData[i] != work) {
			netChange = TRUE;
			pData[i] = work;
		}
	}

	/*
	 * MUTEX off
	 */
	vxStatus = semGive (pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	/*
	 * scan records when we update the cache
	 *
	 * no lock applied here because we dont add to or
	 * delete from this list after initialization completes
	 *
	 * if locking was used here we would need to be carefull
	 * about deadlocks with the EPICS database
	 */
	if (netChange) {
		pElemIO = pIO->pElemIOTbl[elemNo-pIO->elemNo];
		while (pElemIO) {
			(*pElemIO->dev.devFunc->pNewCacheValue) (&pElemIO->dev);
			pElemIO = pElemIO->pNext;
		}
	}

	return 0; /* success */
}



/*
 * drvAbDf1ReadBitString() 
 */
long
drvAbDf1ReadBitString (abDf1ElemIO *pDevElemIO, uint16_t mask, 
		uint16_t *pVal)
{
	drvAbDf1ElemIO *pElemIO = devToDrvElemIOPtr (pDevElemIO);
	absBlockIO	*pIO = pElemIO->pBIO;
	uint16_t work;
	df1Word *pData;
	int vxStatus;
	long status;

	if (!pIO) {
		return S_drvAbDf1_notSubscribed;
	}

	/*
	 * verify that the request is valid
	 */
	if (pIO->lastReadStatus) {
		return pIO->lastReadStatus;
	}

	/*
	 * verify data type and range of the request
	 */
	status = drvAbDf1VerifyTypeAndRange (pIO, df1DTBit, 
								pElemIO->elemNo, pElemIO->subElemNo, 1u, (void **) &pData);
	if (status) {
		return status;
	}

	if (mask & ~mkmask(NBBY*sizeof(df1Word))) {
		return S_drvAbDf1_badParam;
	}

	/*
	 * MUTEX used here to force ls and ms bytes
	 * to be consistent
	 */
	vxStatus = semTake (pIO->pFile->pPLC->pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	work = pData->msData;
	work <<= NBBY;
	work |= pData->lsData;

	*pVal = work & mask;

	/*
	 * MUTEX off
	 */
	vxStatus = semGive (pIO->pFile->pPLC->pDev->mutexSem);
	assert (vxStatus==OK);

	return S_drvAbDf1_OK;
}

/* 
 * drvAbDf1NewElemIO ()
 *
 * initialize (or at least clear) 
 * all bytes in the an allocated element io structure 
 * (this allows the driver to add a driver specific
 * preamble to the element IO structure)
 */
LOCAL abDf1ElemIO *drvAbDf1NewElemIO (void)
{
	drvAbDf1ElemIO *pElemIO;

	pElemIO = (drvAbDf1ElemIO *) calloc (1, sizeof(*pElemIO));
	if (!pElemIO) {
		return NULL;
	}
	return &pElemIO->dev;
}

/*
 * drvAbDf1SetupIO()
 * NOTES
 * o entries are never removed once they are added here
 * o a read cache is allowed to bridge unoccupied slots
 *  only if the	distance is larger than the protocol overhead.
 */
long drvAbDf1SetupIO (const struct link *pLink, 
				abDf1ElemIO *pDevIO, unsigned *pBitNo)
{
	drvAbDf1ElemIO *pIO = devToDrvElemIOPtr (pDevIO);
	unsigned begin;
	unsigned maxElem;
	unsigned nodeNumber;
	unsigned dataTypeClass;
	unsigned fileNo;
	unsigned fileType;
	absBlockIO *pSS;
	const char *pExpectPLC2, *pExpectPLC5;
	drvAbDf1Parm *pDev;
	long status;
	int vxStatus;
	ELLLIST *pList;
	absQueType queType;

	if (pLink->type != INST_IO) {
		return S_drvAbDf1_badAddrType;
	}

	if (pBitNo) {
		pExpectPLC2 = 
"<device (serial port) name> <PLC2 node no> <word no> <bit no>";
		pExpectPLC5 = 
"<device (serial port) name> <PLC5 node no> <N7:10/2>";
	}
	else {
		pExpectPLC2 = 
"<device (serial port) name> <PLC2 node no> <word no>";
		pExpectPLC5 = 
"<device (serial port) name> <PLC5 node no> <N7:10>";
	}
	status = parseAbDf1Address (pLink->value.instio.string, &pDev, 
				&nodeNumber, pIO, pBitNo, &fileType, &fileNo, &dataTypeClass);
	if (status) {
		errPrintf (status, __FILE__, __LINE__, 
			"- DF1 Addr = \"%s\"", 
			pLink->value.instio.string);
		epicsPrintf ("Expected one of the following\n");
		epicsPrintf ("(1) \"%s\"\n", pExpectPLC2);
		epicsPrintf ("(2) \"%s\"\n", pExpectPLC5);
		return status;
	}

	/*
	 * this is used to parse incoming ascii logical addresses
	 * when we dont have an elemnt IO structure handy
	 */
	pDev->pParseAddress = pDevIO->devFunc->pParseAddress;

	/*
	 * override the cache size if this is a local PV
	 */
	if (nodeNumber==pDev->nodeNo) {
		pList = &pDev->localList;
		queType = absQueLocal;
		maxElem = absLocalElemCount;
		/* 
		 * must specify a file number (we are emulating a PLC5)
		 */
		if (dataTypeClass!=dtcTyped) {
			errPrintf (S_drvAbDf1_badNodeNumber, __FILE__, __LINE__,
				"local variables must not use PLC2 address format");
			return S_drvAbDf1_badNodeNumber;
		}
	}
	else {
		maxElem = abDf1TypedReadDataSize/abDataSize(fileType);
		pList = &pDev->scanList;
		queType = absQueScan;
	}

	pIO->pBIO = NULL;

	/*
	 * this is slow but it only happens during init
	 */
	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (vxStatus==OK);
	for (pSS = (absBlockIO *) ellFirst(pList); pSS; 
			pSS = (absBlockIO *) ellNext(&pSS->node)) {

		/*
		 * unique addr map for each station
		 */
		if (pSS->pFile->pPLC->nodeNo != nodeNumber) {
			continue;
		}

		/*
		 * dont mix typed IO with untyped IO
		 * (PLC2 with PLC5 IO)
		 */
		if (pSS->pFile->dataTypeClass!=dataTypeClass) {
			continue;
		}

		/*
		 * file number must match if it is data typed IO
		 */
		if (pSS->pFile->dataTypeClass==dtcTyped) {
			if (pSS->pFile->fileNo!=fileNo) {
				continue;
			}
		}

		/*
		 * and do not mix data types within a file
		 */
		if (pSS->pFile->dataType!=fileType) {
			errPrintf (S_drvAbDf1_wrongType, __FILE__, __LINE__,
				"file number=%u conflict type=\"%s\" orig type=\"%s\"", 
				pSS->pFile->fileNo, abDataTypeToString (pIO->dev.dataType), 
				abDataTypeToString (pSS->pFile->dataType));
			vxStatus = semGive(pDev->mutexSem);
			assert (vxStatus==OK);
			return S_drvAbDf1_wrongType;
		}

		/*
		 * during block io reads we do not want to read in unused bytes that
		 * exceed the number of bytes in the blockio protocol overhead bytes.
		 * Nevertheless we dont want to fragment the block io operations
		 * because of the nondetermanistic order in which the elements
		 * IO structures are attached to the block IO structures.
		 *
		 * Therefore we align blocks on natural boundaries (and dont expand
		 * outside of them. If remote IO blocks are found to contain to many unused
		 * elements then they are broken up just before initialization
		 * completes. The hashing used to find local IO blocks requires that they are
		 * never broken up however.
		 */

		/* 
		 * determine beginning element of a naturally aligned block
		 */
		begin = (pSS->elemNo/maxElem)*maxElem;
		if (pIO->elemNo >= begin && 
			pIO->elemNo < begin + maxElem ) {
			break;
		}
	}

	/*
 	 * expand the coverage of the block as required
	 */
	if (pSS) {

		/*
		 * adjust the block
		 */
		if (pSS->elemNo > pIO->elemNo) {
			pSS->elemCount += pSS->elemNo - pIO->elemNo;
			pSS->elemNo = pIO->elemNo;
		}
		else if (pIO->elemNo >= pSS->elemNo+pSS->elemCount) {
			pSS->elemCount += pIO->elemNo+1u-(pSS->elemNo+pSS->elemCount);
		}

		/*
 		 * check math above 
		 */
		assert (maxElem>=pSS->elemCount);
		assert (pIO->elemNo >= pSS->elemNo);
		assert (pIO->elemNo <  pSS->elemNo+pSS->elemCount);
	}

	if (pSS) {
		pIO->pBIO = pSS;
		status = S_drvAbDf1_OK;
	}
	else {
		/*
		 * here if we didnt find an existing scan entry
		 * for this area of memory
		 */
		status = drvAbDf1CreateCache (pDev, pIO, nodeNumber,
						fileType, fileNo, pList, queType, maxElem, dataTypeClass);
		if (!status) {
			pSS = pIO->pBIO;
		}
	}

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	return status;
}

/*
 * drvAbDf1CreateCache()
 */
LOCAL long drvAbDf1CreateCache (drvAbDf1Parm *pDev, drvAbDf1ElemIO *pIO, unsigned nodeNumber, 
					unsigned fileType, unsigned fileNo, ELLLIST *pAddList, absQueType queType, 
					unsigned maxElem, unsigned dataTypeClass)
{
	absBlockIO *pSS;
	absFileIO *pFileIO;
	int vxStatus;

	pSS = (absBlockIO*) calloc (1, sizeof(*pSS));
	if (!pSS) {
		return S_drvAbDf1_noMemory;
	}

	pFileIO = drbAbDf1CreateFileIO (pDev, nodeNumber, fileNo, 
		(drvAbDf1DefaultScanPeriod_mS * sysClkRateGet()) / 1000ul, 
		fileType, dataTypeClass);
	if (!pFileIO) {
		free (pSS);
		return S_drvAbDf1_noMemory;
	}

	pSS->pFile = pFileIO;
	pSS->elemNo = pIO->elemNo;
	
	pSS->pData = (void *) calloc (maxElem, abDataSize(fileType));
	if (!pSS->pData) {
		free (pSS);
		return S_drvAbDf1_noMemory;
	}

	pSS->pElemIOTbl = calloc (maxElem, sizeof(drvAbDf1ElemIO *));
	if (!pSS->pElemIOTbl) {
		free (pSS->pData);
		free (pSS);
		return S_drvAbDf1_noMemory;
	}

	pSS->elemCount = 1u;
	pSS->lastReadStatus = S_drvAbDf1_udf;
	pSS->lastWriteStatus = S_drvAbDf1_OK;

	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (vxStatus==OK);

	pSS->queue = queType;
	ellAdd (pAddList, &pSS->node);

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	pIO->pBIO = pSS;

	return S_drvAbDf1_OK;
}

/*
 * drbAbDf1CreatePLCIO()
 */
LOCAL absPLCIO *drvAbDf1CreatePLCIO (drvAbDf1Parm *pDev, unsigned nodeNumber)
{
	absPLCIO *pPLCIO;
	int vxStatus;

	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (vxStatus==OK);

	/*
	 * attempt to attach to an existing PLC entry
	 */
	for (pPLCIO = (absPLCIO *) ellFirst (&pDev->plcIOList);
		pPLCIO; pPLCIO = (absPLCIO *)ellNext (&pPLCIO->node)) {
		if (pPLCIO->nodeNo==nodeNumber) {
			break;
		}
	}

	/*
	 * failing to find an existing PLC entry, then
	 * create a new one
	 */
	if (!pPLCIO) {
		pPLCIO = calloc (1, sizeof(*pPLCIO));
		if (pPLCIO) {
			ellInit (&pPLCIO->fileList);
			pPLCIO->pDev = pDev;
			pPLCIO->nodeNo = nodeNumber;
			pPLCIO->ioOutstandingCount = 0u;
			ellAdd (&pDev->plcIOList, &pPLCIO->node);
		}
	}

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	return pPLCIO;
}

/*
 * drbAbDf1CreateFileIO()
 */
LOCAL absFileIO *drbAbDf1CreateFileIO (drvAbDf1Parm *pDev, unsigned nodeNumber, 
				unsigned fileNo, unsigned scanPeriod, unsigned dataType, 
				unsigned dataTypeClass)
{
	absFileIO *pFileIO;
	absPLCIO *pPLC;
	int vxStatus;

	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	assert (vxStatus==OK);

	/*
	 * attach to the node specified
	 */
	pPLC = drvAbDf1CreatePLCIO (pDev, nodeNumber);
	if (!pPLC) {
		vxStatus = semGive (pDev->mutexSem);
		assert (vxStatus==OK);
		return NULL;
	}

	/*
	 * first attempt to attach to an existing file entry
	 */
	for (pFileIO = (absFileIO *) ellFirst (&pPLC->fileList);
		pFileIO; pFileIO = (absFileIO *) ellNext (&pFileIO->node)) {

		if (pFileIO->fileNo==fileNo) {
			break;
		}
	}

	/*
	 * failing to find an existing file entry, then create one
	 */
	if (!pFileIO) {
		pFileIO = calloc (1, sizeof(*pFileIO));
		if (pFileIO) {
			pFileIO->pPLC = pPLC;
			pFileIO->fileNo = fileNo;
			pFileIO->scanPeriod = scanPeriod;
			pFileIO->dataType = dataType;
			if ( dataType == df1DTBit || dataType == df1DTBitStr ) {
			    /* plc5 B# files need be processed as df1DTInt's */
			    pFileIO->effectiveDataType = df1DTInt;
			}
			else {
			    pFileIO->effectiveDataType = dataType;
			}
			pFileIO->dataTypeClass = dataTypeClass;
			ellAdd (&pPLC->fileList, &pFileIO->node);
		}
	}
	else if (pFileIO->dataTypeClass==dtcUnknown) {
		pFileIO->dataTypeClass = dataTypeClass;
		pFileIO->dataType = dataType;
	}
	else if (pFileIO->dataTypeClass!=dataTypeClass) {
		pFileIO = NULL;
	}
	else if (pFileIO->dataTypeClass==dtcTyped && 
		pFileIO->dataType != dataType) {
		pFileIO = NULL;
	}

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	return pFileIO;
}

/*
 * drvAbDf1SetupScan()
 * (this allows the driver to behave differently
 * if the element isnt "scanned on interrupt" in the
 * EPICS database)
 *
 * this is always called for outputs and sometimes
 * called for inputs when the scan type is I/O interrupt
 *
 */
LOCAL void drvAbDf1SetupScan (const int cmd, abDf1ElemIO *pElemIO)
{
	drvAbDf1ElemIO *pIO = devToDrvElemIOPtr(pElemIO);
	drvAbDf1ElemIO **ppEIO;

	ppEIO = &pIO->pBIO->pElemIOTbl[pIO->elemNo-pIO->pBIO->elemNo];

	switch (cmd) {
	case abDf1IntScanStart:
		pIO->pNext = *ppEIO;
		*ppEIO = pIO;
		break;

	case abDf1IntScanStop:
		while (*ppEIO) {
			if (*ppEIO == pIO) {
				*ppEIO = pIO->pNext;
				break;
			}
			ppEIO = &(*ppEIO)->pNext;
		}
		break;

	default:
		break;
	}
}

/*
 * drvAbDf1NewWriteTrans()
 */
LOCAL absTransaction *drvAbDf1NewWriteTrans (drvAbDf1ElemIO *pElemIO)
{
	absBlockIO	*pIO = pElemIO->pBIO;
	absTransaction *pTrans = drvAbDf1NewTrans (pIO->pFile->pPLC->pDev);
	if (pTrans) {
		pTrans->pElemIO = pElemIO;
		pTrans->pIO = pIO;
	}
	return pTrans;
}

/*
 * drvAbDf1NewReadTrans()
 */
LOCAL absTransaction *drvAbDf1NewReadTrans (absBlockIO *pIO)
{
	absTransaction *pTrans = drvAbDf1NewTrans (pIO->pFile->pPLC->pDev);
	if (pTrans) {
		pTrans->pIO = pIO;
		pTrans->read = TRUE;
	}
	return pTrans;
}

/*
 * drvAbDf1MoveTrans()
 * (lock must be applied)
 */
LOCAL void drvAbDf1MoveTrans (absTransaction *pTrans, ELLLIST *pNewList)
{
	ellDelete (pTrans->pTransList, &pTrans->node);
	pTrans->pTransList = pNewList;
	ellAdd (pTrans->pTransList, &pTrans->node);
}

/*
 * drvAbDf1NewTrans()
 */
LOCAL absTransaction *drvAbDf1NewTrans (drvAbDf1Parm *pDev)
{
	absTransaction *pTrans;
	long status;
	int	semStatus;

	pTrans = (absTransaction *) freeListCalloc (pDev->pTransFreeListPVT);
	if (!pTrans) {
		return pTrans;
	}

	/*
	 * Allocate Transaction ID
	 * MUTEX ON/OFF around ++ op which potentially isnt atomic 
	 * and use of hash table
	 */
	semStatus = semTake(pDev->mutexSem, MUTEX_TMO);
	if (semStatus!=OK) {
		freeListFree (pDev->pTransFreeListPVT, pTrans);
		return NULL;
	}

	pTrans->ticksAtReq = tickGet();

	/*
	 * default to something benign
	 */
	pTrans->protoType = dptLoopBack;

	while (TRUE) {
		pDev->nextTransId++;
		pTrans->transId = pDev->nextTransId;

		/*
		 * place trans id in hash table
		 */
		status = bucketAddItemUnsignedId(
				pDev->pTransBucket, &pTrans->transId, pTrans);

#		ifdef S_bucket_idInUse
			if (status!=S_bucket_idInUse) {
				break;
			}
#		else
			break;
#		endif
	}
	
	pTrans->respPending = FALSE;

	pTrans->pTransList = &pDev->transLimboList;
	ellAdd (pTrans->pTransList, &pTrans->node);

	semStatus = semGive (pDev->mutexSem);
	assert (semStatus==OK);

	return pTrans;
}

/*
 * absDisposeTransaction()
 */
LOCAL drvAbDf1Status absDisposeTransaction (absTransaction *pTrans)
{
	absBlockIO *pIO = pTrans->pIO;
	drvAbDf1Parm *pDev = pIO->pFile->pPLC->pDev;
	long status;
	int vxStatus;
	ULONG current;
	ULONG delay;

	/*
	 * MUTEX ON/OFF around hash table / list use
	 */
	vxStatus = semTake (pDev->mutexSem, WAIT_FOREVER);
	if (vxStatus!=OK) {
		epicsPrintf ("%s: unable to dispose transaction (bad sem \"%s\")\n", 
			__FILE__, strerror(errno));
		return S_drvAbDf1_badTransId;
	}

	status = bucketRemoveItemUnsignedId(
			pDev->pTransBucket, &pTrans->transId);
	if (status!=S_bucket_success) {
		epicsPrintf ("%s: unable to dispose corrupt transaction (bad Id=%x)\n", 
			__FILE__, pTrans->transId);
		vxStatus = semGive(pDev->mutexSem);
		assert (vxStatus==OK);
		return S_drvAbDf1_badTransId;
	}
		
	/*
	 * if a frame for this transaction has been sent then
	 * decrement the number of frames known to be outstanding
	 * against this PLC
	 */
	if (pTrans->respPending) {
		pTrans->pIO->pFile->pPLC->ioOutstandingCount--;
		/*
		 * the send task could be waiting for this
		 * semaphore (if more than drvAbDf1MaxOutstandingRequest
		 * requests were pending)
		 */
		if (pTrans->pIO->pFile->pPLC->ioOutstandingCount+1u==
			drvAbDf1MaxOutstandingRequest) {
			semFlush (pDev->ackSem);
		}
	}

	assert (pTrans->pTransList);
	ellDelete (pTrans->pTransList, &pTrans->node);

	current = tickGet();
	if (current>=pTrans->ticksAtReq) {
		delay = current - pTrans->ticksAtReq;
	}
	else {
		delay = ULONG_MAX-pTrans->ticksAtReq;
		delay += current;
	}

	pDev->smoothedDelayToResp = 
		(pDev->smoothedDelayToResp*3+delay)/4;
	pDev->maxDelayToResp = max (delay,pDev->maxDelayToResp);

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	freeListFree (pDev->pTransFreeListPVT, pTrans);

	return S_drvAbDf1_OK;
}

/*
 * cmdGeneralIO ()
 */
LOCAL void cmdGeneralIO (drvAbDf1Parm *pDev, drvSerialResponse *pCmd) 
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;

	/*
	 * call the general IO command function
	 * through a jump table
	 */
	(*plc5GeneralCmdJumpTable[pReq->general.fnc])(pDev, pCmd);
}

/*
 * writeLocalBlock()
 */
LOCAL drvAbDf1Status writeLocalBlock (drvAbDf1Parm *pDev, unsigned fileNo, 
		unsigned elemNo, unsigned subElemNo, unsigned nElem, 
		unsigned elemType, const uint8_t *pBytes)
{
	drvAbDf1Status status;
	localIODescriptor first, second;

	/*
	 * Locate the IO block associated with this
	 * write request
	 *
	 * Every block can be uniquely located by its
	 * file number an its starting byte address
	 *
	 * When a PLC5 receives a request directed at a PLC2
	 * it assigns the request to a file number equivalent
	 * to the node number that is the source of the
	 * request
	 */
	status = locateBlockIO (pDev, fileNo, elemNo, subElemNo, nElem, elemType==df1DTNone,
					elemType, &first, &second);
	if (status) {
		return status;
	}

	if (first.pIO && first.elemCount) {

		status = drvAbDf1WriteBlockRaw (first.pIO, first.elemType, first.elemNo,
					subElemNo, first.elemCount, pBytes, elemType!=df1DTNone);
		if (status) {
			return status;
		}

		first.pIO->ioCount++;

		if (second.pIO && second.elemCount) {
			/*
			 * we never write more than one element when a sub element
			 * is addressed, and therefore will not be here (and therefore
			 * it is ok to use the file type here).
			 */
			pBytes += first.elemCount * abDataSize (first.pIO->pFile->dataType);
			status = drvAbDf1WriteBlockRaw (second.pIO, second.elemType, second.elemNo,
						subElemNo, second.elemCount, pBytes, elemType!=df1DTNone);
			if (status) {
				return status;
			}
		
			second.pIO->ioCount++;
		}
	}
	return S_drvAbDf1_OK;
}

/*
 * readLocalBlock()
 */
LOCAL drvAbDf1Status readLocalBlock (const localIODescriptor *pFirst, 
		const localIODescriptor *pSecond, unsigned subElemNo, uint8_t *pBytes, unsigned nBytes)
{
	unsigned long status;
	unsigned segSize;

	if (pFirst->pIO) {
		segSize = pFirst->elemCount*abDataSize(pFirst->pIO->pFile->dataType);
		if (nBytes<segSize) {
			return S_df1_DataTooBig;
		}
		else {
			nBytes -= segSize;
		}
		/*
		 * There appears to be an ambiguity in the DF1 protocol where 1
		 * element is fetched with an address specifying subelement zero. 
		 * Should we return subelement zero or the entire structured type?
		 * We are currently returning the entire compound type.
		 *
		 * Hence the use of pFirst->pIO->pFile->dataType below.
		 */
		status = drvAbDf1ReadBlock (pFirst->pIO, pFirst->elemType, 
				pFirst->elemNo, subElemNo, pFirst->elemCount, pBytes);
		if (status) {
			return status;
		}

		pFirst->pIO->ioCount++;

		if (pSecond->pIO) {
			segSize = pSecond->elemCount*abDataSize(pSecond->pIO->pFile->dataType);
			if (nBytes<segSize) {
				return S_df1_DataTooBig;
			}
			pBytes += segSize;
			status = drvAbDf1ReadBlock (pSecond->pIO, pSecond->elemType, 
					pSecond->elemNo, subElemNo, pSecond->elemCount, pBytes);
			if (status) {
				return status;
			}
			pSecond->pIO->ioCount++;
		}
	}
	return S_drvAbDf1_OK;
}

/*
 * locateBlockIO ()
 *
 * Locate the IO block associated with this request
 *
 * Every block can be uniquely located by its
 * file number and its starting byte address
 *
 */
LOCAL drvAbDf1Status locateBlockIO (const drvAbDf1Parm *pDev, unsigned fileNo, 
		unsigned elemNo, unsigned subElemNo, unsigned elemCount, 
		unsigned elemCountIsInBytes, unsigned elemType,
		localIODescriptor *pFirst, localIODescriptor *pSecond)
{
	absBlockIO *pIO;
	ELLLIST *pList;
	unsigned id;
	unsigned long status;
	int vxStatus;
	localIODescriptor first;

	/*
	 * it is possible that there was not enough memeory
	 * to create this during init, or that there are
	 * not any local files in this IOC
	 */
	if (pDev->pLocalHashList==NULL) {
		return S_df1_BadAddr;
	}

	id = fileNo ^ (elemNo/absLocalElemCount); 
	id %= pDev->localHashSize;

	/*
	 * lock around use of the hash table
	 */
	vxStatus = semTake (pDev->mutexSem, MUTEX_TMO);
	if (vxStatus!=OK) {
		return S_drvAbDf1_mutexTMO;
	}

	pList = &pDev->pLocalHashList[id];
	for (pIO = (absBlockIO *) ellFirst(pList); 
			pIO; pIO = (absBlockIO *) ellNext(&pIO->node)) {

		if (pIO->elemNo<=elemNo) {
			if ( (elemNo - pIO->elemNo) < pIO->elemCount ) {
				break;
			}
		}
	}

	vxStatus = semGive (pDev->mutexSem);
	assert (vxStatus==OK);

	if (!pIO) {
		return S_df1_BadAddr;
	}

	if (pIO->pFile->pPLC->nodeNo != pDev->nodeNo) {
		return S_df1_Unknown;
	}

	/* 
	 * type/sizeInBytes are independently specified because, yes,
	 * the DF1 protocol can specify an untyped get with a
	 * a number of elements, and not a number of bytes. Likewise,
	 * we can have a typed put which specifies a data type
	 * that is not the file type, but is instead a subelement type.
	 * We also have an untyped get where the data type is
	 * unknown and the element count is in bytes.
	 */
	if (elemType==df1DTNone) {
		if (elemCountIsInBytes) {
			/*
			 * guess the appropriate element type based on the number of
			 * bytes and the subelement number
			 */
			if (abDataStructure(pIO->pFile->dataType)) {
				unsigned elemSize = abDataSize(pIO->pFile->dataType);
				if (elemCount%elemSize==0u) {
					elemType = pIO->pFile->dataType;
				}
				else if (subElemNo<elemSize/sizeof(df1Word)) {
					elemType = abTypeInfo_array[pIO->pFile->dataType].pSubElem->type;
					if (elemCount!=abDataSize(elemType)) {
						return S_df1_BadParm;
					}
				}
				else {
					return S_df1_BadAddr;
				}
			}
			else {
				elemType = pIO->pFile->dataType;
			}
		}
		else {
			/*
			 * if the number of bytes isnt known then we cant determine
			 * sub element vs element get and assume that the data type
			 * is the file type
			 */
			elemType = pIO->pFile->dataType;
		}
	}

	if (elemCountIsInBytes) {
		elemCount /= abDataSize(elemType);
	}

	/*
	 * check for addressing past file boundaries
	 */
	if (pIO->elemCount<absLocalElemCount) {
		if ( elemNo + elemCount > pIO->elemNo + pIO->elemCount) {
			return S_df1_EOF;
		}
	}

	first.pIO = pIO;
	first.elemNo = elemNo;
	first.elemCount = min (elemCount, absLocalElemCount);
	first.elemType = elemType;

	/*
	 * now look to see if a second block is involved
	 * (this io crosses block boundaries)
	 */
	if (first.elemCount<elemCount) {
		if (!pSecond) {
			return S_df1_Unknown; /* internal error */
		}
		status = locateBlockIO (pDev, fileNo, elemNo + first.elemCount, subElemNo,
					elemCount - first.elemCount, FALSE, first.elemType, pSecond, NULL);
		if (status) {
			return status;
		}
		/*
		 * if two blocks are involved then there data types must 
		 * match
		 */
		if (first.pIO->pFile->dataType != pSecond->pIO->pFile->dataType) {
			return S_df1_Unknown;
		}
	}
	else if (pSecond) {
		pSecond->pIO = NULL;
		pSecond->elemNo = 0u;
		pSecond->elemCount = 0u;
		pSecond->elemType = df1DTNone;
	}
	*pFirst = first;

	return S_drvAbDf1_OK;
}

/*
 * cmdInvalid ()
 */
LOCAL void cmdInvalid (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;

	/*
	 * the response packet is just a reflection of the
	 * request with swapped src/dst
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, S_df1_IllCmd);

	errPrintf(S_df1_IllCmd, __FILE__, __LINE__,
		"DF1 command=0X%X from node 0X%X ignored", 
		pReq->hdr.cmd&df1CmdMask, pReq->hdr.src);
}

/*
 * cmdBlockWrite ()
 */
LOCAL void cmdBlockWrite (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status;
	unsigned byteAddr;
	unsigned nBytes;

	/*
	 * compute the byte address
	 */
	byteAddr = pReq->blockWrite.msAddr;
	byteAddr <<= NBBY;
	byteAddr |= pReq->blockWrite.lsAddr;

	nBytes = pCmd->bufCount - (pReq->blockWrite.data.bytes - pReq->buf);

	/*
	 * only even (word) addresses allowed
	 */
	if ((byteAddr&1u)||(nBytes&1u)) {
		status = S_df1_InvAddr;
	}
	else {
		/*
		 * PLC5 directs PLC2 write block commands at a file
		 * matching the node number of the sender
		 */
		status = writeLocalBlock (pDev, pReq->hdr.src, byteAddr>>1u, 0u,
			nBytes, df1DTNone, pReq->blockWrite.data.bytes);
		if (status) {
			status = S_df1_InvAddr; /* force primative DF2 status */
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdBlockRead ()
 */
LOCAL void cmdBlockRead (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	localIODescriptor first, second;
	drvAbDf1Status status;
	unsigned byteAddr;

	/*
	 * compute the byte address
	 */
	byteAddr = pReq->blockRead.msAddr;
	byteAddr <<= NBBY;
	byteAddr |= pReq->blockRead.lsAddr;

	/*
	 * only even (word) addresses allowed
	 */
	if ((byteAddr&1u) || (pReq->blockRead.byteCount&1u)||
			pCmd->bufCount!=sizeof(pReq->blockRead)) {
		status = S_df1_InvAddr;
	}
	else {

		/*
		 * PLC5 directs PLC2 read block commands at a file
		 * matching the node number of the sender
		 *
		 * Every block can be uniquely located by its
		 * file number an its starting element address
		 */
		status = locateBlockIO (pDev, pReq->hdr.src, byteAddr>>1u, 0u, 
						pReq->blockRead.byteCount, TRUE, df1DTNone, &first, &second);
		if (!status) {
			if (first.pIO->pFile->effectiveDataType==df1DTInt) {
				status = readLocalBlock (&first, &second, 0u,
					pRes->blockRead.data.bytes, sizeof(pRes->blockRead.data));
			}
			else {
				status = S_df1_InvAddr;
			}
		}
	}

	if (status) {
		status = S_df1_InvAddr; /* force primative PLC2 status */
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr) + pReq->blockRead.byteCount;
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdBitWrite ()
 */
LOCAL void cmdBitWrite (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status = S_drvAbDf1_OK;
	uint8_t val[2u], mask[2u];
	localIODescriptor first;
	unsigned nsets;
	unsigned i, j;
	unsigned byteAddr;

	nsets = (pCmd->bufCount-sizeof(pReq->bitWrite.hdr))/sizeof(pReq->bitWrite.bits[0]);
	for (i=0u; i<nsets; i++) {
		byteAddr = pReq->bitWrite.bits[i].msAddr;
		byteAddr <<= NBBY;
		byteAddr |= pReq->bitWrite.bits[i].lsAddr;


		/*
		 * Locate the IO block associated with this
		 * bit write request
		 *
		 * Every block can be uniquely located by its
		 * file number an its starting element address
		 */
		status = locateBlockIO (pDev, pReq->hdr.src, byteAddr>>1u, 0u,
						1u, FALSE, df1DTInt, &first, NULL);
		if (status) {
			break;
		}

		/*
		 * convert to internal bit field insert description
		 */
		for (j=0u; j<sizeof(df1Word); j++) {
			if ( (byteAddr&1u) == j) {
				mask[j] = pReq->bitWrite.bits[i].setBits ^ pReq->bitWrite.bits[i].clrBits;
				val[j] = pReq->bitWrite.bits[i].setBits & mask[j];
			}
			else {
				mask[j] = 0u;
				val[j] = 0u;
			}
		}

		/*
		 * write the bits involved
		 */
		status = drvAbDf1WriteBitsRaw (first.pIO, byteAddr>>1, 0u, val, mask);
		if (status) {
			break;
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdFileBlockWrite ()
 */
LOCAL void cmdFileBlockWrite (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status;
	unsigned fileNo, elemNo, subElemNo, offset, totalTrans;
	unsigned dataSize;
	const uint8_t *pb;

	/*
	 * fetch the PLC5 system address
	 */
	pb = pReq->fileBlockWrite.data.bytes;
	status = fetchPLC5Addr (pDev->pParseAddress, &pb, &fileNo, &elemNo, &subElemNo);
	if (status==S_drvAbDf1_OK) {
		dataSize = pCmd->bufCount - (pb - pReq->buf);

		offset = pReq->fileBlockWrite.msPO;
		offset <<= NBBY;
		offset |= pReq->fileBlockWrite.lsPO;
		elemNo += offset;

		totalTrans = pReq->fileBlockWrite.msTT;
		totalTrans <<= NBBY;
		totalTrans |= pReq->fileBlockWrite.lsTT;

		if (totalTrans*sizeof(df1Word) == dataSize) {
			status = writeLocalBlock (pDev, fileNo, elemNo, subElemNo,
							dataSize, df1DTNone, pb);
		}
		else {
			/*
			 * it is difficult to determine what will occur in the
			 * next frame when TT does not match the number of bytes
			 * in this message. Will there be another header?
			 * Will there just be raw bytes?
			 */
			drvAbDf1DebugPrintf (1, 
				"cmdFileBlockWrite: total trans requires multiple frames");
			status = S_df1_BadParm;
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdFileBlockRead ()
 */
LOCAL void cmdFileBlockRead (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	localIODescriptor first, second;
	drvAbDf1Status status;
	unsigned fileNo, elemNo, subElemNo, offset;
	unsigned dataSize, totalTrans;
	const uint8_t *pb;

	/*
	 * fetch the PLC5 system address
	 */
	pb = pReq->fileBlockRead.data.bytes;
	status = fetchPLC5Addr (pDev->pParseAddress, &pb, &fileNo, &elemNo, &subElemNo);
	if (status!=S_drvAbDf1_OK) {
		dataSize = 0u;
	}
	else {
		dataSize = *pb++;

		offset = pReq->fileBlockRead.msPO;
		offset <<= NBBY;
		offset |= pReq->fileBlockRead.lsPO;
		elemNo += offset;

		totalTrans = pReq->fileBlockRead.msTT;
		totalTrans <<= NBBY;
		totalTrans |= pReq->fileBlockRead.lsTT;

		if (dataSize>sizeof(pRes->blockRead.data)) {
			drvAbDf1DebugPrintf (1, "cmdFileBlockRead: data request exceeds space avail in frame");
			status = S_df1_DataTooBig;
		}
		else if ((pb-pReq->buf) != (ptrdiff_t) pCmd->bufCount) {
			drvAbDf1DebugPrintf (1, "cmdFileBlockRead: junk at the end of the request");
			status = S_df1_IllField;
		}
		else if (totalTrans*sizeof(df1Word) != dataSize) {
			/*
			 * it is difficult to determine what will occur in the
			 * multiple frames when TT does not match the number of bytes
			 * requested in this message. Will there be another header?
			 * Will there just be raw bytes?
			 */
			drvAbDf1DebugPrintf (1, "cmdFileBlockRead: total trans requires multiple frames");
			status = S_df1_BadParm;
		}
		else {
			/*
			 * PLC5 directs PLC2 read block commands at a file
			 * matching the node number of the sender
			 *
			 * Every block can be uniquely located by its
			 * file number an its starting element address
			 */
			status = locateBlockIO (pDev, fileNo, elemNo, subElemNo,
							dataSize, TRUE, df1DTNone, &first, &second);
			if (!status) {
				status = readLocalBlock (&first, &second, subElemNo,
					pRes->blockRead.data.bytes, sizeof(pRes->blockRead.data));
			}
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr) + dataSize;
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdTypedBlockWrite ()
 */
LOCAL void cmdTypedBlockWrite (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status;
	unsigned fileNo, elemNo, subElemNo, offset;
	unsigned dataType, dataSize, totalTrans;
	const uint8_t *pb;

	/*
	 * fetch the PLC5 system address
	 */
	pb = pReq->typedWrite.data.bytes;
	status = fetchPLC5Addr (pDev->pParseAddress, &pb, &fileNo, &elemNo, &subElemNo);
	if (status == S_drvAbDf1_OK) {
		/* 
		 * fetch data type and size from the message
		 */
		pb = fetchPLC5TypeDataParam (&dataType, &dataSize, pb);
		if (pb) {

			offset = pReq->typedWrite.msPO;
			offset <<= NBBY;
			offset |= pReq->typedWrite.lsPO;
			elemNo += offset;

			totalTrans = pReq->typedWrite.msTT;
			totalTrans <<= NBBY;
			totalTrans |= pReq->typedWrite.lsTT;

			/*
			 * subtract the header out of the frame length
			 */
			if ( (pb-pReq->buf) < (ptrdiff_t) pCmd->bufCount && dataSize==abDataSize(dataType)) {
				unsigned nElem = (pCmd->bufCount - (pb-pReq->buf))/dataSize;
				if (totalTrans*abDataSize(dataType)==dataSize) {
					status = writeLocalBlock (pDev, fileNo, elemNo, subElemNo,
										nElem, dataType, pb);
				}
				else {
					/*
					 * it is difficult to determine what will occur in the
					 * multiple frames when TT does not match the number of bytes
					 * requested in this message. Will there be another header?
					 * Will there just be raw bytes?
					 */
					drvAbDf1DebugPrintf (1, "cmdTypedBlockWrite: total trans requires multiple frames");
					status = S_df1_BadParm;
				}
			}
			else {
				drvAbDf1DebugPrintf (1, "cmdTypedBlockWrite: data size/type dont match");
				status = S_df1_BadParm;
			}
		}
		else {
			drvAbDf1DebugPrintf (1, "cmdTypedBlockWrite: invalid type/data parameter");
			status = S_df1_BadParm;
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdTypedBlockRead ()
 */
LOCAL void cmdTypedBlockRead (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status;
	unsigned fileNo, elemNo, subElemNo, offset;
	unsigned elemCount, totalTrans, nBytes = 0u;
	localIODescriptor first, second;
	const uint8_t *pb;

	/*
	 * fetch the PLC5 system address
	 */
	pb = pReq->typedRead.data.bytes;
	status = fetchPLC5Addr (pDev->pParseAddress, &pb, &fileNo, &elemNo, &subElemNo);
	if (status == S_drvAbDf1_OK) {
		elemCount = pb[1];
		elemCount <<= NBBY;
		elemCount |= pb[0];
		pb += 2;

		totalTrans = pReq->typedWrite.msTT;
		totalTrans <<= NBBY;
		totalTrans |= pReq->typedWrite.lsTT;

		if ( (pb-pReq->buf) != (ptrdiff_t) pCmd->bufCount ) {
			drvAbDf1DebugPrintf (1, "cmdTypedBlockRead: junk at the end of the request");
			status = S_df1_IllField;
		}
		else if (totalTrans!=elemCount) {
			/*
			 * it is difficult to determine what will occur in the
			 * multiple frames when TT does not match the number of bytes
			 * requested in this message. Will there be another header?
			 * Will there just be raw bytes?
			 */
			drvAbDf1DebugPrintf (1, "cmdTypedBlockRead: total trans requires multiple frames");
			status = S_df1_BadParm;
		}
		else {

			offset = pReq->typedRead.msPO;
			offset <<= NBBY;
			offset |= pReq->typedRead.lsPO;
			elemNo += offset;

			/*
			 * Locate the IO block associated with this
			 * read request
			 *
			 * Every block can be uniquely located by its
			 * file number and its starting element address
			 */
			status = locateBlockIO (pDev, fileNo, elemNo, subElemNo, elemCount, FALSE, df1DTNone,
							&first, &second);
			if (!status) {
				uint8_t *pOutB;
				pOutB = pushPLC5TypeDataParam (pRes->typedRead.data.bytes, first.elemType);
				nBytes = pOutB - pRes->typedRead.data.bytes;
				status = readLocalBlock (&first, &second, subElemNo, pOutB,
					sizeof(pRes->typedRead.data)-nBytes);
				nBytes += elemCount*abDataSize(first.elemType);
			}
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr) + nBytes;
	reflectDF1Frame (pCmd, status);
}

/*
 * cmdReadModifyWrite ()
 */
LOCAL void cmdReadModifyWrite (drvAbDf1Parm *pDev, drvSerialResponse *pCmd)
{
	abDf1ReqProto *pReq = (abDf1ReqProto *) pCmd->buf;
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Status status;
	unsigned fileNo, elemNo, subElemNo;
	uint8_t val[2u], mask[2u];
	localIODescriptor first;
	unsigned i;
	const uint8_t *pb;
	const uint8_t *pl;

	pb = pReq->typedBitWrite.data.bytes;
	pl = pReq->buf + pCmd->bufCount;

	/* 
	 * must be room for a simple system 
	 * address and and/or mask 
	 */
	status = S_df1_IllField;
	while (pb<=pl-5u) {

		/*
		 * fetch the PLC5 system address
		 */
		status = fetchPLC5Addr (pDev->pParseAddress, &pb, &fileNo, &elemNo, &subElemNo);
		if (status) {
			break;
		}

		/* 
		 * must be room for the and/or mask 
		 */
		if (pb>pl-4u) {
			status = S_df1_IllField;
			break;
		}

		/*
		 * Locate the IO block associated with this
		 * read request
		 *
		 * Every block can be uniquely located by its
		 * file number an its starting element address
		 */
		status = locateBlockIO (pDev, fileNo, elemNo, 0u, 
					1u, FALSE, df1DTInt, &first, NULL);
		if (status) {
			break;
		}

		/*
		 * convert to internal bit field insert description
		 */
		for (i=0u; i<sizeof(df1Word); i++) {
			mask[i] = ~pb[i] | pb[i+2u];
			val[i] = pb[i+2u] & mask[i];
		}

		pb += 4u;

		/*
		 * write the bits involved
		 */
		status = drvAbDf1WriteBitsRaw (first.pIO, elemNo, subElemNo, val, mask);
		if (status) {
			break;
		}
	}

	/*
	 * formulate the response packet
	 */
	pDSRes->pAppPrivate = pDev;
	pDSRes->bufCount = sizeof (pRes->hdr);
	reflectDF1Frame (pCmd, status);
}

/*
 * reflectDF1Frame ()
 */
LOCAL void reflectDF1Frame (const drvSerialResponse *pCmd, unsigned long status)
{
	drvSerialRequest *pDSRes = pCmd->pAppPrivate;
	const abDf1ReqProto *pReq = (const abDf1ReqProto *) pCmd->buf;
	abDf1ResProto *pRes = (abDf1ResProto *) pDSRes->buf;
	drvAbDf1Parm *pDev = (drvAbDf1Parm *) pDSRes->pAppPrivate;
	unsigned long stsCode;
	unsigned long stxCode;

	pRes->hdr.cmd = pReq->hdr.cmd | df1RespMask;
	pRes->hdr.dst = pReq->hdr.src;
	pRes->hdr.src = pReq->hdr.dst;
	pRes->hdr.lsTns = pReq->hdr.lsTns;
	pRes->hdr.msTns = pReq->hdr.msTns;

	if (status==0) {
		pRes->hdr.sts = '\0';
	}
	else {
		stsCode = status^M_df1;
		stxCode = status^M_df1e;
		if (stsCode<UCHAR_MAX) {
			pRes->hdr.sts = (uint8_t) stsCode;
			pDSRes->bufCount = sizeof(pRes->hdr);
		}
		else if (stxCode<UCHAR_MAX && pReq->hdr.cmd>=df1CmdPlc4General){
			pRes->hdr.sts = (uint8_t) S_df1_STX;
			pRes->hdrstx.stx = (uint8_t) stxCode;
			pDSRes->bufCount = sizeof(pRes->hdrstx);
		}
		else {
			/*
			 * the DF1 protocol spec indicates that we will not
			 * return STX in this situation. This problem is
			 * addressed at a higher level where the request type
			 * is known
			 */
			errMessage (status, 
				"Unable to return this status to remote node in reflectDF1Frame()");
			pRes->hdr.sts = (uint8_t) S_df1_IllCmd;
			pDSRes->bufCount = sizeof(pRes->hdr);
		}
	}

	pDSRes->pCB = drvAbDf1SendReply;

	/*
	 * the request buffer was preallocated when the cmd frame
	 * was received so we should always bve able to queue
	 * the response here
	 */
	status = drvSerialSendReservedRequest (pDev->id, pDSRes);
	if (status) {
		errMessage(status, "unable to send resp to AB DF1 request");
	}
}

/*
 * parseAbDf1Address()
 */
LOCAL long parseAbDf1Address (const char *pAddr, drvAbDf1Parm **ppDev, 
				unsigned *pNodeNumber, drvAbDf1ElemIO *pIO, unsigned *pBitNo, 
				unsigned *pFileType, unsigned *pFileNo, unsigned *pDataTypeClass) 
{
	char link[64];
	unsigned long node, word;
	const char *pStr;
	char *pFile;
	long status;

	/*
	 * remove white space
	 */
	while (isspace(*pAddr)) {
		pAddr++;
	}

	/*
	 * determine the file (port) name
	 * (and move the string pointer to the next field)
	 */
	pFile = link;
	while (isgraph(*pAddr) && pFile<&link[sizeof(link)-1]) {
		*pFile++ = *pAddr++;
	}
	*pFile = '\0';

	status = drvAbDf1CreateLink (link, ppDev);
	if (status) {
		return status;
	}

	/*
	 * determine the node number
	 *
	 * third strtoul parameter specifies that radix is determined
	 * using C style unsigned constant convention
	 */
	node = strtoul (pAddr, (char **) &pStr, 0);
	if (pStr==pAddr) {
		return S_drvAbDf1_badNodeNumber;
	}
	if (node>0xfe) {
		return S_drvAbDf1_badNodeNumber;
	}
	pAddr = pStr;

	/*
	 * look for PLC2 style address 
	 *
	 * link station word [bit]
	 *              ^^^^
	 *
	 * third strtoul parameter specifies that radix is determined
	 * using C style unsigned constant convention
	 */
	word = strtoul (pAddr, (char **) &pStr, 0);

	/*
	 * check for the original AB PLC2 addressing syntax
	 * (used by KECK and others)
	 *
	 * link station word [bit]
	 * 0-0376 (station number)
	 * 0-0xffff (word)
	 */
	if (pStr!=pAddr) {
		unsigned long bitNo;

		if (word>0xffff) {
			return S_drvAbDf1_badElement;
		}

		if (pBitNo) {
			pAddr = pStr;
			bitNo = strtoul (pAddr, (char **) &pStr, 0);
			if (pStr==pAddr || bitNo>=16ul) {
				return S_drvAbDf1_badBitNumber;
			}
			*pBitNo = (unsigned) bitNo;
			pIO->dev.dataType = df1DTBit;
		}
		else {
			pIO->dev.dataType = df1DTInt;
		}

		/*
		 * verify that the last of the string is
		 * white space
		 */
		while (*pStr) {
			if (!isspace(*pStr++)) {
				return S_drvAbDf1_addrGarbage;
			}
		}

		*pDataTypeClass = dtcUntyped;

		/*
		 * file number not used with PLC2 architecture
		 */
		*pFileType = df1DTInt;
		*pFileNo = 0xffff;
		pIO->elemNo = (uint16_t) word;
		pIO->subElemNo = 0;
	}
	else {
		int fileType, dataType, fileNo, elemNo, subElemNo,
				bitNo, elementSize, structured;
		/*
		 * look for a PLC5 style address
		 */
		status = pIO->dev.devFunc->pParseAddress (
				pStr, &fileType, &dataType, &fileNo, 
				&elemNo, &subElemNo, &bitNo, 
				&elementSize, &structured);
		if (status) {
			return status;
		}

		if (bitNo>=16 || bitNo<0) {
			return S_drvAbDf1_badBitNumber;
		}

		if (pBitNo) {
			if (dataType != df1DTBit) {
				return S_drvAbDf1_badBitNumber;
			}		
			*pBitNo = (unsigned) bitNo;
		}
		else {
			if (dataType == df1DTBit) {
				return S_drvAbDf1_badBitNumber;
			}			
		}

		*pDataTypeClass = dtcTyped;
		
		if (fileNo>0xffff || fileNo<0) {
			return S_drvAbDf1_badFile;
		}
		if (elemNo>0xffff || elemNo<0) {
			return S_drvAbDf1_badElement;
		}
		if (subElemNo>0xffff || subElemNo<0) {
			return S_drvAbDf1_badSubelement;
		}
		*pFileType = fileType;
		*pFileNo = (unsigned) fileNo;
		pIO->elemNo = (uint16_t) elemNo;
		pIO->subElemNo = (uint16_t) subElemNo;
		pIO->dev.dataType = dataType;
	}

	*pNodeNumber = (unsigned) node;

	return S_drvAbDf1_OK;
}


