#-------------------------------------------------
#
# Project created by QtCreator 2016-12-17T13:47:56
#
#-------------------------------------------------

QT       -= core gui

TARGET = stratify_link
TEMPLATE = lib

CONFIG += staticlib

DEFINES += STRATIFYLIBLINK_LIBRARY __link

SOURCES += src/link/link_bootloader.c \
		   src/link/link_debug.c \
		   src/link/link_dir.c \
		   src/link/link_file.c \
		   src/link/link_phy.c \
		   src/link/link_posix_trace.c \
		   src/link/link_process.c \
		   src/link/link_protocol_master.c \
		   src/link/link_stdio.c \
		   src/link/link_time.c \
		   src/link/link.c \
		   src/link_common/link_transport.c \
		   src/link/link_flags.h

INSTALL_HEADERS.files = $$HEADERS

macx: DEFINES += __macosx
macx:INSTALLPATH = /Applications/StratifyLabs-SDK/Tools/gcc
win32:INSTALLPATH = c:/StratifyLabs-SDK/Tools/gcc
win32: DEFINES += __win32

target.path = $$INSTALLPATH/lib
INSTALL_HEADERS.path = $$INSTALLPATH/include

INSTALLS += target
INSTALLS += INSTALL_HEADERS

INCLUDEPATH += $$INSTALLPATH/include
