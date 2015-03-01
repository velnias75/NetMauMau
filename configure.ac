#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([NetMauMau], [0.16], [heiko@rangun.de], [netmaumau], [https://github.com/velnias75/NetMauMau])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AM_INIT_AUTOMAKE([foreign dist-xz no-dist-gzip tar-pax no-installinfo no-texinfo.tex])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([config.h])
LT_INIT
LT_LANG([Windows Resource])

AC_SUBST([BUILD_YEAR], [`date +%Y`])
AC_DEFINE_UNQUOTED([BUILD_DATE], [`date +%s`], [the build date in Unix time])
AC_DEFINE_UNQUOTED([BUILD_NODE], ["`uname -n`"], [the build node])
AC_DEFINE_UNQUOTED([BUILD_HOST], ["$build_cpu-$build_vendor-$build_os"], [the build host])
AC_DEFINE_UNQUOTED([BUILD_TARGET], ["$host_cpu-$host_vendor-$host_os"], [the build target])

AC_DEFINE([SERVER_PORT], [8899], [the default server port])

AC_ARG_ENABLE([ansi], [AS_HELP_STRING([--disable-ansi], [disables ANSI support])],
  [ if test x$enable_ansi = xno; then
      CPPFLAGS="${CPPFLAGS} -DDISABLE_ANSI"
    fi
  ])
AC_ARG_ENABLE([client], [AS_HELP_STRING([--disable-client], [disables build of client library])],
  [ AM_CONDITIONAL([ENABLE_CLIENT], [ test x$enable_client = xyes ]) ],
  [ AM_CONDITIONAL([ENABLE_CLIENT], [ true ]) ])

AC_ARG_ENABLE([cli-client], [AS_HELP_STRING([--enable-cli-client], [enables build of CLI client])],
  [ AM_CONDITIONAL([ENABLE_CLI_CLIENT], [ test x$enable_cli_client = xyes ]) ],
  [ AM_CONDITIONAL([ENABLE_CLI_CLIENT], [ false ]) ])

AC_ARG_ENABLE([apidoc], [AS_HELP_STRING([--disable-apidoc], [disables generation of API docs])],
  [ AM_CONDITIONAL([GENDOX], [ test x$enable_apidoc = xyes ]) ],
  [ AM_CONDITIONAL([GENDOX], [ true ]) ])

AC_ARG_ENABLE([ai-name], [AS_HELP_STRING([--enable-ai-name], [default name of AI player])])

if test "x$enable_ai_name" != "xno"; then
  AC_DEFINE_UNQUOTED([AI_NAME], ["$enable_ai_name"], [default name of AI player])
else
  AC_DEFINE([AI_NAME], ["Computer"], [default name of AI player])
fi

AC_ARG_ENABLE([ai-image], [AS_HELP_STRING([--enable-ai-image], [path to image of AI player])],
  [ AM_CONDITIONAL([BRANDING], [ test x$enable_ai_image != xno ]) ],
  [ AM_CONDITIONAL([BRANDING], [ false ]) ])

if test "x$enable_ai_image" != "xno"; then
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

PKG_PROG_PKG_CONFIG([0.22])

# Checks for programs.
AC_PROG_CXX
AX_CHECK_CLANG
AC_PROG_LN_S
AX_CHECK_BASE64
AC_CHECK_PROGS([HELP2MAN], [help2man])
AC_CHECK_PROGS([DOXYGEN],  [doxygen])
AC_CHECK_PROGS([CPPCHECK], [cppcheck])

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

if test -z "$ac_cv_prog_RC"; then
  AM_CONDITIONAL([RC], [ false ])
else
  AM_CONDITIONAL([RC], [ true ])
fi

if test x$enable_cli_client = xyes; then
  AM_CONDITIONAL([ENABLE_CLIENT], [ true ])
fi

# Checks for libraries.
PKG_CHECK_MODULES([POPT], [popt >= 1.10])
AC_CHECK_LIB([magic], [magic_open])
AC_CHECK_LIB([rt], [timer_create])
PKG_CHECK_MODULES([GSL], [gsl >= 1.9],
  [ AM_CONDITIONAL([GSL], [ true ]) 
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

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([magic.h])
AC_CHECK_HEADERS([poll.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_GCC_ABI_DEMANGLE
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
AS_IF([test "x$cross_compiling" != "xyes"], [
  AC_FUNC_MALLOC
  AC_FUNC_REALLOC
])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([initstate])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strtoul])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNC([arc4random_uniform], 
  [ AC_DEFINE([HAVE_ARC4RANDOM_UNIFORM], [1], [we have arc4random_uniform]) 
])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_CHOWN

SERVER_VERSION_MAJOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f1`"
SERVER_VERSION_MINOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f2`"

AC_SUBST([SERVER_VERSION_MAJOR])
AC_SUBST([SERVER_VERSION_MINOR])

AC_DEFINE([MAXPICBYTES], [1048576], [maximum size of player image in bytes (base64 encoded)])

AC_DEFINE_UNQUOTED([SERVER_VERSION_MAJOR], [${SERVER_VERSION_MAJOR}], [server version major])
AC_DEFINE_UNQUOTED([SERVER_VERSION_MINOR], [${SERVER_VERSION_MINOR}], [server version minor])

if test x$CLANG = xyes; then VISIBILITY=hidden; else VISIBILITY=internal; fi

LDFLAGS="-Wl,--as-needed ${LDFLAGS}"
CFLAGS="${CFLAGS} -std=gnu99 -fvisibility-inlines-hidden"
CXXFLAGS="${CXXFLAGS} -std=gnu++98 -fvisibility=${VISIBILITY} -fvisibility-inlines-hidden -fstrict-aliasing"
CPPFLAGS="${CPPFLAGS} -DBASICLOGGER_NO_PTHREADS -D_GLIBCXX_VISIBILITY=0"
NO_EXCEPTIONS="-fno-exceptions" ;
NO_RTTI_FLAGS="-fno-rtti -fno-exceptions" ;
  
if test x$CLANG = xyes; then CXXFLAGS="-fexceptions -frtti ${CXXFLAGS}" ; fi

AC_SUBST([LDFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([NO_RTTI_FLAGS])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_FILES([src/server/netmaumau.rc doc/client.dox netmaumau.h2m])

AC_MSG_RESULT([You may want to install a client too: https://github.com/velnias75/NetMauMau-Qt-Client])

AC_OUTPUT([Makefile src/images/Makefile src/include/Makefile src/Makefile src/common/Makefile 
  src/sqlite/Makefile src/engine/Makefile src/server/Makefile src/client/Makefile src/test/Makefile 
  doc/Makefile])
