TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += AbDf1

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install foo.dbd into <top>/dbd
DBD += AbDf1.dbd

# specify all source files to be compiled and added to the library
#foo_SRCS += xxx

foo_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

