## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = hexzenith

hexzenith_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	treeview.c treeview.h \
    centres.c centres.h \
    inscriptions.c inscriptions.h \
	equipement.c equipement.h \
	reservation.c reservation.h

hexzenith_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

