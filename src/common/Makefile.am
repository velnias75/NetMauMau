lib_LTLIBRARIES = libnetmaumaucommon.la
noinst_LTLIBRARIES = libnetmaumaucommon_private.la

BUILT_SOURCES = ai-icon.h

noinst_HEADERS = ai-icon.h abstractconnectionimpl.h abstractsocketimpl.h base64.h basiclogger.h \
	ci_char_traits.h errorstring.h icardfactory.h iplayer.h logger.h mimemagic.h pngcheck.h \
	protocol.h random_gen.h smartptr.h

MAINTAINERCLEANFILES = ai-icon.h

.DELETE_ON_ERROR:
ai-icon.h: $(AI_IMAGE)
	$(AM_V_GEN) cat $(AI_IMAGE) | $(SHELL) $(top_srcdir)/src/images/create_ai_icon.sh 'ai_icon_data' \
		> $(builddir)/$(@F)

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libnetmaumaucommon_private_la_CPPFLAGS = $(GSL) -DNMM_EXE_PATH=\"$(bindir)\" \
	-DNMM_EXE_NAME=\"nmm-server$(EXEEXT)\" $(NO_EXCEPTIONS)
libnetmaumaucommon_private_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_private_la_SOURCES = cardtools.cpp ci_char_traits.cpp defaultplayerimage.cpp \
	errorstring.cpp mimemagic.cpp pngcheck.cpp protocol.cpp

libnetmaumaucommon_la_CPPFLAGS = $(GSL) -DNMM_EXE_PATH=\"$(bindir)\" \
	-DNMM_EXE_NAME=\"nmm-server$(EXEEXT)\"
libnetmaumaucommon_la_CXXFLAGS = -I$(top_srcdir)/src/include
libnetmaumaucommon_la_SOURCES = abstractconnection.cpp abstractconnectionimpl.cpp \
	abstractsocket.cpp abstractsocketimpl.cpp base64.cpp logger.cpp socketexception.cpp
libnetmaumaucommon_la_LIBADD = libnetmaumaucommon_private.la
libnetmaumaucommon_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 5:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
