/******************************************************************************
 * df1.h -- Allen-Bradley DF1 Protocol Definition File
 *
 *-----------------------------------------------------------------------------
 * Authors: Eric Bjorklund, Jeff Hill
 * Date:    29 October 1997
 *-----------------------------------------------------------------------------
 * MODIFICATION LOG:
 * 
 * 08-Jun-2016   mdw       Moved data structure instantiations
 *                         to a separate file (df1.c)
 * 06-Jun-2016   mdw       Modified to use <epicsTypes.h>, 
 *                         removed #include <vxWorks.h>
 * 29-Oct-97     bjo,joh   Original Release 
 *
 *-----------------------------------------------------------------------------
 * MODULE DESCRIPTION:
 *
 *      This module contains protocol definitions and EPICS status codes
 *      for the Allen-Bradley DF-1 message protocol.
 *
 *****************************************************************************/

#ifndef INCdf1h
#define INCdf1h

#include <epicsTypes.h>
#include <epicsEndian.h>
#include <errMdef.h>    /* EPICS Module Code definitions                */

#if 0
#include <vxWorks.h>    /* Standard vxWorks Symbols                     */
#endif

/************************************************************************/
/*  EPICS Error Status Codes                                            */
/************************************************************************/

/* 
 * Assign default values to M_df1 and M_df1e if they are not in errMdef.h 
 */
#ifndef M_df1
#define M_df1 (601 << 16)
#endif

#ifndef M_df1e
#define M_df1e (602 << 16)
#endif

/*----------------------------------------------------------------------*/
/*  Error Codes From The Status Byte Of The Reply Message               */
/*----------------------------------------------------------------------*/

/*
 * Link-Level Error Codes
 */
#define S_df1_noBuf             (M_df1 | 0x01)  /* destination node out of buffer space */
#define S_df1_noACK             (M_df1 | 0x02)  /* remote node does not acknowledge command */
#define S_df1_dupToken          (M_df1 | 0x03)  /* duplicate token holder detected */
#define S_df1_lclDisconn        (M_df1 | 0x04)  /* local port is disconnected */

/*
 * Application-Level Error Codes
 */
#define S_df1_IllCmd            (M_df1 | 0x10)  /* Illegal command or format */
#define S_df1_HostComErr        (M_df1 | 0x20)  /* Host has problem and will not communicate */
#define S_df1_HostGone          (M_df1 | 0x30)  /* Remote node host is missing, disconnected, or shut down */
#define S_df1_HwErr             (M_df1 | 0x40)  /* Host could not complete function due to hardware fault */
#define S_df1_InvAddr           (M_df1 | 0x50)  /* Addressing problem or memory protect rungs */
#define S_df1_FuncProt          (M_df1 | 0x60)  /* Function disallowed due to command protection selection */
#define S_df1_PgmMode           (M_df1 | 0x70)  /* Processor is in program mode */
#define S_df1_NoCmodeFile       (M_df1 | 0x80)  /* Compat mode file missing or communication zone problem */
#define S_df1_CantBuffer        (M_df1 | 0x90)  /* Remote node cannot buffer command */
#define S_df1_DownLoadErr       (M_df1 | 0xb0)  /* Remote node problem due to download */
#define S_df1_ActiveIPBs        (M_df1 | 0xc0)  /* Cannot execute command due to active IPBs */
#define S_df1_STX               (M_df1 | 0xf0)  /* Detailed status follows in STX byte */
#define S_df1_UnrecReplStat     (M_df1 | 0xff)  /* Unrecognized reply status value */

/*
 * Status field masks
 */
#define df1RemoteMask           0xf0            /* part of the STS code reserved for application */
#define df1LocalMask            0x0f            /* part of the STS code reserved for link layer */


/*----------------------------------------------------------------------*/
/* Error Codes From The Extended Status Byte Of The Reply Message       */
/*----------------------------------------------------------------------*/

#define S_df1_IllField          (M_df1e | 0x01) /* Field in DF1 packet has an illegal value */
#define S_df1_LoAddrLvl         (M_df1e | 0x02) /* Too few levels specified for a system address */
#define S_df1_HiAddrLvl         (M_df1e | 0x03) /* Too many levels specified for a system address */
#define S_df1_SymNotFound       (M_df1e | 0x04) /* Symbol not found */
#define S_df1_BadSymFmt         (M_df1e | 0x05) /* Symbol has improper format */
#define S_df1_BadAddr           (M_df1e | 0x06) /* System Address does not point to anything useable */
#define S_df1_EOF               (M_df1e | 0x07) /* Attempt to address past end of file */
#define S_df1_EnvChanged        (M_df1e | 0x08) /* Cannot complete request, situation changed since start */
#define S_df1_DataTooBig        (M_df1e | 0x09) /* Data or file is too large */
#define S_df1_TransTooBig       (M_df1e | 0x0a) /* Transaction size plus word address is too large */
#define S_df1_NoPriv            (M_df1e | 0x0b) /* Access denied, improper privilege */
#define S_df1_NoResource        (M_df1e | 0x0c) /* Condition cannot be generated, resource is not available */
#define S_df1_CondExists        (M_df1e | 0x0d) /* Condition already exists, resource is already available */
#define S_df1_CantExecute       (M_df1e | 0x0e) /* DF1 command cannot be executed */
#define S_df1_HistOvfl          (M_df1e | 0x0f) /* Histogram overflow */
#define S_df1_NoAccess          (M_df1e | 0x10) /* No Access */
#define S_df1_BadType           (M_df1e | 0x11) /* Illegal data type */
#define S_df1_BadParm           (M_df1e | 0x12) /* Invalid parameter or invalid data */
#define S_df1_AddrDeleted       (M_df1e | 0x13) /* Address reference exists to deleted area */
#define S_df1_Unknown           (M_df1e | 0x14) /* DF1 command execution failed for unknown reason */
#define S_df1_ConvertErr        (M_df1e | 0x15) /* Data conversion error */
#define S_df1_ScannerComErr     (M_df1e | 0x16) /* Scanner cannot communicate with 1771 rack adapter */
#define S_df1_AdapterComErr     (M_df1e | 0x17) /* Adapter cannot communicate with module */
#define S_df1_Bad1771Resp       (M_df1e | 0x18) /* 1771 module resonse was not valid */
#define S_df1_DupLabel          (M_df1e | 0x19) /* Duplicate Label */
#define S_df1_FileOwned         (M_df1e | 0x1a) /* File is open; another node owns it */
#define S_df1_ProgOwned         (M_df1e | 0x1b) /* Another node is the program owner */
#define S_df1_UnrecExtStat      (M_df1e | 0xff) /* Unrecognized value in extended status byte */


/************************************************************************/
/* Command Code Definitions                                             */
/************************************************************************/

#define df1CmdProtectedBlockWrite       0x00u   /* Block Write, Limited (All PLCs)      */
#define df1CmdBlockRead                 0x01u   /* Block Read (All PLCs)                */
#define df1CmdProtectedBitWrite         0x02u   /* Bit-Masked Write, Limited (All PLCs) */
#define df1CmdPhysicalWrite             0x03u   /* Physical Write (PLC-2 & 1774)        */
#define df1CmdPhysicalRead              0x04u   /* Physical Read (PLC-2 & 1774)         */
#define df1CmdBitWrite                  0x05u   /* Bit-Masked Write, All Areas(All PLCs)*/
#define df1CmdDiagnostic                0x06u   /* Diagnostic Commands (All PLCs)       */
#define df1CmdMode                      0x07u   /* Mode Enable/Disable (PLC-2 & 1774)   */
#define df1CmdBlockWrite                0x08u   /* Block Write, All Areas (All PLCs)    */
#define df1CmdPlc4General               0x0eu   /* General I/O Command (PLC-4)          */
#define df1CmdGeneral                   0x0fu   /* General I/O Command (PLC-3 & PLC-5)  */

/*
 * Command field masks
 */
#define df1CmdMask                      0x0f    /* Command part of cmd field */
#define df1RespMask                     0x40    /* Response indication */

/************************************************************************/
/* Function Codes Associated With The "Diagnostic" Command Code (0x06)  */
/************************************************************************/

#define df1FncEcho                      0x00u   /* Diagnostic Loop-Back Test            */
#define df1FncDiagRead                  0x01u   /* Read Diagnostic Counters             */
#define df1FncSetVariables              0x02u   /* Set ENQs, NAKs, and Timeout at once  */
#define df1FncReadStatus                0x03u   /* Read Diagnostic Status File          */
#define df1FncSetTimeout                0x04u   /* Set Maximum Time To Wait For ACK     */
#define df1FncSetNAKs                   0x05u   /* Set Maximum NAKs Accepted per Message*/
#define df1FncSetENQs                   0x06u   /* Set Maximum ENQs Sent per Message    */
#define df1FncDiagReset                 0x07u   /* Reset Diagnostic Counters            */
#define df1FncSetDataTableSize          0x08u   /* Set Data Table Size (PLC-2 Only)     */

/************************************************************************/
/* Function Codes Associated With The "Mode" Command Code (0x07)        */
/************************************************************************/

#define df1FncDisableOutputs            0x00u   /* Disable 1774 Outputs                 */
#define df1FncEnableOutputs             0x01u   /* Enable 1774 Outputs                  */
#define df1FncEnableScan                0x03u   /* Restart 1774 Scan                    */
#define df1FncDownLoadMode              0x04u   /* Put PLC-2 in Download Mode           */
#define df1FncExitLoadMode              0x05u   /* Take PLC-2 Out of Down/Upload Mode   */
#define df1FncUpLoadMode                0x06u   /* Put PLC-2 in Upload Mode             */

/************************************************************************/
/* Functions Associated With The PLC-4 "General I/O Command Code (0x0E) */
/************************************************************************/

#define df1FncSetProgramMode            0x01u   /* Set PLC-4 to Program Load Mode       */
#define df1FncSetRunMode                0x02u   /* Set PLC-4 to Run Mode                */
#define df1FncSetTestMode               0x03u   /* Set PLC-4 to Test Mode               */
#define df1FncSingleStepMode            0x04u   /* Set PLC-4 to Single Scan Test Mode   */
#define df1FncAllocate                  0x05u   /* Allocate the PLC-4 Processor         */
#define df1FncDeallocate                0x06u   /* Release Ctrl of the PLC-4 Processor  */
#define df1FncInit                      0x0cu   /* Clear Memory of PLC-4 Processor      */
#define df1FncPlc4PhysicalRead          0x0du   /* PLC-4 Physical Read                  */
#define df1FncPlc4PhysicalWrite         0x0eu   /* PLC-4 Physical Write                 */
#define df1FncPhysicalWriteMask         0x0fu   /* Bit-Masked Physical Write            */

/************************************************************************/
/* Functions Associated With The PLC-3 "General I/O Command Code (0x0F) */
/************************************************************************/

#define df1FncWordRangeWrite            0x00u   /* Write Range of Words (PLC-3 & PLC-5) */
#define df1FncWordRangeRead             0x01u   /* Read Range of Words (PLC-3 & PLC-5)  */
#define df1FncBitWriteWord              0x02u   /* Modify Bit In Single Word            */
#define df1FncFileWrite                 0x03u   /* Write a Complete File                */
#define df1FncFileRead                  0x04u   /* Read a Complete File                 */
#define df1FncDownLoadReq               0x05u   /* Request Download Privilege           */
#define df1FncUpLoadReq                 0x06u   /* Request Upload Privilege             */
#define df1FncShutdown                  0x07u   /* Initiate Shutdown / File Alloc Freeze*/
#define df1FncPlc3PhysicalWrite         0x08u   /* Write to PLC-3 Physical Address      */
#define df1FncPlc3PhysicalRead          0x09u   /* Read from PLC-3 Physical Address     */
#define df1FncRestart                   0x0au   /* Terminate Down/Upload Operation      */

/************************************************************************/
/* Functions Associated With The PLC-5 "General I/O Command Code (0x0F) */
/************************************************************************/

#define df1FncGetEdit                   0x11u   /* Get Editing Resource                 */
#define df1FncReturnEdit                0x12u   /* Return Editing Resource              */
#define df1FncPlc5PhysicalRead          0x17u   /* Read Bytes from Physical Address     */
#define df1FncPlc5PhysicalWrite         0x18u   /* Write Bytes to Physical Address      */
#define df1FncWriteBit                  0x26u   /* Read/Modify/Write Specified Bits     */
#define df1FncSetMode                   0x3au   /* Set PLC-5 Mode                       */
#define df1FncStartDownLoad             0x50u   /* Put PLC-5 in "Download" Mode         */
#define df1FncDownLoadComplete          0x52u   /* Indicate PLC-5 Download is Complete  */
#define df1FncStartUpLoad               0x53u   /* Put in "Upload" Mode & Read Mem Segs */
#define df1FncUpLoadComplete            0x55u   /* Indicate PLC-5 Upload is Complete    */
#define df1FncTypedWrite                0x67u   /* Write Typed Data (Check File Type)   */
#define df1FncTypedRead                 0x68u   /* Read Data Words & Type Information   */
#define df1FncApplyPortConfig           0x8fu   /* Apply Port Configuration             */
#define df1FncRestPortConfig            0x90u   /* Restore Port Configuration           */


/************************************************************************/
/* DF-1 PLC-5 Typed Data Format Byte                                    */
/************************************************************************/

/*
 * Structure for typed data format byte
 */
#if EPICS_BYTE_ORDER == EPICS_BIG_ENDIAN
typedef struct /* tdfByte */ {
   unsigned int  typeFollows:1;         /* True if separate type byte(s)*/
   unsigned int  type:3;                /* Type code for data           */
   unsigned int  sizeFollows:1;         /* True if separate size byte(s)*/
   unsigned int  size:3;                /* Size of data element         */
} tdfByte;

#else/* EPICS_BYTE_ORDER == EPICS_LITTLE_ENDIAN */
typedef struct /* tdfByte */ {
   unsigned int  size:3;                /* Size of data element         */
   unsigned int  sizeFollows:1;         /* True if separate size byte(s)*/
   unsigned int  type:3;                /* Type code for data           */
   unsigned int  typeFollows:1;         /* True if separate type byte(s)*/
} tdfByte;
#endif

/*
 * Union of raw and formatted type byte
 */
typedef union {
   tdfByte  bf;
   epicsUInt8  raw;
} tdfUnion;

/*
 * Masks for individual fields in the typed data byte
 */
#define tdfTypeFollowsMask      0x80u   /* True if separate type byte(s)*/
#define tdfTypeMask             0x70u   /* Type code for data           */
#define tdfSizeFollowsMask      0x08u   /* True if separate size byte(s)*/
#define tdfSizeMask             0x07u   /* Size of data element         */

/*
 * Masks for single field in typed data byte
 */
#define tdfOneByteFollows       0x9u    /* One byte follows this field  */
#define tdfBytesFollow          0x8u    /* One or more bytes follow     */
#define tdfMaxEmbedded          0x7u    /* Max number of following bytes*/
#define tdfFieldWidth           0x4u    /* Width of a field             */
#define tdfFieldMask            0xfu    /* Field value mask             */

/*
 * DF-1 Data Type Codes
 */
#define df1DTNone                0u     /* Last Element Indicator       */
#define df1DTBit                 1u     /* Single Bit                   */
#define df1DTBitStr              2u     /* Bit String                   */
#define df1DTByteStr             3u     /* Byte or ASCII Char String    */
#define df1DTInt                 4u     /* Integer (2 bytes)            */
#define df1DTTmr                 5u     /* Timer (10 bytes)             */
#define df1DTCtr                 6u     /* Counter (6 bytes)            */
#define df1DTGCS                 7u     /* General Control Structure    */
#define df1DTFP                  8u     /* IEEE Single Precision Float  */
#define df1DTArray               9u     /* Array of Elements            */
#define df1DTPtr                15u     /* Address                      */
#define df1DTBCD                16u     /* Binary Coded Decimal         */
#define df1DTPid                21u     /* PID Structure (164 bytes)    */
#define df1DTMsg                22u     /* Message Structure (112 bytes)*/
#define df1DTSFCStat            29u     /* SFC Status Block (6 bytes)   */
#define df1DTBT                 32u     /* Block Transfer Control Block */

#define df1MaxDT                32u     /* Maximum type code            */

/************************************************************************/
/* DF-1 PLC-5 Logical Binary Addressing                                 */
/************************************************************************/

#define lbaMaskArea       0x1u  /* Level 1 - Area (default is data table)    */
#define lbaMaskFile       0x2u  /* Level 2 - File (default is file 1)        */
#define lbaMaskElement    0x4u  /* Level 3 - Element (default is zero)       */
#define lbaMaskSubElem    0x8u  /* Level 4 - Sub-Element (default is zero)   */
#define lbaTwoByteAddr   0xffu  /* Indicates that a two byte addr follows    */

#define lbaDefaultAreaNo    0u  /* Default area number if not specified      */
#define lbaDefaultFileNo    0u  /* Default file number if not specified      */
#define lbaDefaultElemNo    0u  /* Default element number if not specified   */
#define lbaDefaultSubElemNo 0u  /* Default sub-element num if not specified  */

/*
 * Allen Bradley DF1 data link layer protocol codes
 */
#define df1dlNONE 0x00  
#define df1dlSTX  0x02
#define df1dlETX  0x03
#define df1dlENQ  0x05
#define df1dlACK  0x06
#define df1dlDLE  0x10
#define df1dlNAK  0x15
#define df1dlLAST df1dlNAK /* used when an array with entries for each is allocated */

#define df1dlMaxFrameLen 250u
#define df1dlMinFrameLen 6u

#define df1MaxAddr 0xffffu

typedef struct {
      epicsUInt8 lsData;
      epicsUInt8 msData;
}df1Word;

typedef struct {
      epicsUInt8 data[4u];
}df1LongWord;

typedef struct {
      epicsUInt8 dst;
      epicsUInt8 src;
      epicsUInt8 cmd;
      epicsUInt8 sts;
      epicsUInt8 lsTns; 
      epicsUInt8 msTns; 
}abDf1ProtoHdr;

typedef struct {
      abDf1ProtoHdr hdr;
      struct{
            epicsUInt8 lsAddr;
            epicsUInt8 msAddr;
            epicsUInt8 setBits;
            epicsUInt8 clrBits;
      } bits[61];
}abDf1BitWriteReq;

#define abDf1BlockWriteDataSize 242u
typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 lsAddr;
      epicsUInt8 msAddr;
      union{
            epicsUInt8 bytes[abDf1BlockWriteDataSize];
            df1Word words[abDf1BlockWriteDataSize/sizeof(df1Word)];
      }data;
}abDf1BlockWriteReq;

typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 lsAddr;
      epicsUInt8 msAddr;
      epicsUInt8 byteCount;
}abDf1BlockReadReq;

typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 fnc;
      union {
            epicsUInt8 bytes[243];
      }data;
}abDf1GeneralReq;

typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 fnc;
      epicsUInt8 lsPO;
      epicsUInt8 msPO;
      epicsUInt8 lsTT;
      epicsUInt8 msTT;
      union {
            epicsUInt8 bytes[239];
      }data;
}abDf1TypedReadReq, abDf1TypedWriteReq, 
      abDf1FileBlockReadReq, abDf1FileBlockWriteReq;

typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 fnc;
      union {
            epicsUInt8 bytes[243];
      }data;
}abDf1TypedBitWriteReq;

typedef union {
      abDf1ProtoHdr hdr;
      abDf1BitWriteReq bitWrite;
      abDf1BlockWriteReq blockWrite;
      abDf1BlockReadReq blockRead;
      abDf1GeneralReq general;
      abDf1TypedWriteReq typedWrite;
      abDf1TypedReadReq typedRead;
      abDf1TypedBitWriteReq typedBitWrite;
      abDf1FileBlockReadReq fileBlockRead;
      abDf1FileBlockWriteReq fileBlockWrite;
      epicsUInt8 buf[df1dlMaxFrameLen+1u];
}abDf1ReqProto;

#define abDf1BlockReadDataSize 244u
typedef struct {
      abDf1ProtoHdr hdr;
      union{
            epicsUInt8 bytes[abDf1BlockReadDataSize];
            df1Word words[abDf1BlockReadDataSize/sizeof(df1Word)];
      }data;
}abDf1BlockReadRes, abDf1FileBlockReadRes;

#define abDf1TypedReadDataSize 240u /* allows 4 bytes for type/data */
typedef struct {
      abDf1ProtoHdr hdr;
      union {
            epicsUInt8 bytes[244u];
            df1Word words[244u/sizeof(df1Word)];
      }data;
}abDf1TypedReadRes;

typedef struct {
      abDf1ProtoHdr hdr;
}abDf1BlockWriteRes, abDf1BitWriteRes, 
      abDf1TypedWriteRes, abDf1TypedBitWriteRes,
      abDf1FileBlockWriteRes;

typedef struct {
      abDf1ProtoHdr hdr;
      epicsUInt8 stx;
}abDf1ProtoHdrWithSTX;

typedef union {
      abDf1ProtoHdr hdr;
      abDf1ProtoHdrWithSTX hdrstx;
      abDf1BlockReadRes blockRead;
      abDf1BlockWriteRes blockWrite;
      abDf1BitWriteRes bitWrite;
      abDf1TypedWriteRes typedWrite;
      abDf1TypedBitWriteRes typedBitWrite;
      abDf1TypedReadRes typedRead;
      abDf1FileBlockWriteRes fileBlockWrite;
      abDf1FileBlockReadRes fileBlockRead;
      epicsUInt8 buf[df1dlMaxFrameLen+1];
}abDf1ResProto;

typedef union {
      abDf1ProtoHdr hdr;
      abDf1ResProto res;
      abDf1ReqProto req;
      epicsUInt8 buf[df1dlMaxFrameLen+1u];
}abDf1Proto;

/*---------------------
 * Symbols used for symbolic sub-element fields
 */
#define MAX_SYMBOL_LEN          7       /* Maximum length of symbol          */

/*---------------------
 * Symbolic sub-element definition structure
 */

typedef struct /* subelementSymbol */ {
   const char  *symbol;         /* Symbol name (last has nill here)      */
   char         bit;            /* Bit number of desired subelement      */
   char         nBits;           /* number of bits of desired subelement  */
} subelementBitSymbol;

/*
 * NOTE: these are stored so that the subelement (word number) is the
 * index
 */
typedef struct /* subelementSymbol */ {
   const char             *symbol;  /* Symbol name                          */
   const subelementBitSymbol *pBits;   /* Pointer to a list of bits            */
   unsigned                   type;    /* Sub-element type                     */
} subelementSymbol;


/*
 * table of info for each type
 *
 * NOTE: 
 * 1) number of subelements in pSubElem
 * can be derived from the size since the
 * sub element number is always a word address
 *
 * 2) pSubElem is nill when it isnt a 
 * structured type
 *
 */
typedef struct {
   const subelementSymbol   *pSubElem;      /* sub element table for structured types */
   const char            *pName;         /* ascii name of type */
   unsigned short         size;         /* size of the data type */
} abTypeInfo;

/*
 * conversion between ASCII AB data type character
 * and the corresponding data type code
 */
#ifdef DF1_SRC    
const abTypeInfo abTypeInfo_array[];
const unsigned char cToabDataType_array[];
#else
extern const abTypeInfo abTypeInfo_array[];
extern const unsigned char cToabDataType_array[];
#endif

#define cToabDataType(A) (cToabDataType_array[(epicsUInt8)(A)])
#define abDataSize(DT) (abTypeInfo_array[(DT)].size)
#define abDataStructure(DT) (abTypeInfo_array[(DT)].pSubElem?1:0)
#define abDataTypeToString(DT) (abTypeInfo_array[(DT)].pName)
#define abSubElemTable(DT) (abTypeInfo_array[(DT)].pSubElem)

#ifndef NELEMENTS
   #define NELEMENTS(array) (sizeof (array) / sizeof ((array) [0]))
#endif



#endif      /* INCdf1h */
