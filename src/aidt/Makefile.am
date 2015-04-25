noinst_LTLIBRARIES = libaidt.la

noinst_HEADERS = abstractaction.h abstractcondition.h aceroundaction.h aceroundcondition.h \
	bestsuitcondition.h checksevencondition.h decisiontree.h iaction.h iaistate.h icondition.h \
	jackonlycondition.h maxsuitaction.h nextaction.h playjackaction.h powersuitaction.h \
	powersuitcondition.h servesevenaction.h skipplayercondition.h suspendaction.h

libaidt_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine
libaidt_la_CXXFLAGS = $(NO_RTTI_FLAGS)
libaidt_la_SOURCES = abstractaction.cpp abstractcondition.cpp aceroundaction.cpp \
	aceroundcondition.cpp bestsuitcondition.cpp checksevencondition.cpp decisiontree.cpp \
	jackonlycondition.cpp maxsuitaction.cpp nextaction.cpp playjackaction.cpp \
	powersuitaction.cpp powersuitcondition.cpp servesevenaction.cpp skipplayercondition.cpp \
	suspendaction.cpp
