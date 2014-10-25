if ENABLE_CLIENT

lib_LTLIBRARIES = libnetmaumauclient.la

noinst_HEADERS = clientcardfactory.h interceptederrorexception.h

pkginclude_HEADERS = abstractclient.h clientconnection.h timeoutexception.h

libnetmaumauclient_la_CPPFLAGS = -I$(top_srcdir)/src/common
libnetmaumauclient_la_SOURCES = abstractclient.cpp clientconnection.cpp clientcardfactory.cpp \
	interceptederrorexception.cpp timeoutexception.cpp
libnetmaumauclient_la_LDFLAGS = -nodefaultlibs -nostartfiles \
	-version-info 0:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)

endif
