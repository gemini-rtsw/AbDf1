/* devAbDf1.c */
/* $Id: devAbDf1.c,v 1.3 2008/02/06 23:09:15 gemvx Exp $ */

/* devAbDf1.c - Allen Bradley DF1 Serial Protocol Device Support Routines */
/*
 *      Author:         Jeff Hill, Eric Bjorklund
 *
 * Modification Log:
 * -----------------
 * $Log: devAbDf1.c,v $
 *
 * Revision 2.0 2016/06/16 mdw
 * Refactored to be EPICS OSI compliant
 *
 * Revision 1.3  2008/02/06 23:09:15  gemvx
 * raise monitors so that channels will update in GEA
 *
 * Revision 1.2  2002/08/23 13:21:07  pedro
 * Upgraded the Allen-Bradley and drvSerial drivers with newer versions.
 *
 * Revision 1.7  1999/05/06 17:01:58  hill
 * added devAbDf1.h header file to improve doc
 *
 * Revision 1.6  1998/09/17 22:47:03  aptdvl
 * added support for signed 16 bit words in PLC
 *
 * Revision 1.5  1998/07/21 22:14:31  hill
 * added support for signed 16 bit integer file elements
 *
 * Revision 1.3  1997/12/19  18:36:00  bjo
 * Modifications to run with both serial interface (regular PLC5)
 * and VME interface (PLC-5/VME) drivers.
 *
 * Revision 1.2  1995/08/28  02:35:55  jhill
 * better expected address diagnostic
 *
 * Revision 1.1  1995/08/24  07:10:56  jhill
 * installed into cvs
 *
 * Fixed potential problem where ai/bi/mbbi go into
 * alarm state because a request times out, but dont reprocess 
 * out of alarm state when the link is restored because the value 
 * has not changed.
 */

/************************************************************************/
/*  Header Files                                                        */
/************************************************************************/
 
/*
 * ANSI C
 */
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <ctype.h>


#define OK        0
#define ERROR   (-1)

/*
 * EPICS
 */
#include    <epicsAssert.h>
#include    <alarm.h>
#include    <cvtTable.h>
#include    <dbDefs.h>
#include    <dbAccess.h>
#include    <dbEvent.h>
#include    <recSup.h>
#include    <devSup.h>
#include    <dbScan.h>
#include    <link.h>
#include    <menuConvert.h>
#include    <menuScan.h>
#include    <devLib.h>
#include    <recGbl.h>
#include    <epicsExport.h>
#include    <epicsPrint.h>

#include    "devAbDf1.h"

/************************************************************************/
/*  Other Symbol Definitions                                            */
/************************************************************************/

#define S_devAbDf1_OK 0
#define S_devAbDf1_dontConvert 2

#define devInitPassBeforeDevInitRec 0
#define devInitPassAfterDevInitRec 1

#define INPUT  1                /* Link is an input link */
#define OUTPUT 0                /* Link is an output link */


/*****************************************************************************/
/*  Define Function Dispatch Table Names/Locations for All Supported Drivers */
/*****************************************************************************/

#define MAX_DRIVERS          2  /* Number of drivers currently supported  */

#define SERIAL_DRIVER_INDEX  0  /* Table index for serial-port driver     */
#define PLC5V_DRIVER_INDEX   1  /* Table index for PLC-5/V driver         */

/*
 * Names of supported driver function tables
 */
#if 0
/* not needed if we don't have vxWorks symLib available */
LOCAL const char  *funcTableName [MAX_DRIVERS] = {
    "_drvAbDf1Func",            /* Driver for Serial Port DF-1 Interface  */
    "_drvPlc5vFunc"             /* Driver for PLC-5/V VME card            */
};


/*
 * Names of supported drivers
 */
LOCAL const char  *driverName [MAX_DRIVERS] = {
    "drvAbDf1.c",               /* Driver for Serial Port DF-1 Interface  */
    "drvPlc5v.c"                /* Driver for PLC-5/V VME card            */
};
#endif




/*
 *Pointers to supported driver function tables
 */
LOCAL drvAbDf1FuncTable  *funcTable [MAX_DRIVERS];


/************************************************************************/
/*  Function Type and Prototype Declarations                            */
/************************************************************************/

//LOCAL devInit          devInitAbDf1; /* moved to devAbDf1.h */
LOCAL devGetIoIntInfo  devGetIoIntInfoAbDf1;

// moved this to devAbDf1.h
//LOCAL epicsInt32       devAbDf1CommonInit (void*, const struct link*, int, abDf1ElemIO**, unsigned*);

/*
 * Allen-Bradley Address Parsing Routines
 */
LOCAL epicsInt32  devAbParseAddress(const char *, int*, int*, int*, int*, int*, int*, int*, int*);
LOCAL epicsInt32  devAbParseSymbol (const char*, int, int*, int*, int*); 

/*
 *   Analog Input Routines
 */
LOCAL aiDevInitRec       aiDevInitRecAbDf1;
LOCAL aiDevRead          aiDevReadSignedAbDf1;
LOCAL aiDevRead          aiDevReadUnsignedAbDf1;
LOCAL aiDevRead          aiDevReadFloatAbDf1;
LOCAL aiDevLinearConv    aiDevLinearConvertAbDf1_12;
LOCAL aiDevLinearConv    aiDevLinearConvertAbDf1_16;

LOCAL void aiDevAbDf1NewFloatValue(abDf1ElemIO *pIO);
LOCAL void  aiDevAbDf1NewWordValue(abDf1ElemIO *pIO);

/*
 *  Analog Output Routines
 */
LOCAL epicsInt32 aoDevInitRecAbDf1 (struct aoRecord *pao, int signedValue);
LOCAL aoDevInitRec       aoDevInitRecAbDf1Unsigned;
LOCAL aoDevInitRec       aoDevInitRecAbDf1Signed;
LOCAL aoDevWrite         aoDevWriteAbDf1;
LOCAL aoDevLinearConv    aoDevLinearConvertAbDf1_12;
LOCAL aoDevLinearConv    aoDevLinearConvertAbDf1_16;

LOCAL void aoDevAbDf1NewValue (struct aoRecord *pao, double value, epicsInt32 status);
LOCAL void aoDevAbDf1NewFloatValue(abDf1ElemIO *pIO);
LOCAL void aoDevAbDf1NewSignedValue(abDf1ElemIO *pIO);
LOCAL void aoDevAbDf1NewUnsignedValue(abDf1ElemIO *pIO);
LOCAL void aoDevAbDf1NewWordValue (abDf1ElemIO *pIO, int signedValue);
LOCAL void aoDevAbDf1WriteCompletion(abDf1ElemIO *pIO, epicsInt32 status);
LOCAL void aoDevAbDf1CurrentWordWriteVal(abDf1ElemIO *pIO, abDf1Value *pVal);
LOCAL void aoDevAbDf1CurrentFloatWriteVal(abDf1ElemIO *pIO, abDf1Value *pVal);

/*
 * Binary Input Routines
 */
LOCAL biDevInitRec       biDevInitRecAbDf1;
LOCAL biDevRead          biDevReadAbDf1;

LOCAL void biDevAbDf1NewValue(abDf1ElemIO *pIO);

/*
 * Binary Output Routines
 */
LOCAL boDevInitRec       boDevInitRecAbDf1;
LOCAL boDevWrite         boDevWriteAbDf1;

LOCAL void boDevAbDf1NewValue(abDf1ElemIO *pIO);
LOCAL void boDevAbDf1WriteCompletion(abDf1ElemIO *pIO, epicsInt32 status);
LOCAL void boDevAbDf1CurrentWriteVal(abDf1ElemIO *pIO, abDf1Value *pVal);

/*
 * Multi-bit Binary Input Routines
 */
LOCAL mbbiDevInitRec       mbbiDevInitRecAbDf1;
LOCAL mbbiDevRead          mbbiDevReadAbDf1;

LOCAL void  mbbiDevAbDf1NewValue(abDf1ElemIO *pIO);

/*
 * Multi-bit Binary Output Routines
 */
LOCAL mbboDevInitRec mbboDevInitRecAbDf1;
LOCAL mbboDevWrite mbboDevWriteAbDf1;

LOCAL void mbboDevAbDf1NewValue(abDf1ElemIO *pIO);
LOCAL void mbboDevAbDf1WriteCompletion(abDf1ElemIO *pIO, epicsInt32 status);
LOCAL void mbboDevAbDf1CurrentWriteVal(abDf1ElemIO *pIO, abDf1Value *pVal);

/*
 * Routines common to all input records
 */
//LOCAL devAbDf1CurrentWriteValueFunc  dummyCurrentWriteVal;
//LOCAL devAbDf1WriteCompletionFunc    dummyWriteCompletion;
//LOCAL devAbDf1NewCacheValueFunc      dummyNewValue;
LOCAL void  dummyCurrentWriteVal(abDf1ElemIO *pElem, abDf1Value *pVal);
LOCAL void  dummyWriteCompletion(abDf1ElemIO *pElem, epicsInt32 status);
LOCAL void  dummyNewValue(abDf1ElemIO *pIO);

LOCAL devAbDf1FuncTable devAbDf1BadTypeCallBacks = {
      dummyWriteCompletion,
      dummyNewValue,
      dummyCurrentWriteVal,
      devAbParseAddress
};


/*****************************************************************************/
/*              Routines for Parsing Allen-Bradley Addresses                 */
/*****************************************************************************/


/*****************************************************************************/
/* devAbParseSymbol () -- Parse a symbolic subelement specification          */
/*      o Some Allen-Bradley file types (such as counters and timers)        */
/*        contain data structures.  The subelements of these fields may be   */
/*        addressed symbolically as shown in the example below.              */
/*              T4:3.ACC        - Addresses the "Accumulated Value" field    */
/*                                of the third element in "Timer" file 4.    */
/*                                                                           */
/*      o This routine parses the symbolic sub-element with respect to the   */
/*        type of the file being addressed, and returns the appropriate      */
/*        subelement number and (in the case of a bit specification) the     */
/*        appropriate bit number within the sub-element.                     */
/*                                                                           */
/*****************************************************************************/

LOCAL epicsInt32 devAbParseSymbol (
   const char  *string,               /* Pointer to subelement string              */
   int          fileType,             /* Type of file the element is in            */
   int         *dataType,             /* Type of subelement                        */
   int         *subelement,           /* Returned subelement number                */
   int         *bitNum)               /* Returned bit number                       */
{

  /*---------------------
   * Local variables
   */
   unsigned char            c = *string++;      /* Next character to parse   */
   int                      i;                  /* Loop counter              */
   const subelementSymbol  *list;               /* List of valid symbols     */
   int                      numSymbols;         /* Number of symbols in list */
   char                     symbol [MAX_SYMBOL_LEN+1]; /* Local cpy of symbol*/

  /*---------------------
   * Copy the subelement symbol into local storage.
   * Abort if no symbol found.
   */
   for (i=0; ((i < MAX_SYMBOL_LEN) && isalnum(c)); c = *string++)
      symbol[i++] = toupper (c);

   if (!i) return S_drvAbDf1_badSubelement;
   symbol[i] = '\0';

  /*---------------------
   * Determine how many symbols are defined for this file type.
   * Abort if file type does not support symbolic addressing.
   */
   if ((unsigned int)fileType > df1MaxDT) return S_drvAbDf1_badType;

   /*--------------------
    * The number of sub elements is the byte size divided by two.
   */
   numSymbols = abDataSize(fileType)/2u;
   if (!numSymbols || !abDataStructure(fileType)) return S_drvAbDf1_unrecSubelement;

  /*---------------------
   * Search the list of valid subelement symbols for this file type.
   * If the symbol is found, return its subelement number and (if binary)
   * the bit number within the subelement.
   */
   list = abSubElemTable(fileType);
   for (i=0; i < numSymbols; i++) {
      const subelementBitSymbol *pBits = list[i].pBits;

      if (!strcmp(symbol, list[i].symbol)) {
         *dataType = list[i].type;
         *subelement = i;
         *bitNum = 0;
         return OK;
      }/*end if we found the symbol*/

     /*---------------------
      * Look for a bit string symbol that matches
      */
     if (pBits) {
        while (pBits->symbol) {
              if (!strcmp(symbol, pBits->symbol)) {
                  *dataType = df1DTBit;
                  *subelement = i;
                  *bitNum = pBits->bit;
                  return OK;
           }/*end if we found the bit symbol*/
           pBits++;
          }/*end for each subelement bit symbol for this subelement*/
     }
   }/*end for each subelement symbol valid for this file type*/

  /*---------------------
   * Error return, specified subelement symbol not found.
   */
   return S_drvAbDf1_unrecSubelement;

}/*end devAbParseSymbol()*/


/*****************************************************************************/
/* devAbParseAddress () -- Parse Allen-Bradley Addresses                     */
/*      o This routine may be called by individual device drivers (via the   */
/*        device-support callback tables) to parse an Allen-Bradley address  */
/*        specification.                                                     */
/*      o The routine takes an Allen-Bradley address string (e.g. B3:12/15)  */
/*        and returns the file number, the file type, and the element number */
/*        For binary files and for certain structure files (e.g. counters    */
/*        and timers), it will also return the subelement (if structured)    */
/*        the bit number within the element or subelement, and whether or    */
/*        not the file contains "structured" elements.                       */
/*                                                                           */
/*****************************************************************************/

LOCAL epicsInt32 devAbParseAddress (
   const char  *address,              /* String containing Allen-Bradley address   */
   int         *fileType,             /* Type of file specified (returned)         */
   int         *dataType,             /* Type of subelement (returned)             */
   int         *fileNumber,           /* File number specified (returned)          */
   int         *element,              /* Element number specified (returned)       */
   int         *subelement,           /* Sub-element (for structures) (returned)   */
   int         *bitNum,               /* Bit number                                */
   int         *elementSize,          /* Size (in bytes) of an element             */
   int         *structured)           /* TRUE if file contains structured elements */
{

  /*---------------------
   * Local variables
   */
   int    base = 10;            /* Base for numeric conversions (base 10)      */
   unsigned char   fileChar;    /* File character from address string         */
   int    noSubelement = TRUE;  /* True when no sub-element in address string   */
   int    predefined = FALSE;   /* True when predefined file is specified      */
   unsigned char  separator;            /* Current separator character               */
   char  *tail;                 /* Rest of address string after numeric parse   */

  /*---------------------
   * Set default return values
   */
   *element = 0;
   *subelement = 0;
   *bitNum = 0;
   *elementSize = 0;
   *structured = FALSE;

  /*---------------------
   * Find the file type character
   */
   for (fileChar = *address++; isspace(fileChar); fileChar = *address++);

  /*---------------------
   * Get the file type
   */
   *fileType = cToabDataType (fileChar);
   if (*fileType == df1DTNone) return S_drvAbDf1_badType;

  /*---------------------
   * Check for predefined files 'O', 'I', and 'S'
   */
   switch (toupper(fileChar)) {

   case 'O':    /* Output file (0) */
      *fileNumber = 0;
      predefined = TRUE;
      base = 8;
      break;

   case 'I':    /* Input file (1) */
      *fileNumber = 1;
      predefined = TRUE;
      base = 8;
      break;

  /*---------------------
   * Handle special cases for two-character file types.
   */

   case 'S':    /* Status file (2) or String (ST) or SFC Status (SC) */
      if (toupper((unsigned char)*address) == 'C') {
         address++;
         *fileType = df1DTSFCStat;
      }/*end if SFC Status*/

      else if (toupper((unsigned char)*address) == 'T') {
         address++;
         *fileType = df1DTByteStr;
      }/*end if block transfer control block*/

      else {
         *fileNumber = 2;
         predefined = TRUE;
       }/*end if status file*/

      break;

   case 'B':    /* Binary (B) or Block Xfr Control (BT) */
      if (toupper((unsigned char)*address) == 'T') {
         address++;
         *fileType = df1DTBT;
      }/*end if block transfer control block*/
      break;

   case 'M':    /* Message (MG) */
      if (toupper((unsigned char)*address) != 'G') return S_drvAbDf1_badType;
      address++;
      break;

   case 'P':    /* PID Block (PD) */
      if (toupper((unsigned char)*address) != 'D') return S_drvAbDf1_badType;
      address++;
      break;

   }/*end switch on file character*/

  /*---------------------
   * Set the default data type and determine whether or not the file elements
   * are structured.
   */
   *dataType = *fileType;
   *structured = abDataStructure(*fileType);

  /*---------------------
   * Parse the file number (if not predefined)
   */
   if (!predefined) {
      if (!isdigit((unsigned char)*address)) return S_drvAbDf1_badFile;
      *fileNumber = strtol (address, &tail, base);
      address = tail;
   }/*end if not a predefined file*/

  /*---------------------
   * Parse the element number
   */
   separator = *address++;
   if (!separator) return S_drvAbDf1_badElement;

   if (separator != ':') {
      if (predefined && isdigit(separator)) address--;
      else if ((separator != '/') || (*fileType != df1DTBit)) return S_drvAbDf1_addrGarbage;
   }/*end if separator character not a colon*/

   if (!isdigit((unsigned char)*address)) return S_drvAbDf1_badElement;
   *element = strtol (address, &tail, base);
   address = tail;

  /*---------------------
   * Get the size of the element based on the file type
   */
   *elementSize = abDataSize (*fileType);

  /*---------------------
   * Parse the sub-element number
   */
   separator = *address++;
   if (separator && !isspace(separator)) {

      /* Check for invalid separator */
      if ((separator != '/') && (separator != '.'))
         return S_drvAbDf1_addrGarbage;

      /* Check for symbolic subelement */
      if (!isdigit((unsigned char)*address))
         return devAbParseSymbol (address, *fileType, dataType, subelement, bitNum);

      /* Convert numeric subelement */
      *subelement = strtol (address, &tail, base);
      address = tail;
      noSubelement = FALSE;
      /* If this is not a structure, then the subelement is really the bit */
      if (!*structured) {
         *bitNum = *subelement;
         *subelement = 0;
         *dataType = df1DTBit;
      }/*end if this is a binary file*/

   }/*end if there was a subelement*/

  /*---------------------
   * Abort if there is garbage at the end of the address string.
   */
   if (separator && *address && !isspace((unsigned char)*address)) return S_drvAbDf1_addrGarbage;

  /*---------------------
   * Special case checking for default or implied subelements in the case
   * where a subelement was not specified.
   */
   if (noSubelement) {

     /*---------------------
      * Handle special case for binary files in which only the bit number
      * is specified in the address. In this situation the element is the word 
     * number and to remain consistent then the file is really of type int 
     * (16 bit words).
      */
      if (*fileType == df1DTBit) {
         *fileType = df1DTInt;
         *bitNum = *element & 0xf;
         *element >>= 4;
      }/*end if only bit number specified in binary file*/

     /*---------------------
     * Not specifying the sub element for a structured type is
     * classified as an invalid address
     */
      else if (*structured) {
       return S_drvAbDf1_unrecSubelement;
      }/*end else check for structured file type*/

   }/*end if subelement not specified*/

   return OK;

}/*end devAbParseAddress()*/


/*****************************************************************************/
/* devAbDf1CommonInit () -- Perform Common Record Initialization Tasks       */
/*      o Check the link type to see which driver should handle this record  */
/*      o Allocate and fill in the element-io structure (this will become    */
/*        the device-private structure for this record).                     */
/*      o Call the driver "SetupIO" routine to parse the address and add the */
/*        record to it's scan list.                                          */
/*                                                                           */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/*  Temporary call-back structure so that the drivers can locate the         */
/*  Allen-Bradley address parsing routine.                                   */
/*---------------------------------------------------------------------------*/

#if 0
LOCAL devAbDf1FuncTable initCallBacks = {
      dummyWriteCompletion,
      dummyNewValue,
      dummyCurrentWriteVal,
      devAbParseAddress
};
#else
//LOCAL devAbDf1FuncTable initCallBacks;
#endif

/*---------------------------------------------------------------------------*/
/*  Code for devAbDf1CommonInit ()                                           */
/*                                                                           */
/* 20180125 MDW Removed static class specifier so add-on device support      */
/*              modules can use this.                                        */
/*---------------------------------------------------------------------------*/

epicsInt32 devAbDf1CommonInit (
   void                *pRec,           /* Ptr to record structure           */
   const struct link   *pLink,          /* Ptr to record's i/o link field    */
   int                  input,          /* TRUE if this is an input record   */
   abDf1ElemIO        **ppIO,           /* Returned ptr to element I/O struct*/
   unsigned            *bitNo)          /* Returned bit number (or NULL)     */
{
  /*---------------------
   * Local variables
   */
   drvAbDf1FuncTable   *drvFunc = NULL; /* Address of driver function table  */
   abDf1ElemIO         *pIO     = NULL; /* Address of element-io structure   */
   int                  status  = OK;   /* Local status variable             */


#if 0
#else
   devAbDf1FuncTable initCallBacks = {
       dummyWriteCompletion,
       dummyNewValue,
       dummyCurrentWriteVal,
       devAbParseAddress
   };
#endif


  /*---------------------
   * Check the link type to locate the appropriate driver.
   */
   switch (pLink->type) {

   case INST_IO:  /* Serial DF-1 Driver - Use "Instrument I/O" */
      drvFunc = funcTable[SERIAL_DRIVER_INDEX];
      break;

   case VME_IO:  /* PLC-5/V Driver - Use "VME I/O" */
      drvFunc = funcTable[PLC5V_DRIVER_INDEX];
      break;

   default:  /* All other link types are invalid */
      status = input ? S_dev_badInpType : S_dev_badOutType;

   }/*end switch*/

  /*---------------------
   * Make sure the driver we want is loaded
   */
   if ((status == OK) && (drvFunc == NULL)) status = S_drvAbDf1_noDriver;

  /*---------------------
   * Try to allocate the element IO (device-private) structure
   */
   if (status == OK) {
      pIO = (*drvFunc->NewElemIO) ();
      if (!pIO) status = S_dev_noMemory;
   }/*end if found driver function table*/

  /*---------------------
   * Report any errors trying to allocate the device private structure
   */
   if (status != OK) {
      recGblRecordError (status, pRec, __FILE__ " - failed to create device-private structure.");
      return status;
   }/*end if could not create device-private structure*/

  /*---------------------
   * Fill in some of the device-private structure fields
   */
   pIO->pRec = pRec;                       /* Pointer back to record       */
   pIO->drvFunc = drvFunc;                 /* Driver Function Table        */
   pIO->devFunc = &initCallBacks;       /* Temp callback struct for now */

   *ppIO = pIO;                            /* Return address of structure  */

  /*---------------------
   * Invoke the appropriate driver routine to parse the device address
   * and set-up for df-1 element I/O.
   */
   status = (*drvFunc->SetupIO) (pLink, pIO, bitNo);
   if (status != OK) {
      recGblRecordError (status, pRec, __FILE__ "- failed to set up element I/O.");
      return status;
   }/*end if could not set up element io*/

  /*---------------------
   * If this is an output record, make sure it is on the scan list to reflect
   * external state changes.
   */
   if (!input) (*drvFunc->SetupScan) (abDf1IntScanStart, pIO);
   return OK;

}/*end devAbDf1CommonInit()*/


/*****************************************************************************/
/* devInitAbDf1() -- Device Support Initialization Routine                   */
/*      o This routine is called twice for each type of Allen Bradley DF-1   */
/*        record in the system.                                              */
/*      o It is called once per record type before any records have been     */
/*        initialized.  During the initial call, it looks to see which       */
/*        of the supported DF-1 drivers have been loaded and obtains the     */
/*        addresses of their driver function tables.                         */
/*      o It is called a second time (per record type) after all the records */
/*        have been initialized.  During the second call, it begins hardware */
/*        scanning by calling the "InitiateAll" function for each of the     */
/*        loaded drivers.                                                    */
/*        20180125 MDW Removed the 'static' class specifier so that add-on   */
/*                     device support modules can use it.                    */
/*****************************************************************************/

epicsInt32 devInitAbDf1 (unsigned pass)
{
   int                 i;                     /* Loop counter                */
   //SYM_TYPE            dummy;                 /* Dummy symbol type           */
   drvAbDf1FuncTable  *drvFunc;               /* Ptr to driver function table*/

   static int          firstPassDone = FALSE; /* True after driver function tables have been located */
   static int          lastPassDone  = FALSE; /* True after card/link scanning has been started      */

   switch (pass) {

  /*-------------------------------------------------------------------------*/
  /* First pass -- Before any records have been initialized.                 */
  /*-------------------------------------------------------------------------*/

   case devInitPassBeforeDevInitRec:

      /* Only do this once */
      if (firstPassDone) break;

     /*---------------------
      * Loop to see which drivers have been loaded and fill in their
      * function table addresses.
      */
#if 0
      for (i=0; i < MAX_DRIVERS; i++) {
         funcTable[i] = NULL;
         symFindByName (sysSymTbl,
                       (char *)funcTableName[i],
                       (char **)&funcTable[i], &dummy);
      }/*end for each possible driver*/
#else
     funcTable[0] =  &drvAbDf1Func;
     funcTable[1] =  NULL; /* &drvPlc5vFunc not supported */
#endif

      firstPassDone = TRUE;
      break;

  /*-------------------------------------------------------------------------*/
  /* Second pass -- After all records have been initialized                  */
  /*-------------------------------------------------------------------------*/

   default:

      /* Only do this once */
      if (lastPassDone) break;

     /*---------------------
      * For each loaded DF-1 driver, initiate scanning on all links/cards.
      */
      for (i=0; i < MAX_DRIVERS; i++) {
         drvFunc = funcTable[i];
         if (drvFunc != NULL) (*drvFunc->InitiateAll) ();
      }/*end for each driver*/

      lastPassDone = TRUE;
      break;

   }/*end switch*/

   return S_devAbDf1_OK;

}/*end devInitAbDf1()*/


/************************************************************************/
/* Dummy Callback Routines Used By Input Records                        */
/************************************************************************/

/*
 * Obtain Current Write Value
 */
LOCAL void dummyCurrentWriteVal (abDf1ElemIO *pElem, abDf1Value *pVal)
{
   /*
    * avoids non-determanistic response to
    * data types that have not been implemented
    */
   memset (pVal, '\0', sizeof(*pVal));
}

/*
 * Complete Write Operation
 */
LOCAL void dummyWriteCompletion (abDf1ElemIO *pElem, epicsInt32 status)
{
}

/*
 * Respond to new value
 */
LOCAL void dummyNewValue (abDf1ElemIO *pIO)
{
}

/*****************************************************************************/
/* devGetIoIntInfoAbDf1 () -- Common "Get I/O Interrupt Info" Routine        */
/*      o Extract the address of the IOSCANPVT structure from the            */
/*        "Element IO" structure.                                            */
/*      o Create the IOSCANPVT structure if it does not already exist.       */
/*      o Inform the driver of the change of status.                         */
/*                                                                           */
/*****************************************************************************/

LOCAL epicsInt32  devGetIoIntInfoAbDf1 (
int         cmd,
dbCommon   *pRec,
IOSCANPVT  *pPvt)
{
     /*---------------------
      * Get the element IO structure
      */
      abDf1ElemIO *pIO = (abDf1ElemIO *)pRec->dpvt;
      if (pIO == NULL) return S_devAbDf1_OK;

     /*---------------------
      * If the IOSCANPVT structure does not exist, create it
      */
      if ((*pPvt = pIO->ioScanPvt) == NULL) {
            scanIoInit (&pIO->ioScanPvt);
            *pPvt = pIO->ioScanPvt;
      }/*end if no IOSCANPVT structure*/

     /*---------------------
      * Inform the driver of the change
      */
      (*pIO->drvFunc->SetupScan) (cmd, pIO);
      return S_devAbDf1_OK;

}/*end devGetIoIntInfoAbDf1()*/


/*****************************************************************************/
/* Device Support for Analog Input (ai) Records                              */
/*****************************************************************************/

ai_dev_sup devAiAbDf1 = {       /* Generic AB ai Record (defaults to unsigned 12-bit) */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadUnsignedAbDf1,
      aiDevLinearConvertAbDf1_12
      };
epicsExportAddress(dset,devAiAbDf1);

ai_dev_sup devAiAbDf1_12 = {    /* unsigned 12-bit AB ai Record              */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadUnsignedAbDf1,
      aiDevLinearConvertAbDf1_12
      };
epicsExportAddress(dset,devAiAbDf1_12);

ai_dev_sup devAiAbDf1_s12 = {    /* signed 12-bit AB ai Record              */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadSignedAbDf1,
      aiDevLinearConvertAbDf1_12
      };
epicsExportAddress(dset,devAiAbDf1_s12);

ai_dev_sup devAiAbDf1_16 = {    /* unsigned 16-bit AB ai Record              */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadUnsignedAbDf1,
      aiDevLinearConvertAbDf1_16
      };
epicsExportAddress(dset,devAiAbDf1_16);

ai_dev_sup devAiAbDf1_s16 = {    /* signed 16-bit AB ai Record                */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadSignedAbDf1,
      aiDevLinearConvertAbDf1_16
      };
epicsExportAddress(dset,devAiAbDf1_s16);

/*
 * !! we change all other ai DSETs installed by the DTYP field to this DSET during 
 * !! record init if the file type specified in the address is "F"
 */
ai_dev_sup devAiAbDf1_fp = {    /* floating point AB ai Record */
      6,
      NULL,
      devInitAbDf1,
      aiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      aiDevReadFloatAbDf1,
      aiDevLinearConvertAbDf1_16 /* liniear convert isnt used if its floating point */
      };
epicsExportAddress(dset,devAiAbDf1_fp);

/* Device CallBack Table (floating point) */
devAbDf1FuncTable aiDevAbDf1FloatCallBacks = {
      dummyWriteCompletion,
      aiDevAbDf1NewFloatValue,
      dummyCurrentWriteVal,
      devAbParseAddress
};

/* Device CallBack Table (unsigned integer) */
LOCAL devAbDf1FuncTable aiDevAbDf1WordCallBacks = {
      dummyWriteCompletion,
      aiDevAbDf1NewWordValue,
      dummyCurrentWriteVal,
      devAbParseAddress
};

/*
 * aiDevInitRecAbDf1 () -- Initialize ai Record
 */
LOCAL epicsInt32
aiDevInitRecAbDf1 (struct aiRecord *pai) 
{
   epicsInt32          status;
   abDf1ElemIO  *pIO;
   ai_dev_sup   *dset;

   status = devAbDf1CommonInit (pai, &pai->inp, INPUT, &pIO, NULL);
   if (status != OK) return status;

   /*
    * Invoke the linear conversion routine to set the slope
    */
   dset = (ai_dev_sup *)pai->dset;
   (*dset->specialLinearConv) (pai, TRUE);

   /*
    * set up async completion
    */
   switch (pIO->dataType) {
   case df1DTInt:

      pIO->devFunc = &aiDevAbDf1WordCallBacks;
      break;

   case df1DTFP:
      pIO->devFunc = &aiDevAbDf1FloatCallBacks;
      /*
       * override the specified DSET
       */
      dset = &devAiAbDf1_fp;
      pai->dset = (void *) dset;
      break;

   default:
      pIO->devFunc = &devAbDf1BadTypeCallBacks;
      recGblRecordError (S_drvAbDf1_uknDataType, (void *) pai,
            __FILE__ "- unsupported data type");
      status = S_drvAbDf1_uknDataType;
   }

   pai->dpvt = (void *) pIO;

   return status;
}


/*
 * aiDevReadUnsignedAbDf1 () -- Analog input routine
 */
LOCAL epicsInt32
aiDevReadUnsignedAbDf1 (struct aiRecord * pai)
{
   drvAbDf1FuncTable  *drvFunc;
   abDf1ElemIO        *pIO = (abDf1ElemIO *) pai->dpvt;
   epicsInt32                status;
   epicsUInt16         rval;

   if (!pIO) {
      recGblSetSevr((struct dbCommon *)pai,
                        READ_ALARM,INVALID_ALARM);
      return S_devAbDf1_dontConvert;
   }

   drvFunc = pIO->drvFunc;
   status = (*drvFunc->ReadWord) (pIO, &rval);
   if (status == S_drvAbDf1_OK) {
      pai->rval = (epicsInt32) rval;
      return status;
   }

   recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
   return status;
}

/*
 * aiDevReadSignedAbDf1 () -- Analog input routine
 */
LOCAL epicsInt32
aiDevReadSignedAbDf1 (struct aiRecord * pai)
{
   drvAbDf1FuncTable  *drvFunc;
   abDf1ElemIO        *pIO = (abDf1ElemIO *) pai->dpvt;
   epicsInt32                status;
   epicsInt16            rval;

   if (!pIO) {
      recGblSetSevr((struct dbCommon *)pai,
                        READ_ALARM,INVALID_ALARM);
      return S_devAbDf1_dontConvert;
   }

   drvFunc = pIO->drvFunc;
   status = (*drvFunc->ReadWord) (pIO, (epicsUInt16 *) &rval);
   if (status == S_drvAbDf1_OK) {
      pai->rval = (epicsInt32) rval;
      return status;
   }

   recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
   return status;
}

/*
 * aiDevReadFloatAbDf1 () -- Analog input routine
 */
LOCAL epicsInt32
aiDevReadFloatAbDf1 (struct aiRecord * pai)
{
   drvAbDf1FuncTable  *drvFunc;
   abDf1ElemIO        *pIO = (abDf1ElemIO *) pai->dpvt;
   epicsInt32                status;
   float            real;

   if (!pIO) {
      recGblSetSevr((struct dbCommon *)pai,
                        READ_ALARM,INVALID_ALARM);
      return S_devAbDf1_dontConvert;
   }

   drvFunc = pIO->drvFunc;
   status = (*drvFunc->ReadReal) (pIO, &real);
   if (status == S_drvAbDf1_OK) {
      pai->val = (double) real;
      pai->udf = FALSE;
      /* AWE raise monitors */
      //db_post_events(pai, &pai->val, DBE_VALUE|DBE_LOG);
      return S_devAbDf1_dontConvert;
   }

   recGblSetSevr(pai,READ_ALARM,INVALID_ALARM);
   return status;
}

/*
 * aiDevLinearConvertAbDf1_12 () -- Set slope for unsigned 12-bit linear conversion
 */
LOCAL epicsInt32 aiDevLinearConvertAbDf1_12 (struct aiRecord *pai, int after)
{

    if(!after) return(S_devAbDf1_OK);
    /* set linear conversion slope*/
    pai->eslo = (pai->eguf - pai->egul)/0xfff;
    return(S_devAbDf1_OK);
}

/*
 * aiDevLinearConvertAbDf1_16 () -- Set slope for unsigned 16-bit linear conversion
 */
LOCAL epicsInt32 aiDevLinearConvertAbDf1_16 (struct aiRecord *pai, int after)
{

    if(!after) return(S_devAbDf1_OK);
    /* set linear conversion slope*/
    pai->eslo = (pai->eguf - pai->egul)/0xffff;
    return(S_devAbDf1_OK);
}

/*
 * aiDevAbDf1NewFloatValue () -- Respond to new floating point value 
 */
LOCAL void aiDevAbDf1NewFloatValue (abDf1ElemIO *pIO)
{
   drvAbDf1FuncTable  *drvFunc = pIO->drvFunc;
   struct aiRecord    *pai = (struct aiRecord *) pIO->pRec;
   unsigned            scanRequired = FALSE;
   epicsInt32                status;
   float             real;

   status = (*drvFunc->ReadReal) (pIO, &real);
   if (status == S_drvAbDf1_OK) {
        if ((pai->val != (double)real) || (pai->sevr >= INVALID_ALARM)) {
            scanRequired = TRUE;
        }
   }
   else if (pai->sevr < INVALID_ALARM) {
        scanRequired = TRUE;
   }

   if (scanRequired && pIO->ioScanPvt) {
      scanIoRequest(pIO->ioScanPvt);
   }
}
/*
 * aiDevAbDf1NewWordValue () -- Respond to new word value 
 */
LOCAL void aiDevAbDf1NewWordValue (abDf1ElemIO *pIO)
{
   drvAbDf1FuncTable  *drvFunc = pIO->drvFunc;
   struct aiRecord    *pai = (struct aiRecord *) pIO->pRec;
   unsigned            scanRequired = FALSE;
   epicsInt32                status;
   epicsUInt16         rval;


   status = (*drvFunc->ReadWord) (pIO, &rval);
   if (status == S_drvAbDf1_OK) {
      /*
       * when looking for a bit level change we discard the
       * upper bits in pai->rval so that signed/unsigned data
       * can be compared with the same code
       */
      epicsUInt16 recVal = (epicsUInt16) pai->rval;
      if ((recVal != (epicsInt32)rval) || (pai->sevr >= INVALID_ALARM)) {
         scanRequired = TRUE;
      }
   }
   else if (pai->sevr < INVALID_ALARM) {
      scanRequired = TRUE;
   }

   if (scanRequired && pIO->ioScanPvt) {
      scanIoRequest(pIO->ioScanPvt);
   }
}

/*****************************************************************************/
/* Device Support for Analog Output (ao) Records                             */
/*****************************************************************************/

ao_dev_sup devAoAbDf1 = {       /* Generic AB ao Record (defaults to 12-bit signed) */
      6,
      NULL,
      devInitAbDf1,
      aoDevInitRecAbDf1Unsigned,
      devGetIoIntInfoAbDf1,
      aoDevWriteAbDf1,
      aoDevLinearConvertAbDf1_12
};
epicsExportAddress(dset, devAoAbDf1);

ao_dev_sup devAoAbDf1_12 = {    /* 12-bit unsigned AB ao Record              */
      6,
      NULL,
      devInitAbDf1,
      aoDevInitRecAbDf1Unsigned,
      devGetIoIntInfoAbDf1,
      aoDevWriteAbDf1,
      aoDevLinearConvertAbDf1_12
};
epicsExportAddress(dset, devAoAbDf1_12);

ao_dev_sup devAoAbDf1_s12 = {    /* 12-bit signed AB ao Record               */
      6,
      NULL,
      devInitAbDf1,
      aoDevInitRecAbDf1Signed,
      devGetIoIntInfoAbDf1,
      aoDevWriteAbDf1,
      aoDevLinearConvertAbDf1_12
};
epicsExportAddress(dset, devAoAbDf1_s12);

ao_dev_sup devAoAbDf1_16 = {    /* 16-bit unsigned AB ao Record              */
      6,
      NULL,
      devInitAbDf1,
      aoDevInitRecAbDf1Unsigned,
      devGetIoIntInfoAbDf1,
      aoDevWriteAbDf1,
      aoDevLinearConvertAbDf1_16
};
epicsExportAddress(dset, devAoAbDf1_16);

ao_dev_sup devAoAbDf1_s16 = {    /* 16-bit signed AB ao Record               */
      6,
      NULL,
      devInitAbDf1,
      aoDevInitRecAbDf1Signed,
      devGetIoIntInfoAbDf1,
      aoDevWriteAbDf1,
      aoDevLinearConvertAbDf1_16
};
epicsExportAddress(dset, devAoAbDf1_s16);

/* Device CallBack Table */
LOCAL devAbDf1FuncTable aoDevAbDf1FloatCallBacks = {
      aoDevAbDf1WriteCompletion,
      aoDevAbDf1NewFloatValue,
      aoDevAbDf1CurrentFloatWriteVal,
      devAbParseAddress
};

/* Device CallBack Table */
LOCAL devAbDf1FuncTable aoDevAbDf1SignedWordCallBacks = {
      aoDevAbDf1WriteCompletion,
      aoDevAbDf1NewSignedValue,
      aoDevAbDf1CurrentWordWriteVal,
      devAbParseAddress
};

/* Device CallBack Table */
LOCAL devAbDf1FuncTable aoDevAbDf1UnsignedWordCallBacks = {
      aoDevAbDf1WriteCompletion,
      aoDevAbDf1NewUnsignedValue,
      aoDevAbDf1CurrentWordWriteVal,
      devAbParseAddress
};

/*
 * aoDevInitRecAbDf1Unsigned () -- Initialize unsigned ao Record
 */
LOCAL epicsInt32
aoDevInitRecAbDf1Unsigned (struct aoRecord *pao) 
{
   return aoDevInitRecAbDf1 (pao, 0);   
}

/*
 * aoDevInitRecSignedAbDf1 () -- Initialize signed ao Record
 */
LOCAL epicsInt32
aoDevInitRecAbDf1Signed (struct aoRecord *pao) 
{
   return aoDevInitRecAbDf1 (pao, 1);   
}

/*
 * aoDevInitRecAbDf1 () -- Initialize ao Record
 *
 * if signedValue is T then the value is interpreted as signed,
 * else unsigned
 */
LOCAL epicsInt32
aoDevInitRecAbDf1 (struct aoRecord *pao, int signedValue) 
{
   epicsInt32          status;
   abDf1ElemIO  *pIO;
   ao_dev_sup   *dset;
   status = devAbDf1CommonInit (pao, &pao->out, OUTPUT, &pIO, NULL);
   if (status != OK) return S_devAbDf1_dontConvert;

   /* 
    * Invoke the linear conversion routine to set the slope
    */
   dset = (ao_dev_sup *)pao->dset;
   (*dset->specialLinearConv) (pao, TRUE);

   /*
    * set up async completion
    */
   switch (pIO->dataType) {
   case df1DTInt:
      if (signedValue) {
         pIO->devFunc = &aoDevAbDf1SignedWordCallBacks;
      }
      else {
         pIO->devFunc = &aoDevAbDf1UnsignedWordCallBacks;
      }
      status = S_devAbDf1_dontConvert;
      break;

   case df1DTFP:
      pIO->devFunc = &aoDevAbDf1FloatCallBacks;
      status = S_devAbDf1_dontConvert;
      break;

   default:
      pIO->devFunc = &devAbDf1BadTypeCallBacks;
      recGblRecordError (S_drvAbDf1_uknDataType, (void *) pao,
            __FILE__ "- unsupported data type");
      status = S_drvAbDf1_uknDataType;
      break;

   }

   pao->dpvt = (void *) pIO;

   return status; 
}

/*
 * aoDevWriteAbDf1 () -- Analog output routine
 */
LOCAL epicsInt32
aoDevWriteAbDf1 (struct aoRecord *pao)
{
   abDf1ElemIO *pIO = (abDf1ElemIO *)pao->dpvt;
   epicsInt32 status;

   if (pao->pact) {
      return S_devAbDf1_OK; 
   }

   if (!pIO) {
      recGblSetSevr((struct dbCommon *)pao,
                        WRITE_ALARM,INVALID_ALARM);
      return S_devAbDf1_OK; 
   }

   pao->pact = TRUE; /* see aoDevAbDf1NewValue */
   status = (*pIO->drvFunc->InitiateWrite) (pIO);
   if (status == S_drvAbDf1_OK) {
      pao->pact = FALSE;
   }
   else if (status==S_drvAbDf1_asyncCompletion) {
      status = S_drvAbDf1_OK;
   }
   else {
      recGblSetSevr (pao, WRITE_ALARM, INVALID_ALARM);
      pao->pact = FALSE;
   }
   return status;
}

/*
 * aoDevLinearConvertAbDf1_12 () -- Set slope for 12-bit linear conversion
 */
LOCAL epicsInt32 aoDevLinearConvertAbDf1_12 (struct aoRecord *pao, int after)
{
    if(!after) return(S_devAbDf1_OK);
    /* set linear conversion slope*/
    pao->eslo = (pao->eguf - pao->egul)/0xfff;
    return(S_devAbDf1_OK);
}

/*
 * aoDevLinearConvertAbDf1_16 () -- Set slope for 16-bit linear conversion
 */
LOCAL epicsInt32 aoDevLinearConvertAbDf1_16 (struct aoRecord *pao, int after)
{
    if(!after) return(S_devAbDf1_OK);
    /* set linear conversion slope*/
    pao->eslo = (pao->eguf - pao->egul)/0xffff;
    return(S_devAbDf1_OK);
}

/*
 * aoDevAbDf1WriteCompletion () -- Write completion routine
 */
LOCAL void aoDevAbDf1WriteCompletion (abDf1ElemIO *pIO, epicsInt32 status)
{
      struct aoRecord *pao = (struct aoRecord *) pIO->pRec;
      struct rset *prset = (struct rset *) (pao->rset);

      dbScanLock ( (struct dbCommon *) pao);

      /*
       * always process if this is the end of 
       * async rec  processing
       */
      if (status!=S_drvAbDf1_OK) {
            recGblSetSevr (pao, WRITE_ALARM, INVALID_ALARM);
      }

      /*
       * write routine is a NOOP when PACT is true
       */
      (*prset->process)(pao);

      dbScanUnlock ( (struct dbCommon *) pao);
}

/*
 * aoDevAbDf1NewSignedValue () -- Respond to new integer value
 */
LOCAL void aoDevAbDf1NewSignedValue (abDf1ElemIO *pIO)
{
   aoDevAbDf1NewWordValue (pIO, 1);
}

/*
 * aoDevAbDf1NewUnsignedValue () -- Respond to new integer value
 */
LOCAL void aoDevAbDf1NewUnsignedValue (abDf1ElemIO *pIO)
{
   aoDevAbDf1NewWordValue (pIO, 0);
}

/*
 * aoDevAbDf1NewWordValue () -- Respond to new integer value
 *
 * if signedValue is T then the value is interpreted as signed,
 * else unsigned
 */
LOCAL void aoDevAbDf1NewWordValue (abDf1ElemIO *pIO, int signedValue)
{
   drvAbDf1FuncTable  *drvFunc = pIO->drvFunc;
   struct aoRecord    *pao = (struct aoRecord *) pIO->pRec;
   unsigned            scanRequired = FALSE;
   epicsInt32                status = S_devAbDf1_OK;
   double              value;
   epicsUInt16         rval;

   /*
    * no updates when asynch write in progress
    */
   if (pao->pact) {
      return;
   }

   dbScanLock ( (struct dbCommon *) pao);

   status = (*drvFunc->ReadWord) (pIO, &rval);
   if (status == S_drvAbDf1_OK) {
      /*
       * when looking for a bit level change we discard the
       * upper bits in pai->rval so that signed/unsigned data
       * can be compared with the same code
       */
      epicsInt16 recVal = (epicsUInt16) pao->rval;
      if ((recVal != rval) || (pao->sevr >= INVALID_ALARM)) {
         scanRequired = TRUE;

         if (signedValue) {
            epicsInt16 newVal = (epicsInt16) rval;
            pao->rval = (epicsInt32) newVal;
         }
         else {
            pao->rval = (epicsInt32) rval;
         }

         value = pao->rval + pao->roff;
         if (pao->aslo!=0.0) value *= pao->aslo;
         if (pao->aoff!=0.0) value += pao->aoff;
         if (pao->linr==menuConvertLINEAR) {
            value = value*pao->eslo + pao->egul;
         }
         else if (pao->linr==menuConvertNO_CONVERSION) {
            value = pao->rval;
         }
         else {
            cvtRawToEngBpt(&value, pao->linr, pao->init,
               (void *)&pao->pbrk, &pao->lbrk);
         }
      }
   }
   else if (pao->sevr < INVALID_ALARM) {
      scanRequired = TRUE;
   }

   if (scanRequired) {
      aoDevAbDf1NewValue (pao, value, status);
   }

   dbScanUnlock ( (struct dbCommon *) pao);
}
/*
 * aoDevAbDf1NewFloatValue () -- Respond to new floating point value
 */
LOCAL void aoDevAbDf1NewFloatValue (abDf1ElemIO *pIO)
{
   drvAbDf1FuncTable  *drvFunc = pIO->drvFunc;
   struct aoRecord    *pao = (struct aoRecord *) pIO->pRec;
   unsigned            scanRequired = FALSE;
   epicsInt32                status = S_devAbDf1_OK;
   double              value = 0.0;
   float            real;

   /*
    * no updates when asynch write in progress
    */
   if (pao->pact) {
      return;
   }

   dbScanLock ( (struct dbCommon *) pao);

   status = (*drvFunc->ReadReal) (pIO, &real);
   if (status == S_drvAbDf1_OK) {
      if ((pao->val != (double)real) || (pao->sevr >= INVALID_ALARM)) {
         value = (double) real;
         scanRequired = TRUE;
      }
   }
   else if (pao->sevr < INVALID_ALARM) {
      scanRequired = TRUE;
   }

   if (scanRequired) {
      aoDevAbDf1NewValue (pao, value, status);
   }

   dbScanUnlock ( (struct dbCommon *) pao);
}

/*
 * aoDevAbDf1NewValue () -- update record if there is a state change
 * !! dbScanLock() must be applied here !!
 */
LOCAL void aoDevAbDf1NewValue (struct aoRecord *pao, double value, epicsInt32 status)
{
   struct rset        *prset = (struct rset *) (pao->rset);

   if (status==S_devAbDf1_OK) {
        pao->val = value;
        pao->oval = value;
        pao->pval = value;
        pao->udf = FALSE;
        /* AWE raise monitors */
        //db_post_events(pao, &pao->val, DBE_VALUE|DBE_LOG);
   }     
   else {
        recGblSetSevr (pao, WRITE_ALARM, INVALID_ALARM);
   }
   /*
    * fake async write completion so that the 
    * write routine in this dev sup, and the  
    * conversion part of record processing will be
    * a NOOP.
    */
   pao->pact = TRUE;
   (*prset->process) (pao);
}

/*
 * adDevAbDf1CurrentWriteVal () -- Return current raw value of record
 */
LOCAL void aoDevAbDf1CurrentWordWriteVal (abDf1ElemIO *pIO, abDf1Value *pVal)
{
   struct aoRecord *pao = (struct aoRecord *) pIO->pRec;

   pVal->word = (epicsUInt16) pao->rval;
}

/*
 * adDevAbDf1CurrentWriteVal () -- Return current value of record
 */
LOCAL void aoDevAbDf1CurrentFloatWriteVal (abDf1ElemIO *pIO, abDf1Value *pVal)
{
   struct aoRecord *pao = (struct aoRecord *) pIO->pRec;

   pVal->real = (float) pao->val;
}

/*****************************************************************************/
/* Device Support for Binary Input (bi) Records                              */
/*****************************************************************************/

bi_dev_sup devBiAbDf1 = {
      5,
      NULL,
      devInitAbDf1,
      biDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      biDevReadAbDf1
};
epicsExportAddress(dset, devBiAbDf1);

LOCAL devAbDf1FuncTable bidevAbDf1FuncTable = {
      dummyWriteCompletion,
      biDevAbDf1NewValue,
      dummyCurrentWriteVal,
      devAbParseAddress
};

/*
 * biDevInitRecAbDf1 () -- Initialize Binary Input Record
 */
LOCAL epicsInt32 biDevInitRecAbDf1 (struct biRecord *pbi) 
{
      epicsInt32 status;
      abDf1ElemIO *pIO;
      unsigned bitNo;
 
      status = devAbDf1CommonInit (pbi, &pbi->inp, INPUT, &pIO, &bitNo);
      if (status != OK) return status;

      pbi->mask = 1;
      pbi->mask <<= bitNo;

      /*
       * set up async notification of scan complete 
       */
      pIO->devFunc = &bidevAbDf1FuncTable;

      if (pIO->dataType!=df1DTBit) {
            recGblRecordError (S_drvAbDf1_uknDataType, (void *) pbi,
                   __FILE__ "- incompatible data type");
            return S_drvAbDf1_uknDataType;
      }

      pbi->dpvt = (void *) pIO;

      return status;
}

/*
 * biDevReadAbDf1 () -- Binary Input Routine
 */
LOCAL epicsInt32 biDevReadAbDf1 (struct biRecord * pbi)
{
      abDf1ElemIO *pIO = (abDf1ElemIO *)pbi->dpvt;
      epicsUInt16 rval;
      epicsInt32 status;

    if (!pIO) {
        recGblSetSevr((struct dbCommon *)pbi,
                        READ_ALARM,INVALID_ALARM);
        return S_devAbDf1_dontConvert; 
    }

      status = (*pIO->drvFunc->ReadBitString) (pIO, (epicsUInt16) pbi->mask, &rval);
      if (status == S_drvAbDf1_OK) {
            pbi->rval = rval;
            pbi->val = rval?1:0;
            /* AWE raise monitors */
            // 
            //db_post_events(pbi, &pbi->val, DBE_VALUE|DBE_LOG);
      }
      else {
            recGblSetSevr(pbi,READ_ALARM,INVALID_ALARM);
      }

      return status;
}

/*
 * biDevAbDf1NewValue () -- Respond to Value Change 
 */
LOCAL void biDevAbDf1NewValue (abDf1ElemIO *pIO)
{
      struct biRecord *pbi = (struct biRecord *) pIO->pRec;
      unsigned scanRequired = FALSE;
      epicsUInt16 rval;
      epicsInt32 status;

      status = (*pIO->drvFunc->ReadBitString) (pIO, (epicsUInt16) pbi->mask, &rval);
      if (status == S_drvAbDf1_OK) {
            if (pbi->rval!=rval || pbi->sevr>=INVALID_ALARM) {
                  scanRequired = TRUE;
            }
      }
      else if (pbi->sevr<INVALID_ALARM) {
            scanRequired = TRUE;
      }

      if (scanRequired && pIO->ioScanPvt) {
            scanIoRequest(pIO->ioScanPvt);
      }
}


/*****************************************************************************/
/* Device Support for Binary Output (bo) Records                             */
/*****************************************************************************/

bo_dev_sup devBoAbDf1 = {
      5,
      NULL,
      devInitAbDf1,
      boDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      boDevWriteAbDf1
};
epicsExportAddress(dset, devBoAbDf1);

LOCAL devAbDf1FuncTable bodevAbDf1FuncTable = {
      boDevAbDf1WriteCompletion,
      boDevAbDf1NewValue,
      boDevAbDf1CurrentWriteVal,
      devAbParseAddress
};

/*
 * boDevInitRecAbDf1 () -- Initialize Binary Ouptput Record
 */
LOCAL epicsInt32
boDevInitRecAbDf1(struct boRecord *pbo) 
{
      epicsInt32 status;
      abDf1ElemIO *pIO;
      unsigned bitNo;
 
      status = devAbDf1CommonInit (pbo, &pbo->out, OUTPUT, &pIO, &bitNo);
      if (status != OK) return status;

      pbo->mask = 1;
      pbo->mask <<= bitNo;

      /*
       * set up async completion
       */
      pIO->devFunc = &bodevAbDf1FuncTable;

      if (pIO->dataType!=df1DTBit) {
            recGblRecordError (S_drvAbDf1_uknDataType, (void *) pbo ,
                   __FILE__ "- incompatible data type");
            return S_drvAbDf1_uknDataType;
      }

      pbo->dpvt = (void *) pIO;

      return S_devAbDf1_dontConvert;
}

/*
 * boDevWriteAbDf1 () -- Binary Output Routine
 */
LOCAL epicsInt32 boDevWriteAbDf1 (struct boRecord * pbo)
{
      abDf1ElemIO *pIO = (abDf1ElemIO *)pbo->dpvt;
      int         status;

      if (pbo->pact) {
            return S_devAbDf1_OK; 
      }

      if (!pIO) {
            recGblSetSevr( (struct dbCommon *) pbo,
                                    WRITE_ALARM, INVALID_ALARM);
            return S_devAbDf1_OK; 
      }

      pbo->pact = TRUE; /* see boDevAbDf1NewValue */
      status = (*pIO->drvFunc->InitiateWrite) (pIO);
      if (status == S_drvAbDf1_OK) {
            pbo->pact = FALSE;
      }
      else if (status==S_drvAbDf1_asyncCompletion) {
            status = S_drvAbDf1_OK;
      }
      else {
            pbo->pact = FALSE;
            recGblSetSevr (pbo, WRITE_ALARM, INVALID_ALARM);
      }

      return status;
}

/*
 * boDevAbDf1WriteCompletion () -- Binary Output Write Completion Routine
 */
LOCAL void boDevAbDf1WriteCompletion (abDf1ElemIO *pIO, epicsInt32 status)
{
      struct boRecord *pbo = (struct boRecord *) pIO->pRec;
      struct rset *prset = (struct rset *) (pbo->rset);

      dbScanLock ( (struct dbCommon *) pbo);

      /*
       * always process if this is the end of 
       * async rec  processing
       */
      if (status!=S_drvAbDf1_OK) {
            recGblSetSevr (pbo, WRITE_ALARM, INVALID_ALARM);
      }

      /*
       * write routine is a NOOP when PACT is true
       */
      (*prset->process)(pbo);

      dbScanUnlock ( (struct dbCommon *) pbo);
}

/*
 * boDevAbDf1NewValue () -- Respond to value change from outside EPICS
 */
LOCAL void boDevAbDf1NewValue (abDf1ElemIO *pIO)
{
      struct boRecord *pbo = (struct boRecord *) pIO->pRec;
      struct rset *prset = (struct rset *) (pbo->rset);
      unsigned scanRequired = FALSE;
      epicsInt32 status = S_devAbDf1_OK;
      epicsUInt16 rval;

      /*
       * no updates when asynch write in progress
       */
      if (pbo->pact) {
            return;
      }

      dbScanLock ( (struct dbCommon *) pbo);
      status = (*pIO->drvFunc->ReadBitString) (pIO, (epicsUInt16) pbo->mask, &rval);
      if (status == S_drvAbDf1_OK) {
            if (pbo->rval!=rval || pbo->sevr>=INVALID_ALARM) {
                  scanRequired = TRUE;
                  pbo->rval = rval;
                  pbo->val = rval?1:0;
                  pbo->udf = FALSE;
                  /* AWE raise monitors */
                  //db_post_events(pbo, &pbo->val, DBE_VALUE|DBE_LOG);
            }
      }
      else if (pbo->sevr<INVALID_ALARM) {
            recGblSetSevr (pbo, WRITE_ALARM, INVALID_ALARM);
            scanRequired = TRUE;
      }

      if (scanRequired) {
            /*
             * fake async write completion so that the 
             * write routine in this dev sup, an the  
             * conversion part of record processing will be
             * a NOOP.
             */
            pbo->pact = TRUE;
            (*prset->process) (pbo);
      }

      dbScanUnlock ( (struct dbCommon *) pbo);
}

/*
 * boDevAbDf1CurrentWriteVal () -- Return current record value
 */
LOCAL void boDevAbDf1CurrentWriteVal (abDf1ElemIO *pIO, abDf1Value *pVal)
{
      struct boRecord *pbo = (struct boRecord *) pIO->pRec;

      pVal->bitString.mask = pbo->mask;
      pVal->bitString.value = pbo->rval;
}


/*****************************************************************************/
/* Device Support for Multi-Bit Binary Input (mbbi) Records                  */
/*****************************************************************************/

mbbi_dev_sup devMbbiAbDf1 = {
      5,
      NULL,
      devInitAbDf1,
      mbbiDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      mbbiDevReadAbDf1
};
epicsExportAddress(dset, devMbbiAbDf1);

LOCAL devAbDf1FuncTable mbbidevAbDf1FuncTable = {
      dummyWriteCompletion,
      mbbiDevAbDf1NewValue,
      dummyCurrentWriteVal,
      devAbParseAddress};

/*
 * mbbiDevInitRecAbDf1 () -- Initialize mbbi Record
 */
LOCAL epicsInt32
mbbiDevInitRecAbDf1(struct mbbiRecord *pmbbi)
{
      abDf1ElemIO *pIO;
      epicsInt32 status;
      unsigned bitNo;
      status = devAbDf1CommonInit (pmbbi, &pmbbi->inp, INPUT, &pIO, &bitNo);
      if (status != OK) return status;

      pmbbi->shft = bitNo;
      pmbbi->mask <<= bitNo;

      /*
       * set up async notification of scan complete 
       */
      pIO->devFunc = &mbbidevAbDf1FuncTable;
 
      if (pIO->dataType!=df1DTBit) {
            recGblRecordError (S_drvAbDf1_uknDataType, (void *) pmbbi,
                   __FILE__ "- incompatible data type");
            return S_drvAbDf1_uknDataType;
      }

      pmbbi->dpvt = (void *) pIO;

      return status;
}

/*
 * mbbiDevAbDf1NewValue () -- Respond to New Value 
 * (save CPU by causing scan on interrupt only when the value has changed)
 */
LOCAL void mbbiDevAbDf1NewValue(abDf1ElemIO *pIO)
{
      struct mbbiRecord *pmbbi = (struct mbbiRecord *) pIO->pRec;
      unsigned scanRequired = FALSE;
      epicsInt32 status;
      epicsUInt16 rval;

      status = (*pIO->drvFunc->ReadBitString) (pIO, (epicsUInt16) pmbbi->mask, &rval);
      if(status == S_drvAbDf1_OK){
             if (pmbbi->rval!=rval || pmbbi->sevr>=INVALID_ALARM) {
                  scanRequired = TRUE;
             }
      }
      else {
            if (pmbbi->sevr<INVALID_ALARM) {
                  scanRequired = TRUE;
            }
      }

      if (scanRequired && pIO->ioScanPvt) {
            scanIoRequest(pIO->ioScanPvt);
      }
}

/*
 * mbbiDevReadAbDf1 () -- Mbbi Input Routine
 */
LOCAL epicsInt32 mbbiDevReadAbDf1 (struct mbbiRecord * pmbbi)
{
      abDf1ElemIO *pIO = (abDf1ElemIO *) pmbbi->dpvt;
      epicsUInt16 rval;
      epicsInt32 status;

      if (!pIO) {
            recGblSetSevr (pmbbi, READ_ALARM, INVALID_ALARM);
            return S_devAbDf1_dontConvert; 
      }

      status = (*pIO->drvFunc->ReadBitString) (pIO, (epicsUInt16) pmbbi->mask, &rval);
      if (status) {
            recGblSetSevr (pmbbi, READ_ALARM, INVALID_ALARM);
      }
      else {
            pmbbi->rval = (epicsUInt16) rval;
      }

      return status;
}


/*****************************************************************************/
/* Device Support for Multi-Bit Binary Output (mbbo) Records                 */
/*****************************************************************************/

mbbo_dev_sup devMbboAbDf1 = {
      5,
      NULL,
      devInitAbDf1,
      mbboDevInitRecAbDf1,
      devGetIoIntInfoAbDf1,
      mbboDevWriteAbDf1
};
epicsExportAddress(dset, devMbboAbDf1);

LOCAL devAbDf1FuncTable mbbodevAbDf1FuncTable = {
      mbboDevAbDf1WriteCompletion,
      mbboDevAbDf1NewValue,
      mbboDevAbDf1CurrentWriteVal,
      devAbParseAddress
};

/*
 * mbboDevInitRecAbDf1 () -- Initialize mbbo Record
 */
LOCAL epicsInt32
mbboDevInitRecAbDf1(struct mbboRecord *pmbbo)
{
      epicsInt32 status;
      abDf1ElemIO *pIO;
      unsigned bitNo;

    status = devAbDf1CommonInit (pmbbo, &pmbbo->out, OUTPUT, &pIO, &bitNo);
    if (status != OK) return status;

      pmbbo->shft = bitNo;
      pmbbo->mask <<= bitNo;
 
      /*
       * set up async completion
       */
      pIO->devFunc = &mbbodevAbDf1FuncTable;

      if (pIO->dataType!=df1DTBit) {
            recGblRecordError (S_drvAbDf1_uknDataType, (void *) pmbbo,
                   __FILE__ "- incompatible data type");
            return S_drvAbDf1_uknDataType;
      }

      pmbbo->dpvt = (void *) pIO;

      return S_devAbDf1_dontConvert;
}

/*
 * mbboDevWriteAbDf1 () -- Mbbo Output Routine
 */
LOCAL epicsInt32 mbboDevWriteAbDf1 (struct mbboRecord *pmbbo)
{
        abDf1ElemIO     *pIO = (abDf1ElemIO *) pmbbo->dpvt;
      int         status;
 
      if (pmbbo->pact) {
            return S_devAbDf1_OK; 
      }

      if (!pIO) {
            recGblSetSevr ((struct dbCommon *)pmbbo,
                                    WRITE_ALARM,INVALID_ALARM);
            return S_devAbDf1_dontConvert; 
      }
 
      pmbbo->pact = TRUE; /* see mbboDevAbDf1NewValue */
      status = (*pIO->drvFunc->InitiateWrite) (pIO);
      if (status == S_drvAbDf1_OK) {
            pmbbo->pact = FALSE;
      }
      else if (status==S_drvAbDf1_asyncCompletion) {
            status = S_drvAbDf1_OK;
      }
      else {
            recGblSetSevr (pmbbo, WRITE_ALARM, INVALID_ALARM);
            pmbbo->pact = FALSE;
      }
      return status;
}

/*
 * mbboDevAbDf1WriteCompletion () -- Write Completion Routine
 */
LOCAL void mbboDevAbDf1WriteCompletion (abDf1ElemIO *pIO, epicsInt32 status)
{
      struct mbboRecord *pmbbo = (struct mbboRecord *) pIO->pRec;
      struct rset *prset = (struct rset *) (pmbbo->rset);

      dbScanLock ( (struct dbCommon *) pmbbo);

      /*
       * always process if this is the end of 
       * async rec  processing
       */
      if (status!=S_drvAbDf1_OK) {
            recGblSetSevr (pmbbo, WRITE_ALARM, INVALID_ALARM);
      }

      /*
       * write routine is a NOOP when PACT is true
       */
      (*prset->process)(pmbbo);

      dbScanUnlock ( (struct dbCommon *) pmbbo);
}

/*
 * mbboDevAbDf1NewValue () -- Respond to new value
 */
LOCAL void mbboDevAbDf1NewValue (abDf1ElemIO *pIO)
{
      struct mbboRecord *pmbbo = (struct mbboRecord *) pIO->pRec;
      struct rset *prset = (struct rset *) (pmbbo->rset);
      unsigned scanRequired = FALSE;
      epicsInt32 status = S_devAbDf1_OK;
      epicsUInt16 rval;

      /*
       * no updates when asynch write in progress
       */
      if (pmbbo->pact) {
            return;
      }

      dbScanLock ( (struct dbCommon *) pmbbo);

      status = (*pIO->drvFunc->ReadBitString) (pIO, pmbbo->mask, &rval);
      if (status == S_drvAbDf1_OK) {
            if (pmbbo->rval!=rval || pmbbo->sevr>=INVALID_ALARM) {

                  pmbbo->rval = (epicsInt32) rval;

                  if (pmbbo->shft>0) rval >>= pmbbo->shft;

                  if (pmbbo->sdef){
                        epicsUInt32 *pstate_values;
                        epicsUInt16 i;

                        pstate_values = &(pmbbo->zrvl);
                        for (i = 0u; i < 16u; i++){
                              if (pstate_values[i] == rval){
                                    pmbbo->val = i;
                                    pmbbo->udf = FALSE;
                                    scanRequired = TRUE;
                                    break;
                              }
                        }
                  }
                  else{
                        /* the raw  is the desired val */
                        pmbbo->val =  (epicsUInt16) rval;
                        pmbbo->udf = FALSE;
                        scanRequired = TRUE;
                  }
            }
      }
      else if (pmbbo->sevr<INVALID_ALARM) {
            recGblSetSevr (pmbbo, WRITE_ALARM, INVALID_ALARM);
            scanRequired = TRUE;
      }

      if (scanRequired) {
            /*
             * fake async write completion so that the 
             * write routine in this dev sup, and the  
             * conversion part of record processing will be
             * a NOOP.
             */
            pmbbo->pact = TRUE;
            (*prset->process) (pmbbo);
      }

      dbScanUnlock ( (struct dbCommon *) pmbbo);
}

LOCAL void mbboDevAbDf1CurrentWriteVal (abDf1ElemIO *pIO, abDf1Value *pVal)
{
    struct mbboRecord *pmbbo = (struct mbboRecord *) pIO->pRec;
    
    pVal->bitString.mask = pmbbo->mask; 
    pVal->bitString.value = pmbbo->rval;
}


/************************************************************************/
/* ab_test_parse () -- Global routine to test out how the Allen-Bradley */
/*                     address parser will work on your own strings.    */
/*                                                                      */
/************************************************************************/

void ab_test_parse (char *address)
{
   int fileType, dataType, fileNumber, element;
   int subelement, subbit, size, structured;
   int status;

   status = devAbParseAddress (address, &fileType, &dataType, &fileNumber,
                               &element, &subelement, &subbit, &size,
                               &structured);
   epicsPrintf ("Address = \'%s\'  Status = %x\n"
           "File Number = %d\n"
           "File Type = %d\n"
           "Data Type = %d\n"
           "Element Number = %d\n"
           "Sub-Element Number = %d\n"
           "Bit Number = %d\n"
           "Element Size = %d\n"
           "Structured = %d\n",
            address, status, fileNumber, fileType, dataType, element, subelement, subbit, size, structured);
}
