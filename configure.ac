#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([NetMauMau], [0.2], [heiko@rangun.de], [netmaumau], [https://github.com/velnias75/NetMauMau])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])
LT_INIT

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

PKG_PROG_PKG_CONFIG([0.22])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES([POPT], [popt >= 1.10]);

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([poll.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_GCC_ABI_DEMANGLE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UID_T
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNC([arc4random_uniform], 
  [ AC_DEFINE([HAVE_ARC4RANDOM_UNIFORM], [1], [we have arc4random_uniform]) 
])
AC_FUNC_ERROR_AT_LINE

SERVER_VERSION_MAJOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f1`"
SERVER_VERSION_MINOR="`echo ${PACKAGE_VERSION} | tr -s '~.' '\t' | cut -f2`"

AC_SUBST([SERVER_VERSION_MAJOR])
AC_SUBST([SERVER_VERSION_MINOR])

AC_DEFINE_UNQUOTED([SERVER_VERSION_MAJOR], [${SERVER_VERSION_MAJOR}], [server version major])
AC_DEFINE_UNQUOTED([SERVER_VERSION_MINOR], [${SERVER_VERSION_MINOR}], [server version minor])

LDFLAGS="-Wl,--as-needed ${LDFLAGS}"
CFLAGS="${CFLAGS} -std=gnu99 -fvisibility-inlines-hidden"
CXXFLAGS="${CXXFLAGS} -std=gnu++98 -fvisibility=internal -fvisibility-inlines-hidden"
CPPFLAGS="${CPPFLAGS} -DBASICLOGGER_NO_PTHREADS"
NO_RTTI_FLAGS="-fno-rtti -fno-exceptions"

AC_SUBST([LDFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([NO_RTTI_FLAGS])

AC_CONFIG_MACRO_DIR([m4])
AC_OUTPUT([Makefile src/Makefile src/common/Makefile src/engine/Makefile src/server/Makefile \
  src/client/Makefile src/test/Makefile])
