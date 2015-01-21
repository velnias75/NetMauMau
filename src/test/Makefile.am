check_PROGRAMS = test_netmaumau

if ENABLE_CLI_CLIENT
bin_PROGRAMS = nmm-client
endif

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

EXTRA_DIST = testimg.png

MAINTAINERCLEANFILES = testimg.h

BUILT_SOURCES = testimg.h

export GSL_RNG_TYPE := ranlxs2
export GSL_RNG_SEED := 280375

TESTS = $(check_PROGRAMS)

noinst_HEADERS = testeventhandler.h testclient.h testimg.h

test_netmaumau_CPPFLAGS = $(GSL)
test_netmaumau_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine
test_netmaumau_SOURCES = test_netmaumau.cpp testeventhandler.cpp
test_netmaumau_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la \
	../sqlite/libsqlite.la
test_netmaumau_LDFLAGS = -no-install

if ENABLE_CLI_CLIENT
nmm_client_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/client $(POPT_CFLAGS)
nmm_client_SOURCES = test_client.cpp testclient.cpp
nmm_client_LDADD = ../client/libnetmaumauclient.la ../common/libnetmaumaucommon.la $(POPT_LIBS)
endif

.DELETE_ON_ERROR:
testimg.h: testimg.png
	$(SHELL) $(top_srcdir)/src/images/create_ai_icon.sh 'test_client_img' $< > $@
