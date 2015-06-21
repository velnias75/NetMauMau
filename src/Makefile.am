SUBDIRS = include images common sqlite lua ai engine server client test

if ENABLE_DEHEADER
deheader:
	@echo "Running deheader with DEHEADER_FLAGS=\"$(DEHEADER_FLAGS)\" …" ;
	@cd $(abs_top_srcdir)/src && \
	deheader $(DEHEADER_FLAGS) -m "$(MAKE) CXXFLAGS='$(LIBLUA_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -DHAVE_GSL=1 \
		-I$(abs_top_srcdir)/src/ai -I$(abs_top_srcdir)/src/client -I$(abs_top_srcdir)/src/common \
		-I$(abs_top_srcdir)/src/engine -I$(abs_top_srcdir)/src/images -I$(abs_top_srcdir)/src/include \
		-I$(abs_top_srcdir)/src/lua -I$(abs_top_srcdir)/src/server -I$(abs_top_srcdir)/src/sqlite \
		-I$(abs_top_srcdir)/src/test -I$(abs_top_builddir) -DNMM_EXE_NAME=\\\".\\\" \
		-DNMM_EXE_PATH=\\\".\\\" -DSYSCONFDIR=\\\".\\\" -DLUADIR=\\\".\\\" -DCLIENTVERSION=15 \
		-DBUILDDIR=\\\".\\\" -DPKGDATADIR=\\\".\\\" -I$(abs_top_builddir)/src/common \
		-DHAVE_CONFIG_H=1 -I$(abs_top_builddir)/src/test -Werror -Wfatal-errors'"
endif
