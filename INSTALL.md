HDHomeRun Config GTK instructions
=================================

Extract both libhdhomerun and hdhomerun_config_gui to the same directory, e.g.:

	directory/libhdhomerun
	directory/hdhomerun_config_gui

From the hdhomerun_config_gui directory, run:
	./configure
	make
	sudo make install


Troubleshooting
===============

During the ./configure step, if you receive an error about gtk+-2.0 not being
installed, you will need to install it using your distribution’s package
management system. On most Debian-based distributions, including Ubuntu, use
the following command:

	sudo apt-get install libgtk2.0-dev

If you receive an error stating hdhomerun_config_gui: error while loading
shared libraries: libhdhomerun.so: cannot open shared object file: No such file
or directory when attempting to run hdhomerun_config_gui, run the following:

	sudo ldconfig

