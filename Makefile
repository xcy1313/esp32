#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := WiFiBT_Scale_FatScalePlus_US.V

include $(IDF_PATH)/make/project.mk

ctags:
	ctags -R `pwd` $(IDF_PATH)/components
