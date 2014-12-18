noinst_LTLIBRARIES = libeventhandler.la libengine_private.la libengine.la

noinst_HEADERS = talon.h engine.h stdcardfactory.h defaulteventhandler.h ieventhandler.h \
	stdruleset.h iruleset.h stdplayer.h italonchange.h

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif
	
libeventhandler_la_CXXFLAGS = -fno-exceptions -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include
libeventhandler_la_SOURCES = defaulteventhandler.cpp

libengine_private_la_CPPFLAGS = $(GSL)
libengine_private_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include $(GSL_CFLAGS)
libengine_private_la_SOURCES = engine.cpp stdplayer.cpp

libengine_la_CPPFLAGS = $(GSL)
libengine_la_CXXFLAGS = $(NO_RTTI_FLAGS) -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	$(GSL_CFLAGS)
libengine_la_SOURCES = talon.cpp stdcardfactory.cpp stdruleset.cpp
libengine_la_LIBADD = libengine_private.la libeventhandler.la ../common/libnetmaumaucommon.la \
	$(GSL_LIBS)
