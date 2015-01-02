## Process this file with automake to produce Makefile.in

INCLUDES = \
	-I../../libhdhomerun \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = hdhomerun_config_gui

hdhomerun_config_gui_SOURCES = \
	main.cpp AppInclude.h \
	support.cpp support.h \
	interface.cpp interface.h \
	callbacks.cpp callbacks.h \
	Debug.cpp Debug.h \
	Devices.cpp Devices.h \
	Viewer.cpp Viewer.h \
	HDHRConfig.cpp HDHRConfig.h

install-exec-local:
	test -z "$(libdir)" || $(MKDIR_P) "$(DESTDIR)$(libdir)"
	$(INSTALL) ../../libhdhomerun/libhdhomerun.so $(DESTDIR)$(libdir)
	$(INSTALL) ../../libhdhomerun/hdhomerun_config $(DESTDIR)$(bindir)

uninstall-local:
	-rm -f "$(DESTDIR)$(libdir)/libhdhomerun.so"
	-rm -f "$(DESTDIR)$(bindir)/hdhomerun_config" 
	
hdhomerun_config_gui_LDADD = \
	-L../../libhdhomerun -lhdhomerun \
	@PACKAGE_LIBS@ $(INTLLIBS)

