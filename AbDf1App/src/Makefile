TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += AbDf1

AbDf1_LIBS += $(EPICS_BASE_IOC_LIBS)
DBD += AbDf1.dbd

# fix https://gitlab.com/nsf-noirlab/gemini/rtsw/support/AbDf1/-/issues/4
AbDf1_DBD += base.dbd

# specify all source files to be compiled and added to the library
AbDf1_SRCS += df1.c
AbDf1_SRCS += drvAbDf1.c
AbDf1_SRCS += devAbDf1.c

AbDf1_LIBS += $(EPICS_BASE_IOC_LIBS)

INC += drvAbDf1.h


USR_CFLAGS += -DUSE_TYPED_RSET -DUSE_TYPED_DSET -DUSE_TYPED_DRVET
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

