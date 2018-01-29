/*************************************************************************
 * drvAbDf1.h -- Allen Bradley DF1 Serial Protocol EPICS Driver Support
 *               For PLC-2, PLC-5, and PLC-5/V Controllers.
 *
 * Authors:  Jeff Hill, Eric Bjorklund
 * Date:     17 October 1997
 *
 *------------------------------------------------------------------------
 * MODIFICATION LOG:
 *
 * 06-Jun-2016 mdw      Modified to use <epicsTypes.h> instead of <sys/types.h>
 *                      Fixed up funtion pointer typdefs
 * 17-Oct-1997 joh,bjo  Re-structured for use with shared device-support
 *
 *------------------------------------------------------------------------
 * MODULE DESCRIPTION:
 *
 * This header file defines the interface between the device-support and
 * the driver-support layers for Allen-Bradley DF-1 Protocol devices.
 *
 * The interface consists mainly of two structures and two function tables.
 * The structures defined here are:
 *
 * o abDf1Value  - The union of the various element types that it is
 *                 possible to return.
 *
 * o abDf1ElemIO - The DF-1 Element IO structure.  This structure contains
 *                 the information neccessary to address single elements
 *                 in the Allen-Bradley file structure.  This structure
 *                 contains pointers to the appropriate device and driver
 *                 function tables described below.
 *
 * Device-Support Callback Function Table:
 * --------------------------------------
 * This structure contains pointers to three functions that are supplied by
 * the device-support layer.  The specific functions will vary depending on
 * the record type (ai, ao, bi, bo, mbbi, mbbo, etc.) and are called by the
 * driver layer to get current record values, to asynchronously complete
 * write operations, and to notify output records of state changes that took
 * place independantly.  The three routines in the Device-Support Callback
 * Function Table are:
 *
 * o WriteCompletion   - Called when a write operation completes. The value
 *                       written is returned by the driver so that the cache
 *                       can be updated.
 *
 * o NewCacheValue     - Called when cache is updated and when going in and out
 *                       of alarm state (i.e. when device availability changes)
 *
 * o CurrentWriteValue - Fetches current raw write value from device support.
 *                       Called at various times during asynchronous write
 *                       IO operations.  Note that data type in the value
 *                       union must match the data type flag in the element
 *                       io structure.
 *
 * Driver-Support Function Table:
 * -----------------------------
 * This structure is provided by the individual device drivers.  It contains
 * the routines that the device-support layer calls to actually read or write
 * the data.  The device-support layer determines which driver to use from the
 * hardware address specified in the record.  The routines found in the
 * Driver-Support Function Table are:
 *
 * o NewElemIO      - Allocate and clear an element IO structure. Driver is
 *                embed the element IO structure within a
 *                larger driver specific structure.
 *
 * o SetupIO       - Called by the device support "Init Record" routine to
 *                   parse the hardware address information and initialize
 *                   the element IO structure.  Note that the last argument
 *                   should be NULL if this is not bit field IO.
 *
 * o SetupScan     - Called from the device support "Get I/O Interrupt" routine
 *                   to inform the driver when a record is being placed into
 *                   or taken out of "I/O Intr" scan mode.  This routine is
 *                   also called for all output records so that they will
 *                   reflect external changes.
 *
 * o InitiateAll   - Called from the device support initialization routine
 *                   after all calls to "SetupIO ()" (see above) have been
 *                   made. This initiates the DF-1 links, start the scan,
 *                   and begins sending write cache updates.
 *
 * o InitiateWrite - Initiates a write operation.  Integer words, bit strings,
 *                   and floating point currently supported.  Uses the
 *                   "CurrentWriteValue" and "WriteCompletion" call-back
 *                   routines.
 *
 * o ReadWord      - Reads current value from the cache.  Returns unsigned
 *                   word value (epicsUInt16).  Note that the data type in the
 *                   value union should match the data type flag in the element
 *                   io structure.
 * 
 * o ReadBitString - Reads current value from the cache.  Returns bit string.
 *                   Note that the data type in the value union should match
 *                   the data type flag in the element io structure.
 *
 * o ReadReal      - Reads current value from the cache.  Returns IEEE single-
 *                   precision floating point value.  Note that the data type
 *                   in the value union should match the data type flag in the
 *                   element io structure.
 *
 *------------------------------------------------------------------------
 * NOTES:
 *
 * o Output transactions occur on demand. Input transactions occur
 *   at a rate determined by the driver because it is more efficient
 *   to scan multiple input signals in one DF1 block IO transaction.
 *   Because of this the asynch IO callback is called for outputs 
 *   when the associated transaction completes and is called for
 *   inputs unsolicited _every_ time that an DF1 block IO read 
 *   transaction completes.
 *
 * o When an output transaction completes, the output value is written to
 *   the IO cache.  Any record which reads that output value is then
 *   processed.
 *
 * o The two notes above are not completely true for the VME-based PLC-5/V
 *   PLC's, which use the "Copy-To-VME" function to update the local cache
 *   at the end of every PLC scan. 
 *
 *************************************************************************/


#ifndef INCdrvAbDf1h
#define INCdrvAbDf1h

/*****************************************************************************/
/*  Other Files Included By this Header                                      */
/*****************************************************************************/

#include        <epicsTypes.h> /* Architecture-independent type definitions  */
#include        <epicsEvent.h>
#include        <bucketLib.h>
#include        <dbScan.h>     /* EPICS database scan definitions            */
#include        <errMdef.h>    /* EPICS error message module definitions     */
#include        <link.h>       /* EPICS Database link structure definitions  */
#include        <drvSerial.h>

#include        "df1.h"

/*****************************************************************************/
/*  Type Definitions                                                         */
/*****************************************************************************/

/*
 * Forward References to Defined Structures
 */
typedef struct  abDf1ElemIO             abDf1ElemIO;
typedef union   abDf1Value              abDf1Value; 
typedef struct  devAbDf1FuncTable       devAbDf1FuncTable;
typedef struct  drvAbDf1FuncTable       drvAbDf1FuncTable;
typedef struct  drvAbDf1Parm            drvAbDf1Parm;

/*
 * Function Type Declarations
 */
typedef void (*devAbDf1WriteCompletionFunc) (abDf1ElemIO*, epicsInt32);
typedef void (*devAbDf1NewCacheValueFunc) (abDf1ElemIO *);
typedef void (*devAbDf1CurrentWriteValueFunc) (abDf1ElemIO*, abDf1Value*);
typedef epicsInt32 (*devAbDf1ParseAddressFunc) (const char *address, 
                                                 int *fileType, 
                                                 int *dataType, 
                                                 int *fileNumber, 
                                                 int *element, 
                                                 int *subelement, 
                                                 int *bitNum, 
                                                 int *elementSize, 
                                                 int *structured);

/* 
 * drvAbDf1NewElemIOFunc must initialize (or at least clear) 
 * all bytes in the the allocated element io structure 
 * (this allows the driver to add a driver specific
 * preamble to the element IO structure)
 */
typedef abDf1ElemIO *(*drvAbDf1NewElemIOFunc) (void); 
typedef void (*drvAbDf1InitiateAllFunc) (void);
typedef epicsInt32 (*drvAbDf1SetupIOFunc) (const struct link *, abDf1ElemIO*, unsigned *pBitNo);
typedef void (*drvAbDf1SetupScanFunc) (const int cmd, abDf1ElemIO *);
typedef epicsInt32 (*drvAbDf1InitiateWriteFunc) (abDf1ElemIO *);
typedef epicsInt32 (*drvAbDf1ReadWordFunc) (abDf1ElemIO *, epicsUInt16 *pVal);
typedef epicsInt32 (*drvAbDf1ReadBitStringFunc) (abDf1ElemIO*, epicsUInt16 mask, epicsUInt16 *pVal);
typedef epicsInt32 (*drvAbDf1ReadRealFunc) (abDf1ElemIO *, float *pVal);
typedef int (*drvAbDf1InputFunc)(FILE *, drvSerialResponse *, drvAbDf1Parm *);

/*
 * Other Type Definitions
 */
typedef void *drvAbDf1CacheId;  /* Pointer to cache structure                */

#define abDf1IntScanStart  0    /* Add record to I/O Interrupt scan list     */
#define abDf1IntScanStop   1    /* Remove record from I/O Interrupt scan list*/


/****************************************************************************/
/*  Structure Declarations                                                  */
/****************************************************************************/

/*
 * Device-Support Function Table
 */
struct devAbDf1FuncTable {
   devAbDf1WriteCompletionFunc   pWriteCompletion;
   devAbDf1NewCacheValueFunc     pNewCacheValue;
   devAbDf1CurrentWriteValueFunc pCurrentWriteValue;
   devAbDf1ParseAddressFunc      pParseAddress;
};

/*
 * Driver-Support Function Table
 */
struct drvAbDf1FuncTable {
   drvAbDf1NewElemIOFunc       NewElemIO;      /* allocate and clear elem IO structure */
   drvAbDf1InitiateAllFunc     InitiateAll;    /* Initiate All Links        */
   drvAbDf1SetupIOFunc         SetupIO;        /* Setup Element IO Struct   */
   drvAbDf1SetupScanFunc       SetupScan;      /* Setup "Scan on Interrupt" */
   drvAbDf1InitiateWriteFunc   InitiateWrite;  /* Start a write operation   */
   drvAbDf1ReadWordFunc        ReadWord;       /* Get 16-bit raw value      */
   drvAbDf1ReadBitStringFunc   ReadBitString;  /* Get bit-string value      */
   drvAbDf1ReadRealFunc        ReadReal;       /* Get IEEE float value      */
};

/*
 * Structure for Holding Allen-Bradley DF-1 Element Values.
 */
union abDf1Value{
   epicsUInt16      word;    /* 16-bit raw value          */
   struct {               /* 16-bit bit string & mask  */
      epicsUInt16   value; 
      epicsUInt16   mask; 
   } bitString;
   float         real;    /* Single-Precision IEEE floating point value */
}; 


/*
 * Allen-Bradley DF-1 Element IO Descriptor 
 * (device private structure for Allen-Bradley DF-1 records)
 *
 * all fields are initialized by device support
 */
struct abDf1ElemIO {

   drvAbDf1FuncTable  *drvFunc;         /* Addr of driver function table (init by dev support)          */
   devAbDf1FuncTable  *devFunc;         /* Addr of dev sup func table (init by dev support)             */
   void               *pRec;            /* Addr of record (init by dev support)                         */
   IOSCANPVT           ioScanPvt;       /* IO event for asynch record processing (pvt to dev support)   */
   epicsUInt8          dataType;        /* Data type of PV (init by drv support)                        */
};

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
   epicsMutexId mutex;
   epicsEventId ackEvt;
   epicsEventId scanEvt;
   drvSerialLinkId id;
   BUCKET *pTransBucket;
   ELLLIST *pInitOutIO;
   ELLLIST *pLocalHashList;
   void *pTransFreeListPVT;
   devAbDf1ParseAddressFunc pParseAddress;
   drvAbDf1InputFunc pReadCharFunc;
   drvAbDf1InputFunc inHandlerList[df1dlLAST+1u];
   epicsUInt32 smoothedDelayToResp;
   epicsUInt32 maxDelayToResp;
   epicsUInt32 smoothedDelayToReadSend;
   epicsUInt32 maxDelayToReadSend;
   epicsUInt32 smoothedDelayToWriteSend;
   epicsUInt32 maxDelayToWriteSend;
   epicsThreadId  taskId;
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
   epicsUInt16 nextTransId; /* must be an unsugned 16 bit integer */
   epicsUInt8 lastAckSent;
   epicsUInt8 nextAckSent;
   epicsUInt8 nodeNo; /* node number */
   unsigned ackRecv:1;
   unsigned negAckRecv:1;
   unsigned scanInit:1;
};

extern drvAbDf1FuncTable drvAbDf1Func;

/*****************************************************************************/
/*  Driver Message Code Definitions                                          */
/*****************************************************************************/

#ifndef M_drvAbDf1
#define M_drvAbDf1 (603 << 16u)
#endif

#define S_drvAbDf1_OK 0
#define S_drvAbDf1_badParam             (M_drvAbDf1 | 1)  /* bad parameter */
#define S_drvAbDf1_noInit               (M_drvAbDf1 | 2)  /* AB DF1 driver has not been initialized */
#define S_drvAbDf1_linkDown             (M_drvAbDf1 | 3)  /* serial link is down */
#define S_drvAbDf1_noMemory             (M_drvAbDf1 | 4)  /* out of dynamic memory*/
#define S_drvAbDf1_badFrame             (M_drvAbDf1 | 5)  /* corrupt input frame */
#define S_drvAbDf1_mutexTMO             (M_drvAbDf1 | 6)  /* timed out waiting for mutex lock  */
#define S_drvAbDf1_notSubscribed        (M_drvAbDf1 | 9)  /* data point has not been subscribed */
#define S_drvAbDf1_badNodeNumber        (M_drvAbDf1 | 10) /* invalid node number */
#define S_drvAbDf1_xmitTMO              (M_drvAbDf1 | 11) /* frame timed out in request queue */
#define S_drvAbDf1_badAddrType          (M_drvAbDf1 | 12) /* INSTIO adressing required */
#define S_drvAbDf1_udf                  (M_drvAbDf1 | 13) /* undefined data (first scan has not occurred) */
#define S_drvAbDf1_respTMO              (M_drvAbDf1 | 14) /* response to DF1 request timed out */
#define S_drvAbDf1_noBuf                (M_drvAbDf1 | 15) /* no buffer space to queue request */
#define S_drvAbDf1_negAckTMO            (M_drvAbDf1 | 16) /* excessive neg acks when transmitting frame */
#define S_drvAbDf1_ackTMO               (M_drvAbDf1 | 17) /* remote node does not ack frame */
#define S_drvAbDf1_asyncCompletion      (M_drvAbDf1 | 18) /* operation will complete asynchronously */
#define S_drvAbDf1_badTransId           (M_drvAbDf1 | 19) /* transaction does not exist */
#define S_drvAbDf1_noDriver             (M_drvAbDf1 | 20) /* required df-1 driver is not loaded */
#define S_drvAbDf1_badType              (M_drvAbDf1 | 21) /* invalid file character in address specification */
#define S_drvAbDf1_wrongType            (M_drvAbDf1 | 22) /* requested data type does not match data file type */
#define S_drvAbDf1_badFile              (M_drvAbDf1 | 23) /* invalid file number in address specification */
#define S_drvAbDf1_noFile               (M_drvAbDf1 | 24) /* Requested PLC file number does not exist */
#define S_drvAbDf1_badElement           (M_drvAbDf1 | 25) /* invalid element number in address specification */
#define S_drvAbDf1_badSubelement        (M_drvAbDf1 | 26) /* invalid sub-element or bit number in address specification */
#define S_drvAbDf1_addrGarbage          (M_drvAbDf1 | 27) /* undecipherable Allen-Bradley address specification */
#define S_drvAbDf1_unrecSubelement      (M_drvAbDf1 | 28) /* invalid symbolic subelement for this file type */
#define S_drvAbDf1_badBitNumber         (M_drvAbDf1 | 29) /* invalid bit number */
#define S_drvAbDf1_archConflict         (M_drvAbDf1 | 30) /* host/compiler data architecture does not match AB type */
#define S_drvAbDf1_uknDataType          (M_drvAbDf1 | 31) /* no support for requested data type */

#endif /* INCdrvAbDf1h */
