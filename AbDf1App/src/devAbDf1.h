
/*************************************************************************
 * devAbDf1.h -- Allen Bradley DF1 Serial Protocol EPICS Device Support
 *               For PLC-2, PLC-5, and PLC-5/V Controllers.
 *
 * Authors:  Jeff Hill, Eric Bjorklund
 * Date:     17 October 1997
 *
 *------------------------------------------------------------------------
 * MODIFICATION LOG:
 *
 *
 *------------------------------------------------------------------------
 * MODULE DESCRIPTION:
 *
 * This header file defines the interface between the records and the
 * device-support layer for Allen-Bradley DF-1 Protocol devices.
 *
 *************************************************************************/

/*========================================================================
 *                      Format For PLC-2 Addressing
 * Format of the instrument type addresses accepted by analog records for PLC2 IO
 * <serial device address> = "@ <file name> <node number> <PLC2 word address>"
 * EX: "@ /tyCo/2 9 100"
 *
 * Format of the instrument type addresses accepted by binary records for PLC2 IO
 * <serial device address> = "@ <file name> <node number> 
 *                            <PLC2 word address> <bit number>"
 * EX: "@ /tyCo/2 9 100 2"
 *
 *========================================================================
 *                      Format For PLC-5 Addressing
 * Format of the instrument type addresses accepted by analog records for PLC5 IO
 * <serial device address> = "@ <file name> <node number> 
 *                                  <AB PLC5 ascii logical address (omit bit number)>"
 * EX: "@ /tyCo/0 9 N7:100"
 *
 * Format of the instrument type addresses accepted by binary records for PLC5 IO
 * <serial device address> = "@ <file name> <node number> <AB PLC5 ascii logical address>"
 * EX: "@ /tyCo/0 9 N7:100/2"
 *
 * All examples above use serial port /tyCo/0, node 9, file 5 (PLC 5 only), word
 * (PLC5 element) 100, bit 2.
 *
 * (where a word address is a byte address divided by two)
 * (where bits are number with the ls bit is bit 0 and the ms bit is bit 15)
 * (where a file name is frequently the device name of a serial port
 *    ex: /tyCo/2 (see iosDevShow under vxWorks) )
 * 
 *========================================================================
 *                      Format For PLC-5/VME Addressing
 * The PLC-5/VME uses VME_IO addressing to specify the card and signal number
 * (signal number is always 0).  The Allen-Bradley ascii logical address is
 * specfied in the parameter field (after the '@' sign).
 *
 * EX: "#C0 S0 @N7:100"                 -- Specifies word 100 in Integer file 7
 *                                         on Card 0.
 *
 * EX: "#C0 S0 @N7:100/2"               -- Specifies bit 2 of word 100 in Integer file 7
 *                                         on Card 0.
 *
 *========================================================================
 *                      Allen Bradley Ascii Logical Addressing
 * Ascii Logical Addressing is used to specify the hardware addresses on both PLC-5
 * and PLC-5/VME devices.  An Allen-Bradley Ascii logical address consists of the
 * following:
 *      o A file type (one or two letters)
 *      o A file number
 *      o An element number -- separated from the file number by a colon (":").
 *      o Optionally, a bit or subelement number separated from the element
 *        number by a forward slash ("/") or a period (".").
 *
 * File types currently recognized are:
 *      A  = Ascii Characters
 *      B  = Bit String
 *      BT = Block Transfer Control Structure
 *      C  = Counter
 *      D  = Binary Coded Decimal (BCD)
 *      F  = Floating Point (IEEE Single precision)
 *      MG = Message Structure
 *      N  = Integer
 *      PD = PID Control Structure
 *      R  = General Control Structure
 *      SC = SFC Status Block
 *      ST = Ascii String
 *      T  = Timer
 *
 *      Each file in the PLC5 can only have a single type, therefore you would not
 *      expect to see both N7 and F7 in the same PLC.  The exception to this is that
 *      integer files can be addressed either by word (e.g. N7:29) or by bit (e.g. B7/29)
 *      Note that in the last example, we did not specify an element number, only a bit
 *      number.  This is equivalent to saying "B7:1/14" (bit 29 of the file is found
 *      in word 1 bit 14).
 *
 * Special Files:
 *      There are three special files that have their own addressing conventions.
 *      These are:
 *        O  = Output Image File (file 0)
 *        I  = Input Image File (file 1)
 *        S  = PLC Status File (file 2)
 *      The Input, Output, and Status files are specified by their file letter only,
 *      and do not take a file number.  The element number immediately follows the
 *      file letter, as in the following example:
 *              S7/2  - Specifies bit 2 in word 7 of the PLC Status File
 *                      (this happens to be the "fault" bit for rack 2)
 *
 *      Another exception is that the element and bit numbers for the Input and Output
 *      image files are always specified in octal (all other numbers in the Ascii
 *      Logical Addressing scheme are decimal).  Thus, to reference bit 15 of word 29
 *      in the Input file (I), you would write:
 *              I037/17
 *      Note that this corresponds to Allen-Bradley rack 3, word 7, bit 15 (which probably
 *      explains the weird octal convention).
 *
 * Symbolic Addressing of Structure Sub-Elements
 *      Sub-elements of some structured files may be symbolically referenced using
 *      the same mnemonics as are used in the ladder logic programs.  A symbolic
 *      subelement may either refer to a single bit:
 *              EX. T4:12/DN    - The "Done" bit of Timer element 12 in file 4
 *      or to an entire field:
 *              EX. C5:4/ACC    - The "Accumulated Value" field of counter element 4
 *                                in file 5.
 *      Refer to your Allen-Bradley programming manuals for mnemonic field definitions.
 *      Structured element types that currently support symbolic sub-element
 *      references are: Block Transfer Controllers (BT), Counters (C), General Control
 *      Structures (R), and Timers (T).
 *      
 * Note that PLC5 logical ascii addresses are currently restricted to only direct
 * adressing modes.
 */

/*
 * EPICS record definitions
 */
#include    <aiRecord.h>
#include    <aoRecord.h>
#include    <biRecord.h>
#include    <boRecord.h>
#include    <mbbiRecord.h>
#include    <mbboRecord.h>

/*
 *  Common Device Support Routines
 */
typedef epicsInt32 devInit (unsigned);
typedef epicsInt32 devGetIoIntInfo (int, dbCommon*,  IOSCANPVT*);

/*****************************************************************************/
/* Device Support for Analog Input (ai) Records                              */
/*****************************************************************************/

typedef epicsInt32 aiDevReport (struct aiRecord *, unsigned);
typedef epicsInt32 aiDevInitRec (struct aiRecord *);
typedef epicsInt32 aiDevRead (struct aiRecord *);
typedef epicsInt32 aiDevLinearConv (struct aiRecord *, int);

typedef struct /* ai_dev_sup */ {
      epicsInt32                   number;
      aiDevReport           *report;
      devInit               *init;
      aiDevInitRec          *initRec;
      devGetIoIntInfo       *getIoIntInfo;
      aiDevRead             *read;
      aiDevLinearConv       *specialLinearConv;
} ai_dev_sup;

extern ai_dev_sup devAiAbDf1; /* Generic AB ai Record (defaults to unsigned 12-bit) */
extern ai_dev_sup devAiAbDf1_12; /* unsigned 12-bit AB ai Record */
extern ai_dev_sup devAiAbDf1_s12; /* signed 12-bit AB ai Record */
extern ai_dev_sup devAiAbDf1_16; /* unsigned 16-bit AB ai Record */
extern ai_dev_sup devAiAbDf1_s16; /* signed 16-bit AB ai Record */
/*
 * !! we change all other ai DSETs installed by the DTYP field to this DSET during 
 * !! record init if the file type specified in the address is "F"
 */
extern ai_dev_sup devAiAbDf1_fp; /* floating point AB ai Record */

/*****************************************************************************/
/* Device Support for Analog Output (ao) Records                             */
/*****************************************************************************/

typedef epicsInt32 aoDevReport (struct aoRecord *, unsigned);
typedef epicsInt32 aoDevInit (unsigned);
typedef epicsInt32 aoDevInitRec (struct aoRecord *);
typedef epicsInt32 aoDevWrite (struct aoRecord *);
typedef epicsInt32 aoDevLinearConv (struct aoRecord *, int);

typedef struct /* ao_dev_sup */ {
      epicsInt32               number;
      aoDevReport       *report;
      devInit           *init;
      aoDevInitRec      *initRec;
      devGetIoIntInfo   *getIoIntInfo;
      aoDevWrite        *write;
      aoDevLinearConv   *specialLinearConv;
} ao_dev_sup;

extern ao_dev_sup devAoAbDf1; /* Generic AB ao Record (defaults to 12-bit signed) */
extern ao_dev_sup devAoAbDf1_12; /* 12-bit unsigned AB ao Record */
extern ao_dev_sup devAoAbDf1_s12; /* 12-bit signed AB ao Record */
extern ao_dev_sup devAoAbDf1_16; /* 16-bit unsigned AB ao Record */
extern ao_dev_sup devAoAbDf1_s16; /* 16-bit signed AB ao Record */

/*****************************************************************************/
/* Device Support for Binary Input (bi) Records                              */
/*****************************************************************************/

typedef epicsInt32 biDevReport (struct biRecord *, unsigned);
typedef epicsInt32 biDevInit (unsigned);
typedef epicsInt32 biDevInitRec (struct biRecord *);
typedef epicsInt32 biDevRead (struct biRecord *);

typedef struct /* bi_dev_sup */{
      epicsInt32               number;
      biDevReport       *report;
      devInit           *init;
      biDevInitRec      *initRec;
      devGetIoIntInfo   *getIoIntInfo;
      biDevRead         *read;
} bi_dev_sup;

extern bi_dev_sup devBiAbDf1;

/*****************************************************************************/
/* Device Support for Binary Output (bo) Records                             */
/*****************************************************************************/
typedef epicsInt32 boDevReport (struct boRecord *, unsigned);
typedef epicsInt32 boDevInit (unsigned);
typedef epicsInt32 boDevInitRec (struct boRecord *);
typedef epicsInt32 boDevWrite (struct boRecord *);

typedef struct /* bo_dev_sup */ {
      epicsInt32               number;
      boDevReport       *report;
      devInit           *init;
      boDevInitRec      *initRec;
      devGetIoIntInfo   *getIoIntInfo;
      boDevWrite        *write;
} bo_dev_sup;

extern bo_dev_sup devBoAbDf1;

/*****************************************************************************/
/* Device Support for Mulit-Bit Binary Input (mbbi) Records                  */
/*****************************************************************************/

typedef epicsInt32 mbbiDevReport (struct mbbiRecord *, unsigned);
typedef epicsInt32 mbbiDevInit (unsigned);
typedef epicsInt32 mbbiDevInitRec (struct mbbiRecord *);
typedef epicsInt32 mbbiDevRead (struct mbbiRecord *);

typedef struct /* mbbi_dev_sup */ {
      epicsInt32                  number;
      mbbiDevReport        *report;
      devInit              *init;
      mbbiDevInitRec       *initRec;
      devGetIoIntInfo      *getIoIntInfo;
      mbbiDevRead          *read;
} mbbi_dev_sup;

extern mbbi_dev_sup devMbbiAbDf1;

/*****************************************************************************/
/* Device Support for Multi-Bit Binary Output (mbbo) Records                 */
/*****************************************************************************/

typedef epicsInt32 mbboDevReport (struct mbboRecord *, unsigned);
typedef epicsInt32 mbboDevInit (unsigned);
typedef epicsInt32 mbboDevInitRec (struct mbboRecord *);
typedef epicsInt32 mbboDevWrite (struct mbboRecord *);

typedef struct /* mbbo_dev_sup */ {
      epicsInt32                  number;
      mbboDevReport        *report;
      devInit              *init;
      mbboDevInitRec       *initRec;
      devGetIoIntInfo      *getIoIntInfo;
      mbboDevWrite         *write;
} mbbo_dev_sup;

extern mbbo_dev_sup devMbboAbDf1;
