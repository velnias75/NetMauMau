AC_DEFUN([AX_CHECK_BASE64],
[  AC_CHECK_PROGS([BASE64], [base64])
   if test -z "$ac_cv_prog_BASE64"; then
     AC_MSG_ERROR([base64 not found, please install])
   fi
   AC_MSG_CHECKING([if base64 supports -w0])
   B64_RES=`echo x 2>/dev/null | base64 -w0 2>/dev/null`
   if test "$B64_RES" == "eAo="; then
     AC_MSG_RESULT([yes])
     AC_SUBST([B64_TR], [cat])
     BASE64="$BASE64 -w0";
   else
     AC_MSG_RESULT([no])
     B64_TR="tr -d '\n' | tr -d '\r'";
     AC_SUBST([B64_TR])
   fi
])
