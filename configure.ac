#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([NetMauMau], [0.24.5], [heiko@rangun.de], [netmaumau], [https://github.com/velnias75/NetMauMau])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AM_INIT_AUTOMAKE([foreign dist-xz no-dist-gzip tar-pax no-installinfo no-texinfo.tex])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])
LT_INIT
LT_LANG([Windows Resource])

build_host="$build_cpu-$build_vendor-$build_os"
if test "$build_host" = "--" ; then
  build_host=`uname -m`-`uname -s`
fi

build_target="$host_cpu-$host_vendor-$host_os"
if test "$build_target" = "--" ; then
  build_target=`${CC} -dumpmachine`
fi

AC_SUBST([BUILD_YEAR], [`date +%Y`])
AC_DEFINE_UNQUOTED([BUILD_DATE], [`date +%s`], [the build date in Unix time])
AC_DEFINE_UNQUOTED([BUILD_NODE], ["`uname -n`"], [the build node])
AC_DEFINE_UNQUOTED([BUILD_HOST], ["$build_host"], [the build host])
AC_DEFINE_UNQUOTED([BUILD_TARGET], ["$build_target"], [the build target])

AC_DEFINE([SERVER_PORT], [8899], [the default server port])
AC_DEFINE([HTTPD_PORT], [9000], [the default httpd port])

AC_ARG_ENABLE([ansi], [AS_HELP_STRING([--disable-ansi], [disables ANSI support])],
  [ if test x$enable_ansi = xno; then
      CPPFLAGS="${CPPFLAGS} -DDISABLE_ANSI"
    fi
  ])

AC_ARG_ENABLE([client], [AS_HELP_STRING([--disable-client], [disables build of client library])],
  [ AM_CONDITIONAL([ENABLE_CLIENT], [ test x$enable_client = xyes ]) ],
  [ AM_CONDITIONAL([ENABLE_CLIENT], [ true ]) ])

AC_ARG_ENABLE([console-client], [AS_HELP_STRING([--enable-console-client],
  [enables build of console client])],
  [ AM_CONDITIONAL([ENABLE_CLI_CLIENT], [ test x$enable_console_client = xyes ]) ],
  [ AM_CONDITIONAL([ENABLE_CLI_CLIENT], [ false ]) ])

AC_ARG_ENABLE([apidoc], [AS_HELP_STRING([--disable-apidoc], [disables generation of API docs])],
  [ AM_CONDITIONAL([GENDOX], [ test x$enable_apidoc = xyes ]) ],
  [ AM_CONDITIONAL([GENDOX], [ true ]) ])

AC_ARG_ENABLE([ai-name], [AS_HELP_STRING([--enable-ai-name], [default name of AI player])])

if test ! -z "$enable_ai_name" -a "x$enable_ai_name" != "xno"; then
  AC_DEFINE_UNQUOTED([AI_NAME], ["$enable_ai_name"], [default name of AI player])
else
  AC_DEFINE([AI_NAME], ["Computer"], [default name of AI player])
fi

AC_ARG_ENABLE([ai-image], [AS_HELP_STRING([--enable-ai-image], [path to image of AI player])],
  [ AM_CONDITIONAL([BRANDING], [ test x$enable_ai_image != xno ]) ],
  [ AM_CONDITIONAL([BRANDING], [ false ]) ])

AC_ARG_ENABLE([ai-tracing], [AS_HELP_STRING([--enable-ai-tracing],
  [enables tracing of AI decisions (useful for developers only, needs NDEBUG undefined)])],
  [  if test x$enable_ai_tracing = xyes; then
       AC_DEFINE([TRACE_AI], [1], [Define to 1 to enable tracing of AI decisions])
     fi
  ])

if test ! -z "$enable_ai_image" -a "x$enable_ai_image" != "xno"; then
  AC_SUBST([AI_IMAGE], [$enable_ai_image])
else
  AC_SUBST([AI_IMAGE], [\$\(top_srcdir\)/src/images/KS.PNG])
fi

AC_ARG_ENABLE([pidfile], [AS_HELP_STRING([--enable-pidfile], [enables creation of pid file])],
  [ AM_CONDITIONAL([PIDFILE], [ test x$enable_pidfile = xyes ]) ],
  [ AM_CONDITIONAL([PIDFILE], [ false ]) ])

AC_ARG_ENABLE([xinetd], [AS_HELP_STRING([--disable-xinetd], [disables creation and installation of the xinetd service])],
  [ AM_CONDITIONAL([XINETD], [ test x$enable_xinetd = xyes ]) ],
  [ AM_CONDITIONAL([XINETD], [ true ]) ])

AC_ARG_ENABLE([webserver], [AS_HELP_STRING([--disable-webserver], [disables the integrated webserver])],
  [ if test x$enable_webserver = xyes; then NOHTTPD=0; else NOHTTPD=1; fi ], [ NOHTTPD=0 ])

AC_ARG_ENABLE([threads], [AS_HELP_STRING([--disable-threads], [disables threading support])],
  [ if test x$enable_threads = xno; then
      AM_CONDITIONAL([THREADS_ENABLED], [ false ])
	  if test $NOHTTPD -eq 0 ; then
		AC_MSG_WARN([disabling threads with enabled webserver is *NOT* recommended])
	  fi
    fi
  ])

if test x$enable_threads = x -o x$enable_threads = xyes; then
  AM_CONDITIONAL([THREADS_ENABLED], [ true ])
  AC_DEFINE([ENABLE_THREADS], [1], [Define to 1 to enable threading support])
fi

PKG_PROG_PKG_CONFIG([0.22])

# Checks for programs.
AC_PROG_CXX
AX_CHECK_CLANG
AC_PROG_LN_S
AX_CHECK_BASE64
AC_CHECK_PROGS([HELP2MAN], [help2man])
AC_CHECK_PROGS([DOXYGEN],  [doxygen])
AC_CHECK_PROGS([CPPCHECK], [cppcheck])
AC_CHECK_PROGS([DEHEADER], [deheader])

if test -z "$ac_cv_prog_HELP2MAN"; then
  AM_CONDITIONAL([GENMAN], [ false ])
else
  AM_CONDITIONAL([GENMAN], [ true ])
fi

if test -z "$ac_cv_prog_DOXYGEN"; then
  AM_CONDITIONAL([GENDOX], [ false ])
fi

if test -z "$ac_cv_prog_CPPCHECK"; then
  AM_CONDITIONAL([CHECKCPP], [ false ])
else
  AM_CONDITIONAL([CHECKCPP], [ true ])
fi

if test -z "$ac_cv_prog_DEHEADER"; then
  AM_CONDITIONAL([ENABLE_DEHEADER], [ false ])
else
  AM_CONDITIONAL([ENABLE_DEHEADER], [ true ])
fi

if test -z "$ac_cv_prog_RC"; then
  AM_CONDITIONAL([RC], [ false ])
else
  AM_CONDITIONAL([RC], [ true ])
fi

if test x$enable_console_client = xyes; then
  AM_CONDITIONAL([ENABLE_CLIENT], [ true ])
fi

# Checks for libraries.
CHECK_ZLIB

AM_CONDITIONAL([WITHZLIB], [ test "${WANT_ZLIB}" != no -a -n "${ZLIB_HOME}" ])

PKG_CHECK_MODULES([POPT], [popt >= 1.10])
AC_CHECK_LIB([magic], [magic_open],
  [ AC_SUBST(DOCMAGIC, "libmagic")
	AC_DEFINE([HAVE_LIBMAGIC], [1], [Define to 1 if you have the `magic' library (-lmagic).])
	AC_SUBST([MAGIC_LIBS], [-lmagic]) ],
  [ AC_SUBST(DOCMAGIC, "") ])
AC_CHECK_LIB([rt], [timer_create],
  [ AC_DEFINE([HAVE_LIBRT], [1], [Define to 1 if you have the `rt' library (-lrt).])
	AC_SUBST([RT_LIBS], [-lrt]) ])
PKG_CHECK_MODULES([GSL], [gsl >= 1.9],
  [ AM_CONDITIONAL([GSL], [ true ])
    AC_SUBST(DOCGSL, "gsl")
	echo "[[ENVIRONMENT]]
.TP
\fBGSL_RNG_TYPE\fR
the name of a generator, such as ranlxs2, taus or mt19937
.TP
\fBGSL_RNG_SEED\fR
the desired seed value
.PP
See the
.mso www.tmac
.URL \"http://www.gnu.org/software/gsl/manual/html_node/Random-number-environment-variables.html\" \"GSL Manual\" \" for more information\"" > gsl.h2m ],
  [ AM_CONDITIONAL([GSL], [ false ])] )
PKG_CHECK_MODULES([LIBSQLITE], [sqlite3 >= 3.4.2])
PKG_CHECK_MODULES([LIBLUA], [lua5.3 >= 5.3.3], [ AC_SUBST([PKGC_LUA], [lua5.3]) ],
  [ PKG_CHECK_MODULES([LIBLUA], [lua53 >= 5.3.3], [ AC_SUBST([PKGC_LUA], [lua53]) ],
  [ PKG_CHECK_MODULES([LIBLUA], [lua >= 5.3.3], [AC_SUBST([PKGC_LUA], [lua])] ) ] ) ])
PKG_CHECK_MODULES([LIBMICROHTTPD], [libmicrohttpd >= 0.4.6],
				  [ if test $NOHTTPD -eq 0; then
				      AM_CONDITIONAL([MICROHTTPD], [ true ])
					  AC_DEFINE([HAVE_LIBMICROHTTPD], [1], [Define to 1 if you have libmicrohttpd.])
					else
					  AM_CONDITIONAL([MICROHTTPD], [ false ])
					fi ], [ AM_CONDITIONAL([MICROHTTPD], [ false ]) ])

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([sys/wait.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([magic.h])
AC_CHECK_HEADERS([poll.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/syscall.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_GCC_ABI_DEMANGLE
AX_CHECK_COMPILE_FLAG([-fvisibility=internal],
  [ VISIBILITY="internal" ], [ VISIBILITY="hidden" ])
AX_CHECK_COMPILE_FLAG([-Wa,--noexecstack],
  [ NOEXECSTACK_AS="-Wa,--noexecstack" ])
AX_CHECK_LINK_FLAG([-Wl,-z,noexecstack],
  [ NOEXECSTACK_LD="-Wl,-z,noexecstack" ])
AX_CHECK_LINK_FLAG([-funsafe-math-optimizations],
  [ FASTMATH_LD="-funsafe-math-optimizations" ])
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_C_INLINE

# Checks for library functions.
AC_LANG_PUSH([C++])
AS_IF([test "x$cross_compiling" != "xyes"], [
  AC_FUNC_MALLOC
  AC_FUNC_REALLOC
])
AC_CHECK_FUNCS([raise])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([strncasecmp])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([initstate])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strndup])
AC_CHECK_FUNCS([strtoul])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([pselect])
AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([confstr])
AC_CHECK_FUNC([arc4random_uniform],
  [ AC_DEFINE([HAVE_ARC4RANDOM_UNIFORM], [1], [we have arc4random_uniform])
])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_CHOWN
AC_LANG_POP([C++])

# backwards compat with older pkg-config
# - pull in AC_DEFUN from pkg.m4
m4_ifndef([PKG_CHECK_VAR], [
# PKG_CHECK_VAR(VARIABLE, MODULE, CONFIG-VARIABLE,
# [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------------------------------------
# Retrieves the value of the pkg-config variable for the given module.
AC_DEFUN([PKG_CHECK_VAR],
[AC_REQUIRE([PKG_PROG_PKG_CONFIG])dnl
AC_ARG_VAR([$1], [value of $3 for $2, overriding pkg-config])dnl

_PKG_CONFIG([$1], [variable="][$3]["], [$2])
AS_VAR_COPY([$1], [pkg_cv_][$1])

AS_VAR_IF([$1], [""], [$5], [$4])dnl
])# PKG_CHECK_VAR
])

AM_CONDITIONAL([INSTALL_BASHCOMP], [ false ])

AC_ARG_WITH([bashcompletiondir], [AS_HELP_STRING([--with-bashcompletiondir=DIR],
  [directory to install bash completion files into])],
  [ if test "x$with_bashcompletiondir" != "xno"; then
      AM_CONDITIONAL([INSTALL_BASHCOMP], [ true ])
      bashcompdir="$with_bashcompletiondir"
	fi ],
  [  AM_CONDITIONAL([INSTALL_BASHCOMP], [ true ])
     PKG_CHECK_VAR(bashcompdir, [bash-completion], [completionsdir], ,
      bashcompdir="${sysconfdir}/bash_completion.d")
  ])

AC_SUBST([bashcompdir])

SERVER_VERSION_MAJOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f1`"
SERVER_VERSION_MINOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f2`"
SERVER_VERSION_RELEASE="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f3`"

AC_SUBST([SERVER_VERSION_MAJOR])
AC_SUBST([SERVER_VERSION_MINOR])
AC_SUBST([SERVER_VERSION_RELEASE])

NMM_CLIENTVERSION([0],[15])

AC_DEFINE([MAXPICBYTES], [1048576], [maximum size of player image in bytes (base64 encoded)])

AC_DEFINE_UNQUOTED([SERVER_VERSION_MAJOR], [${SERVER_VERSION_MAJOR}], [server version major])
AC_DEFINE_UNQUOTED([SERVER_VERSION_MINOR], [${SERVER_VERSION_MINOR}], [server version minor])
AC_DEFINE_UNQUOTED([SERVER_VERSION_RELEASE], [${SERVER_VERSION_RELEASE}], [server version release])

LDFLAGS="${NOEXECSTACK_LD} ${FASTMATH_LD} -Wl,--as-needed ${LDFLAGS}"
CFLAGS="${CFLAGS} ${NOEXECSTACK_AS} -std=gnu99 -ffast-math -fvisibility=${VISIBILITY} -fvisibility-inlines-hidden -fstrict-aliasing"
CXXFLAGS="${CXXFLAGS} ${NOEXECSTACK_AS} -std=gnu++98 -ffast-math -fvisibility=${VISIBILITY} -fvisibility-inlines-hidden -fstrict-aliasing"
CPPFLAGS="${CPPFLAGS} -D_GLIBCXX_VISIBILITY=0"

if test x$enable_threads = x -o x$enable_threads != xyes; then
  CPPFLAGS="${CPPFLAGS} -DBASICLOGGER_NO_PTHREADS"
fi

NO_EXCEPTIONS="-fno-exceptions"

if test x$CLANG = xyes; then CXXFLAGS="-fexceptions -frtti ${CXXFLAGS}" ; fi

AC_SUBST([LDFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([NO_EXCEPTIONS])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([src/server/netmaumau.rc doc/client.dox doc/engine.dox])

AC_MSG_RESULT([You may want to install a client too: https://github.com/velnias75/NetMauMau-Qt-Client])

AC_OUTPUT([Makefile doc/Makefile src/Makefile src/ai/Makefile src/client/Makefile
  src/common/Makefile src/engine/Makefile src/images/Makefile src/include/Makefile
  src/lua/Makefile src/server/Makefile src/sqlite/Makefile src/test/Makefile])
