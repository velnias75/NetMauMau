noinst_LTLIBRARIES = libaidt.la

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

noinst_HEADERS = abstractaction.h abstractcondition.h aceroundaction.h aceroundcondition.h \
	bestjackaction.h bestsuitcondition.h binarycondition.h checkjacksuitaction.h \
	checksevencondition.h decisiontree.h havejackcondition.h havelessthancondition.h \
	iaction.h iaistate.h icondition.h jackonlycondition.h jackremoverbase.h jacksuitaction.h \
	maxsuitaction.h nextaction.h playjackaction.h powerplayaction.h powersuitaction.h \
	powersuitcondition.h randomjackaction.h randomjackcondition.h servesevenaction.h \
	skipplayeraction.h skipplayercondition.h staticcondition.h suspendaction.h

libaidt_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(GSL)
libaidt_la_CXXFLAGS = $(NO_RTTI_FLAGS)
libaidt_la_SOURCES = abstractaction.cpp abstractcondition.cpp aceroundaction.cpp \
	aceroundcondition.cpp bestjackaction.cpp bestsuitcondition.cpp checkjacksuitaction.cpp \
	checksevencondition.cpp havejackcondition.cpp jackonlycondition.cpp jackremoverbase.cpp \
	jacksuitaction.cpp maxsuitaction.cpp nextaction.cpp playjackaction.cpp powerplayaction.cpp \
	powersuitaction.cpp powersuitcondition.cpp randomjackaction.cpp randomjackcondition.cpp \
	servesevenaction.cpp skipplayeraction.cpp skipplayercondition.cpp suspendaction.cpp
