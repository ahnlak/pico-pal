PicoPal
=======

PicoPal is an additional library of useful Pico stuff, gathered together in
one place.

This was mainly done because of a current lack of Pico-ready C(++) libraries
for the various peripherals I've been playing with; there's a lot of code
scattered around Github and elsewhere, but not always in the most useful form
(or even the right language!), so I wanted to get things more organised.

The aims of this library are to produce discrete blocks of functionality that:

* do one thing, but do it well
* are as small and fast as possible
* are properly documented

Each discrete block is offered as a 'sublibrary' in it's own directory, and exposed
as a standalone INTERFACE library to CMake; this is so that you can easily include
only the elements you need into your code in the same way as with the Pico SDK.


Usage
-----

Using the library should be a simple case of adding

    add_subdirectory(/path/to/pico-pal build)

to your project's `CMakeLists.txt` file, and then making sure that the relavent
sublibrary is included in your `target_link_libraries` section. All these
sublibraries are prefixed with `pal-` (and the classes lurk in the `PAL` namespace)
to try and avoid any name clashes.


Sublibraries
------------

`pal-ssd1306` is a driver for I2C SSD1306-based monochrome OLED displays.

