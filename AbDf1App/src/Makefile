TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# For base-7
USR_CFLAGS += -DUSE_TYPED_RSET -DUSE_TYPED_DSET -DUSE_TYPED_DRVET

#Install headers
DBD += AbDf1Support.dbd

#==================================================
# build a support library

LIBRARY_IOC += AbDf1

# specify all source files to be compiled and added to the library
AbDf1_SRCS += df1.c
AbDf1_SRCS += drvAbDf1.c
AbDf1_SRCS += devAbDf1.c

# Install headers
INC += drvAbDf1.h

AbDf1_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an IOC for testing

PROD_IOC = AbDf1App

# Generate DBD file
DBD += AbDf1App.dbd

AbDf1App_DBD += base.dbd
AbDf1App_DBD += drvSerial.dbd
AbDf1App_DBD += AbDf1Support.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
AbDf1App_SRCS += AbDf1App_registerRecordDeviceDriver.cpp
AbDf1App_SRCS_DEFAULT += AbDf1AppMain.cpp
AbDf1App_SRCS_vxWorks += -nil-

AbDf1App_LIBS += AbDf1
AbDf1App_LIBS += drvSerial
AbDf1App_LIBS += $(EPICS_BASE_IOC_LIBS)


include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

