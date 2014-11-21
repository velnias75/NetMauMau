lib_LTLIBRARIES = libnetmaumaucommon.la

noinst_HEADERS = icardfactory.h iplayer.h basiclogger.h logger.h random_gen.h

libnetmaumaucommon_la_CPPFLAGS = -DNMM_EXE_PATH=\"$(bindir)\" -DNMM_EXE_NAME=\"nmm-server$(EXEEXT)\"
libnetmaumaucommon_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_la_SOURCES = abstractsocket.cpp abstractconnection.cpp socketexception.cpp \
	cardtools.cpp logger.cpp
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 0:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
