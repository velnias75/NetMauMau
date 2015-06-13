noinst_LTLIBRARIES = libai.la

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

noinst_HEADERS = abstractaction.h abstractcondition.h aceroundaction.h aceroundcondition.h \
	baseaiplayer.h bestjackaction.h bestsuitcondition.h binarycondition.h \
	checkjacksuitaction.h checksevencondition.h decisionbase.h decisionchain.h \
	havejackcondition.h havelessthancondition.h iaction.h iaistate.h icondition.h \
	jackonlycondition.h jackplusoneaction.h jackplusonecondition.h jacksuitaction.h \
	maxsuitaction.h nextaction.h playjackaction.h powerplayaction.h powersuitaction.h \
	powersuitcondition.h randomjackaction.h randomjackcondition.h servesevenaction.h \
	skipplayeraction.h skipplayercondition.h staticcondition.h suspendaction.h

libai_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(GSL)
libai_la_CXXFLAGS = $(NO_EXCEPTIONS)
libai_la_SOURCES = abstractaction.cpp abstractcondition.cpp aceroundaction.cpp \
	aceroundcondition.cpp bestjackaction.cpp bestsuitcondition.cpp checkjacksuitaction.cpp \
	checksevencondition.cpp decisionbase.cpp havejackcondition.cpp jackonlycondition.cpp \
	jackplusonecondition.cpp jackplusoneaction.cpp jacksuitaction.cpp maxsuitaction.cpp \
	nextaction.cpp playjackaction.cpp powerplayaction.cpp powersuitaction.cpp \
	powersuitcondition.cpp randomjackaction.cpp randomjackcondition.cpp servesevenaction.cpp \
	skipplayeraction.cpp skipplayercondition.cpp suspendaction.cpp
