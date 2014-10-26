lib_LTLIBRARIES = libnetmaumaucommon.la

noinst_HEADERS = icardfactory.h iplayer.h stdplayer.h basiclogger.h logger.h

libnetmaumaucommon_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_la_SOURCES = abstractsocket.cpp abstractconnection.cpp socketexception.cpp \
	cardtools.cpp stdplayer.cpp logger.cpp
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 0:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
