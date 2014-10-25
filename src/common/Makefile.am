lib_LTLIBRARIES = libnetmaumaucommon.la

noinst_HEADERS = icardfactory.h iplayer.h stdplayer.h basiclogger.h logger.h

pkginclude_HEADERS = icard.h linkercontrol.h abstractconnection.h abstractsocket.h \
	socketexception.h cardtools.h

libnetmaumaucommon_la_SOURCES = abstractsocket.cpp abstractconnection.cpp socketexception.cpp \
	cardtools.cpp stdplayer.cpp logger.cpp
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 0:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
