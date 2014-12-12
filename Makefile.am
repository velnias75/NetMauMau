SUBDIRS = src doc

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = COPYING netmaumau.ico

if CHECKCPP
# suppression template: // cppcheck-suppress {id}
cppcheck:
	$(CPPCHECK) $(srcdir)/src -q --inline-suppr --enable=style,performance \
		--std=posix --std=c++03 --template="{file}:{line}: {severity}: {message}" \
		--inconclusive -I$(builddir) -I$(srcdir)/src/include -I$(srcdir)/src/common \
		-I$(srcdir)/src/engine -I$(srcdir)/src/server -I$(srcdir)/src/client \
		-I$(srcdir)/src/test -i$(srcdir)/src/test
endif
