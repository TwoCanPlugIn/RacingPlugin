Race Start Display for OpenCPN
==============================

A simple plugin to assist sailors with their race start.
Prior to the start, sail to each end of the start line and "ping" each end by pressing the port & starboard buttons respectively.
When the 5' gun sounds press the start button.
The display will then display a 5' countdown timer, the boat's speed over ground, and based on your current speed and heading, distance and time to the start line. Good luck !

Obtaining the source code
-------------------------

git clone https://github.com/twocanplugin/racing_pi.git


Build Environment
-----------------

Both Windows and Linux are currently supported.

This plugin builds outside of the OpenCPN source tree

For both Windows and Linux, refer to the OpenCPN developer manual for details regarding other requirements such as git, cmake and wxWidgets.

For Windows you must place opencpn.lib into the twocan_pi/build directory to be able to link the plugin DLL. opencpn.lib can be obtained from your local OpenCPN build, or alternatively downloaded from http://sourceforge.net/projects/opencpnplugins/files/opencpn_lib/

Build Commands
--------------
 mkdir racing_pi/build

 cd racing_pi/build

 cmake ..

 cmake --build . --config debug

  or

 cmake --build . --config release

Creating an installation package
--------------------------------
 cmake --build . --config release --target package

  or

 cpack

Installation
------------
Run the resulting setup package created above for your platform.

Eg. For Windows run racing\_pi\_1.0.0-ov50.exe

Problems
--------

Please send bug reports/questions/comments to the opencpn forum or via email to twocanplugin@hotmail.com

License
-------
The plugin code is licensed under the terms of the GPL v3 or, at your convenience, a later version.