lib_LTLIBRARIES = libnetmaumaucommon.la
noinst_LTLIBRARIES = libnetmaumaucommon_private.la

BUILT_SOURCES = ai-icon.h

noinst_HEADERS = abstractconnectionimpl.h abstractsocketimpl.h base64.h basiclogger.h \
	ci_string.h errorstring.h icardfactory.h iobserver.h iplayer.h logger.h mimemagic.h \
	observable.h pathtools.h pngcheck.h protocol.h select.h signalblocker.h smartptr.h \
	smartsingleton.h zlibexception.h zstreambuf.h

DISTCLEANFILES = ai-icon.h

.DELETE_ON_ERROR:
ai-icon.h: $(AI_IMAGE)
	$(AM_V_GEN) cat $(AI_IMAGE) | $(SHELL) $(top_srcdir)/src/images/create_ai_icon.sh \
	'ai_icon_data' > $(builddir)/$(@F)

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libnetmaumaucommon_private_la_CPPFLAGS = $(GSL) -DNMM_EXE_PATH=\"$(bindir)\" \
	-DNMM_EXE_NAME=\"nmm-server\" -DNMM_EXE_EXT=\"$(EXEEXT)\" -DPKGDATADIR="\"$(pkgdatadir)\"" \
	$(NO_EXCEPTIONS) -DEXPORT_SMARTSINGLETON
libnetmaumaucommon_private_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_private_la_SOURCES = cardtools.cpp ci_string.cpp defaultplayerimage.cpp \
	errorstring.cpp mimemagic.cpp pathtools.cpp pngcheck.cpp protocol.cpp signalblocker.cpp
libnetmaumaucommon_private_la_LIBADD = $(MAGIC_LIBS)

libnetmaumaucommon_la_CPPFLAGS = $(GSL) -DEXPORT_SMARTSINGLETON
libnetmaumaucommon_la_CXXFLAGS = -I$(top_srcdir)/src/include 
libnetmaumaucommon_la_SOURCES = abstractconnection.cpp abstractconnectionimpl.cpp \
	abstractsocket.cpp abstractsocketimpl.cpp base64.cpp logger.cpp select.cpp \
	socketexception.cpp

if WITHZLIB
libnetmaumaucommon_la_SOURCES += zlibexception.cpp zstreambuf.cpp
endif

libnetmaumaucommon_la_LIBADD = libnetmaumaucommon_private.la $(ZLIB_LIBS)
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 6:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
