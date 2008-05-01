
LATEX_CFLAGS=
LATEX_LIBS=

if test "$enable_latex" == "yes"; then

LATEX_CFLAGS="$LATEX_CFLAGS "'${PLUGIN_CFLAGS}'
LATEX_LIBS="$LATEX_LIBS "'${PLUGIN_LIBS}'

if test "$enable_latex_builtin" == "yes"; then
	LATEX_CFLAGS="$LATEX_CFLAGS -DABI_PLUGIN_BUILTIN"
fi

fi

AC_SUBST([LATEX_CFLAGS])
AC_SUBST([LATEX_LIBS])
