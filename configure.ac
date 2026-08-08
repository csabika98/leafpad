AC_INIT(leafpad, 0.8.19, tarot@sdf.org)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

AC_PROG_INTLTOOL(0.31, no-xml)
AC_PROG_CC
if test "$GCC" = "yes"; then
	CFLAGS="$CFLAGS -Wall"
fi
AM_PROG_CC_C_O

PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.10.0)

dnl The menus are still built with GtkAction/GtkUIManager and stock ids, which
dnl GTK+ 3 deprecates but continues to ship. Silence the noise rather than
dnl rewrite the menu layer against GAction for a 3.x-only target.
CFLAGS="$CFLAGS -Wno-deprecated-declarations"

dnl GtkFileChooser and the GtkPrintOperation API are unconditional in GTK+ 3;
dnl GtkFileSelector and libgnomeprintui are both gone.
AC_DEFINE(ENABLE_CHOOSER, 1, [Define if GtkFileChooser is enabled.])

AC_ARG_ENABLE(print,
	AS_HELP_STRING(--disable-print, [force to disable print feature]))
if test "$enable_print" != "no"; then
	AC_DEFINE(ENABLE_PRINT, 1, [Define if print feature is enabled.])
fi

AC_ARG_ENABLE(emacs,
	AS_HELP_STRING(--enable-emacs, [implement Emacs key theme (experimental)]),
	AC_DEFINE(ENABLE_EMACS, 1, [Define if Emacs key theme is enabled.])
)

dnl macOS: gtk-mac-integration moves the menu bar to the system bar and maps
dnl Cmd accelerators. Optional; the build works without it.
AC_ARG_ENABLE(mac-integration,
	AS_HELP_STRING(--disable-mac-integration, [do not use gtk-mac-integration]))
if test "$enable_mac_integration" != "no"; then
	PKG_CHECK_MODULES(GTKMACINTEGRATION, gtk-mac-integration-gtk3,
		[AC_DEFINE(ENABLE_MAC_INTEGRATION, 1,
			[Define if gtk-mac-integration is available.])
		 have_mac_integration=yes],
		[have_mac_integration=no])
fi

GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(LOCALEDIR)

AC_OUTPUT([
	Makefile
	src/Makefile
	data/Makefile
	data/icons/Makefile
	data/icons/16x16/Makefile
	data/icons/22x22/Makefile
	data/icons/24x24/Makefile
	data/icons/32x32/Makefile
	data/icons/scalable/Makefile
	po/Makefile.in
])
