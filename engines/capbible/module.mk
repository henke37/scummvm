MODULE := engines/capbible

MODULE_OBJS := \
	mainarchive.o \
	metaengine.o \
	debugger.o \
	saveload.o \
	music.o \
	capbible.o

# This module can be built as a plugin
ifeq ($(ENABLE_CAPBIBLE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objecs
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_CAPBIBLE), STATIC_PLUGIN)
# External dependencies for detection.
# DETECT_OBJS += $(MODULE)/resource.o
endif
