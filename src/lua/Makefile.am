noinst_LTLIBRARIES = libluaruleset.la

noinst_HEADERS = luaexception.h luaruleset.h luastate.h

pkgdata_DATA = stdrules.lua

EXTRA_DIST = $(pkgdata_DATA)

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libluaruleset_la_CPPFLAGS = $(GSL)
libluaruleset_la_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(LIBLUA_CFLAGS)
libluaruleset_la_SOURCES = luaexception.cpp luaruleset.cpp luastate.cpp
libluaruleset_la_LIBADD = $(LIBLUA_LIBS)
