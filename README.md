xf86-input-libinput - a libinput-based X driver
===============================================

The official repository for this driver is

  https://github.com/X11Libre/xf86-input-libinput

This is an X driver based on libinput. It is a thin wrapper around libinput,
so while it does provide all features that libinput supports it does little
beyond.

***WARNING: misconfiguration of an X input driver may leave you without
usable input devices in your X session. Use with caution.***


Building
--------

To build this driver:

    autoreconf -vif
    ./configure --prefix=$HOME/build
    make && make install

Note that this assumes the same prefix as used in "Building the X Window
System" above, adjust as required. If you want a system install, use a
prefix of */usr*.

Install the default configuration file:

    cp conf/99-libinput.conf /etc/X11/xorg.conf.d/

This will assign this driver to *all* devices. Use with caution.
