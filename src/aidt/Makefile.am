noinst_LTLIBRARIES = libaidt.la

noinst_HEADERS = abstractaction.h abstractcondition.h aistate.h checksevencondition.h \
	decisiontree.h iaction.h icondition.h jackonlycondition.h nextaction.h playjackaction.h \
	suspendaction.h

libaidt_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine
libaidt_la_CXXFLAGS = $(NO_RTTI_FLAGS)
libaidt_la_SOURCES = abstractaction.cpp abstractcondition.cpp aistate.cpp checksevencondition.cpp \
	decisiontree.cpp jackonlycondition.cpp nextaction.cpp playjackaction.cpp suspendaction.cpp
