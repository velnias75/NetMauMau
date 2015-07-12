noinst_LTLIBRARIES = libai.la

SUFFIXES = .aai

EXTRA_DIST = genaction.awk aceroundaction.aai bestjackaction.aai jackplusoneaction.aai \
	jacksuitaction.aai maxsuitaction.aai playjackaction.aai powerjackaction.aai \
	powerplayaction.aai randomjackaction.aai servesevenaction.aai skipplayeraction.aai \
	suspendaction.aai

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

BUILT_SOURCES = aceroundaction.h bestjackaction.h jackplusoneaction.h jacksuitaction.h \
	maxsuitaction.h playjackaction.h powerjackaction.h powerplayaction.h randomjackaction.h \
	servesevenaction.h skipplayeraction.h suspendaction.h

DISTCLEANFILES = $(BUILT_SOURCES)

noinst_HEADERS = abstractaction.h abstractcondition.h aceroundcondition.h \
	baseaiplayer.h bestsuitcondition.h binarycondition.h checkjacksuitaction.h \
	checksevencondition.h decisionbase.h decisionchain.h havejackcondition.h \
	havelessthancondition.h iaction.h iaistate.h icondition.h jackonlycondition.h \
	jackplusonecondition.h nextaction.h powerjackcondition.h powersuitaction.h \
	powersuitcondition.h randomjackcondition.h skipplayercondition.h staticcondition.h 

libai_la_CPPFLAGS = $(GSL)
libai_la_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(NO_EXCEPTIONS)
libai_la_SOURCES = abstractaction.cpp abstractcondition.cpp aceroundaction.cpp \
	aceroundcondition.cpp bestjackaction.cpp bestsuitcondition.cpp checkjacksuitaction.cpp \
	checksevencondition.cpp decisionbase.cpp havejackcondition.cpp jackonlycondition.cpp \
	jackplusoneaction.cpp jackplusonecondition.cpp jacksuitaction.cpp maxsuitaction.cpp \
	nextaction.cpp playjackaction.cpp powerplayaction.cpp powerjackaction.cpp \
	powerjackcondition.cpp powersuitaction.cpp powersuitcondition.cpp randomjackaction.cpp \
	randomjackcondition.cpp servesevenaction.cpp skipplayeraction.cpp skipplayercondition.cpp \
	suspendaction.cpp

.DELETE_ON_ERROR:
.aai.h:
	$(AM_V_GEN)$(AWK) -f $(srcdir)/genaction.awk < $< > $@
