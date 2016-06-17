/******************************************************************************
 * df1.c -- Allen-Bradley DF1 Protocol data structure instantiations
 *
 *-----------------------------------------------------------------------------
 * Authors: Eric Bjorklund, Jeff Hill
 * Date:    29 October 1997
 *-----------------------------------------------------------------------------
 * MODIFICATION LOG:
 * 08-Jun-2016   mdw       Separated data structure instantiations out 
 *                         of df1.h into a separate (this) source file.
 * 06-Jun-2016   mdw       Modified to use <epicsTypes.h>, 
 *                         removed #include <vxWorks.h>
 * 29-Oct-97     bjo,joh   Original Release 
 *
 *-----------------------------------------------------------------------------
 * MODULE DESCRIPTION:
 *
 *      This module contains data structure instantiations 
 *      for the Allen-Bradley DF-1 message protocol driver.
 *
 *****************************************************************************/
#define DF1_SRC
#include "df1.h"

   /*---------------------------------------------------------------------------*/
   /* Arrays relating symbolic sub-elements to file types                       */
   /*---------------------------------------------------------------------------*/
   /*---------------------------------------------------------------------------*/
   /* Define symbolic sub-element symbols for Timer elements                    */
   /* (Note: First element is default if no subelement specified)               */
   /*---------------------------------------------------------------------------*/

   static const subelementBitSymbol timer_status_bits [] = {
      { "EN",     15,       1   }, /* Enable bit        */
      { "TT",     14,       1   }, /* Timing bit        */
      { "DN",     13,       1   }, /* Done bit          */
      { 0,        0,        0   }  /* end of array      */
   };

   static const subelementSymbol timer_symbols [] = {

   /*      Symbol    Bits                SubElem Type                       */
   /*      ------    ----                ------------                       */
      { "STS",  timer_status_bits,      df1DTInt   }, /* Status bits       */ 
      { "PRE",  0,                      df1DTInt   }, /* Preset Value      */
      { "ACC",  0,                      df1DTInt   }, /* Accumulated Value */
      { "",     0,                      df1DTNone  }, /* Reserved           */
      { "",     0,                      df1DTNone  }, /* Reserved          */
   };


   /*---------------------------------------------------------------------------*/
   /* Define symbolic sub-element symbols for Counter elements                  */
   /* (Note: First element is default if no subelement specified)               */
   /*---------------------------------------------------------------------------*/

   static const subelementBitSymbol counter_status_bits [] = {
      { "CU",     15,   1},   /* Count up bit      */
      { "CD",     14,   1},   /* Count down bit    */
      { "DN",     13,   1},   /* Done bit          */
      { "OV",     12,   1},   /* Overflow bit      */
      { "UN",     11,   1},   /* Underflow bit     */
      { 0,        0,    0}    /* end of array      */
   };

   static const subelementSymbol counter_symbols [] = {

   /*      Symbol    Bits                SubElem Type                       */
   /*      ------    ----                ------------                       */
      { "STS",  counter_status_bits,    df1DTInt },   /* Status bits       */
      { "PRE",  0,                      df1DTInt },   /* Preset Value      */
      { "ACC",  0,                      df1DTInt },   /* Accumulated Value */
   };


   /*---------------------------------------------------------------------------*/
   /* Define symbolic sub-element symbols for Control elements                  */
   /* (Note: First element is default if no subelement specified)               */
   /*---------------------------------------------------------------------------*/
   static const subelementBitSymbol control_status_bits [] = {
      { "EN",     15,   1 },   /* Enable bit        */
      { "EU",     14,   1 },   /* Enable Unload bit */
      { "DN",     13,   1 },   /* Done bit          */
      { "EM",     12,   1 },   /* Empty bit         */
      { "ER",     11,   1 },   /* Error bit         */
      { "UL",     10,   1 },   /* Unload bit        */
      { "IN",     9,     1 },   /* Inhibit bit       */
      { "FD",     8,    1 },   /* Found bit         */
      { 0,        0,    0 }     /* end of array      */
   };

   static const subelementSymbol control_symbols [] = {

   /*      Symbol    Bits                SubElem Type                       */
   /*      ------    ----                ------------                       */
      { "STS",  control_status_bits,    df1DTInt },   /* Status bits       */
      { "LEN",  0,                      df1DTInt },   /* Length            */
      { "POS",  0,                      df1DTInt }    /* Position          */
   };


   /*---------------------------------------------------------------------------*/
   /* Define symbolic sub-element symbols for Block Transfer Control Structures */
   /* (Note: First element is default if no subelement specified)               */
   /*---------------------------------------------------------------------------*/

   static const subelementBitSymbol control_block_xfer_status_bits [] = {
      { "EN",     15,   1 },   /* Xfer enabled bit  */
      { "ST",     14,   1 },   /* Start transfer bit*/
      { "DN",     13,   1 },   /* Transfer done bit */
      { "ER",     12,   1 },   /* Error bit         */
      { "CO",     11,   1 },   /* Continuous md bit */
      { "EW",     10,   1 },   /* Enable waiting bit*/
      { "NR",     9,    1 },   /* No response bit   */
      { "TO",     8,    1 },   /* Timeout bit       */
      { "RW",     7,    1 },   /* Read/Write bit    */
      { 0,        0,    0 }    /* end of array      */
   };

   static const subelementBitSymbol control_block_xfer_rgs_bits [] = {
      { "RACK",   5,   5 },   /* Start of rack num */
      { "GRP",    2,   3 },   /* Start of group num*/
      { "SLOT",   0,   2 },   /* Start of slot num */
      { 0,        0,   0 }    /* end of array      */
   };

   static const subelementSymbol block_xfer_symbols [] = {

   /*      Symbol    Bits                        SubElem Type                       */
   /*      ------    ----                        ------------                       */
      { "STS",  control_block_xfer_status_bits,       df1DTInt },   /* Status bits       */
      { "RLEN", 0,                            df1DTInt },   /* Requested Length  */
      { "DLEN", 0,                            df1DTInt },   /* Transmitted Length*/
      { "FILE", 0,                            df1DTInt },   /* File number       */
      { "ELEM", 0,                            df1DTInt },   /* Element number    */
      { "RGS",  control_block_xfer_rgs_bits,          df1DTInt }    /* Rack/Grp/Slt word */
   };

   const abTypeInfo abTypeInfo_array[] = {
   /*   subElemTable         Name               Size */
   /*   ------------         ----               ---- */
      {0,                   "Invalid",             0},
      {0,                   "B (Single Bit)",      2},
      {0,                   "BS (Bit String)",     2},
      {0,                   "CS (Byte String)",   84},
      {0,                   "N (Integer)",         2},
      {timer_symbols,       "T (Timer)",           NELEMENTS(timer_symbols)*2},
      {counter_symbols,     "C (Counter)",         NELEMENTS(counter_symbols)*2},
      {control_symbols,     "R (Control)",         NELEMENTS(control_symbols)*2},
      {0,                   "F (Floating Point)",  4},
      {0,                   "A (Array)",           0},
   
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "ADR (Address)",       0}, /* physical addr size varies */
      {0,                   "D (BCD)",             2},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},

      {0,                   "Reserved",            0},
      {0,                   "PD (PID)",          164},
      {0,                   "MG (Message)",      112},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {0,                   "SC (Status)",         6},
   
      {0,                   "Reserved",            0},
      {0,                   "Reserved",            0},
      {block_xfer_symbols,  "BT (Block Transfer)", NELEMENTS(block_xfer_symbols)*2}
   };

   const unsigned char cToabDataType_array[0x100] = 
   {
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
    
      /*
       * uppercase ASCII
       */
      df1DTNone, df1DTByteStr, df1DTBit, df1DTCtr, df1DTBCD, df1DTNone, df1DTFP, df1DTNone, 
      df1DTNone, df1DTInt, df1DTNone, df1DTNone, df1DTNone, df1DTMsg, df1DTInt, df1DTInt, 
      df1DTPid, df1DTNone, df1DTGCS, df1DTInt, df1DTTmr, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 

      /* 
       * lower case ASCII
       */
      df1DTNone, df1DTByteStr, df1DTBit, df1DTCtr, df1DTBCD, df1DTNone, df1DTFP, df1DTNone, 
      df1DTNone, df1DTInt, df1DTNone, df1DTNone, df1DTNone, df1DTMsg, df1DTInt, df1DTInt, 
      df1DTPid, df1DTNone, df1DTGCS, df1DTInt, df1DTTmr, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 

      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, 
      df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone, df1DTNone
   };


