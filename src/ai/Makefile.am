noinst_LTLIBRARIES = libai.la

SUFFIXES = .aai .cai

EXTRA_DIST = genaction.awk gencondition.awk aceroundaction.aai aceroundcondition.cai \
	bestjackaction.aai bestsuitcondition.cai checksevencondition.cai havejackcondition.cai \
	jackonlycondition.cai jackplusoneaction.aai jackplusonecondition.cai jacksuitaction.aai \
	maxsuitaction.aai playjackaction.aai powerjackaction.aai powerjackcondition.cai \
	powerplayaction.aai randomjackaction.aai randomjackcondition.cai servesevenaction.aai \
	skipplayeraction.aai skipplayercondition.cai suspendaction.aai

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

BUILT_SOURCES = aceroundaction.h aceroundcondition.h bestjackaction.h bestsuitcondition.h \
	checksevencondition.h havejackcondition.h jackonlycondition.h jackplusoneaction.h \
	jackplusonecondition.h jacksuitaction.h maxsuitaction.h playjackaction.h powerjackaction.h \
	powerjackcondition.h powerplayaction.h randomjackaction.h randomjackcondition.h \
	servesevenaction.h skipplayeraction.h skipplayercondition.h suspendaction.h

DISTCLEANFILES = $(BUILT_SOURCES)

noinst_HEADERS = abstractaction.h abstractcondition.h baseaiplayer.h binarycondition.h \
	checkjacksuitaction.h decisionbase.h decisionchain.h havelessthancondition.h iaction.h \
	iaistate.h icondition.h nextaction.h powersuitaction.h powersuitcondition.h \
	staticcondition.h 

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

.cai.h:
	$(AM_V_GEN)$(AWK) -f $(srcdir)/gencondition.awk < $< > $@
