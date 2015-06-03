check_PROGRAMS = test_netmaumau
noinst_SCRIPTS = stresstest.sh

if ENABLE_CLI_CLIENT
bin_PROGRAMS = nmm-client
endif

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

EXTRA_DIST = testimg.png stresstest.sh.in

MAINTAINERCLEANFILES = testimg.h
DISTCLEANFILES = stresstest.sh

BUILT_SOURCES = testimg.h

export GSL_RNG_TYPE := ranlxs2
export GSL_RNG_SEED := 280375
export NETMAUMAU_RULES := $(abs_top_srcdir)/src/lua/stdrules.lua
export NMM_NO_TRACE := 1
export NMM_NO_SQLITE := 1

TESTS = $(check_PROGRAMS)

noinst_HEADERS = testeventhandler.h testclient.h testimg.h

test_netmaumau_CPPFLAGS = $(GSL)
test_netmaumau_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine -I$(top_srcdir)/src/ai -I$(top_srcdir)/src/lua
test_netmaumau_SOURCES = test_netmaumau.cpp testeventhandler.cpp
test_netmaumau_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la
test_netmaumau_LDFLAGS = -no-install

if ENABLE_CLI_CLIENT
nmm_client_CPPFLAGS = -DCLIENTVERSION=$(CLIENTVERSION)
nmm_client_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/client $(POPT_CFLAGS)
nmm_client_SOURCES = test_client.cpp testclient.cpp
nmm_client_LDADD = ../common/libnetmaumaucommon.la ../client/libnetmaumauclient.la $(POPT_LIBS)
endif

.DELETE_ON_ERROR:
testimg.h: testimg.png
	$(AM_V_GEN)$(SHELL) $(top_srcdir)/src/images/create_ai_icon.sh 'test_client_img' $< > $@

stresstest.sh: $(top_srcdir)/src/test/stresstest.sh.in
	$(AM_V_GEN)$(SED) \
		-e 's|@RULES[@]|$(NETMAUMAU_RULES)|g' \
		-e 's|@SHELL[@]|$(SHELL)|g' \
		-e 's|@check_PROGRAMS[@]|$(abs_builddir)/$(check_PROGRAMS)|g' < $< > $@
	@chmod u+x $@
