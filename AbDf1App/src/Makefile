TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += AbDf1

DBD += AbDf1.dbd

# specify all source files to be compiled and added to the library
AbDf1_SRCS += df1.c
AbDf1_SRCS += drvAbDf1.c
AbDf1_SRCS += devAbDf1.c

AbDf1_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

