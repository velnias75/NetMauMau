AC_DEFUN([AX_CHECK_CLANG], [
   AC_MSG_CHECKING([if compiling with clang])
   AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([], [[
#ifndef __clang__
       not clang
#endif
   ]])], [CLANG=yes], [CLANG=no])
AC_MSG_RESULT([$CLANG])])
