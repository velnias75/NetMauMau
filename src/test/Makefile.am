check_PROGRAMS = test_netmaumau

if ENABLE_CLIENT
noinst_PROGRAMS = test_client
endif

TESTS = $(check_PROGRAMS)

noinst_HEADERS = testeventhandler.h testclient.h

test_netmaumau_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine
test_netmaumau_SOURCES = test_netmaumau.cpp testeventhandler.cpp
test_netmaumau_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la
test_netmaumau_LDFLAGS = -no-install

if ENABLE_CLIENT
test_client_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/client $(POPT_CFLAGS)
test_client_SOURCES = test_client.cpp testclient.cpp
test_client_LDADD = ../client/libnetmaumauclient.la ../common/libnetmaumaucommon.la $(POPT_LIBS)
test_client_LDFLAGS = -no-install
endif
