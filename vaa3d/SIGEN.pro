TEMPLATE	= lib
CONFIG	+= qt plugin
VAA3DPATH = /Users/arosh/git/vaa3d
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= SIGEN_plugin.h
SOURCES	+= SIGEN_plugin.cpp basic_surf_objs.cpp v3d_message.cpp

TARGET	= $$qtLibraryTarget(SIGEN)
