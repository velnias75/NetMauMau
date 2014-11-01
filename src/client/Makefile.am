if ENABLE_CLIENT

lib_LTLIBRARIES = libnetmaumauclient.la

noinst_HEADERS = clientcardfactory.h interceptederrorexception.h

libnetmaumauclient_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include
libnetmaumauclient_la_SOURCES = abstractclient.cpp clientconnection.cpp clientcardfactory.cpp \
	interceptederrorexception.cpp timeoutexception.cpp
libnetmaumauclient_la_LDFLAGS = -nodefaultlibs -nostartfiles -no-undefined \
	-version-info 0:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
libnetmaumauclient_la_LIBADD = ../common/libnetmaumaucommon.la

endif
