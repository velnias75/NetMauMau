if ENABLE_CLIENT

lib_LTLIBRARIES = libnetmaumauclient.la
noinst_LTLIBRARIES = libnetmaumauclient_private.la

noinst_HEADERS = clientcardfactory.h interceptederrorexception.h

libnetmaumauclient_private_la_CXXFLAGS = $(NO_RTTI_FLAGS) -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/include
libnetmaumauclient_private_la_SOURCES = clientcardfactory.cpp

libnetmaumauclient_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include
libnetmaumauclient_la_SOURCES = abstractclient.cpp clientconnection.cpp \
	interceptederrorexception.cpp timeoutexception.cpp shutdownexception.cpp \
	playerlistexception.cpp capabilitiesexception.cpp protocolerrorexception.cpp \
	connectionrejectedexception.cpp versionmismatchexception.cpp nonetmaumauserverexception.cpp
libnetmaumauclient_la_LDFLAGS = -nodefaultlibs -nostartfiles -no-undefined \
	-version-info 1:$(SERVER_VERSION_MINOR):$(SERVER_VERSION_MAJOR)
libnetmaumauclient_la_LIBADD = libnetmaumauclient_private.la ../common/libnetmaumaucommon.la

endif
