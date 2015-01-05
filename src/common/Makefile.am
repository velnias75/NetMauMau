lib_LTLIBRARIES = libnetmaumaucommon.la
noinst_LTLIBRARIES = libnetmaumaucommon_private.la

noinst_HEADERS = abstractconnectionimpl.h abstractsocketimpl.h base64.h basiclogger.h \
	errorstring.h icardfactory.h iplayer.h logger.h pngcheck.h random_gen.h 

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libnetmaumaucommon_private_la_CPPFLAGS = $(GSL) -DNMM_EXE_PATH=\"$(bindir)\" \
	-DNMM_EXE_NAME=\"nmm-server$(EXEEXT)\" $(NO_RTTI_FLAGS)
libnetmaumaucommon_private_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_private_la_SOURCES = cardtools.cpp errorstring.cpp pngcheck.cpp

libnetmaumaucommon_la_CPPFLAGS = $(GSL) -DNMM_EXE_PATH=\"$(bindir)\" \
	-DNMM_EXE_NAME=\"nmm-server$(EXEEXT)\"
libnetmaumaucommon_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_la_SOURCES = abstractconnection.cpp abstractconnectionimpl.cpp \
	abstractsocket.cpp abstractsocketimpl.cpp base64.cpp logger.cpp socketexception.cpp
libnetmaumaucommon_la_LIBADD = libnetmaumaucommon_private.la
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 2:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
