#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := vesync_sdk_esp32

include $(IDF_PATH)/make/project.mk

ctags:
	ctags -R `pwd` $(IDF_PATH)/components
