brightness
==========

A command-line utility to programmatically adjust the brightness of
displays on Mac OS X. Brightness is adjustable in this way
only on certain displays, typically the built-in displays of Macs.

Personally I find this useful when used together with display calibration -
a simple script can restore the display to the brightness setting used
to calibrate colours, allowing me to make rough adjustments via keyboard
commands (e.g., when playing games or watching movies) without worrying
about being able to go back to the calibrated setting.

There are other utilities for this, but most of them seem to use the
`CGDisplayIOServicePort()` function, which is deprecated as of OS X 10.9,
and/or they do not support display names.

~ [Kimmo Kulovesi](http://arkku.com/), 2014-07-18

Usage
=====

The default behaviour of `brightness` is to list all displays and
their brightness values. Specifying a brightness (typically a number
from 0.0 to 1.0) on the command-line sets the brightness of all
applicable displays to that number. The option `-d display` can be used
to name a specific display to adjust.

Example to set the display named `iMac` to brightness `0.5` (50%):

    brightness -d iMac 0.5

Run the program with the argument `-h` for help, or see the included
man page.

