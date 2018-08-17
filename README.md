# mikes-common
Library of all common modules, and tools shared in all Mikes projects

== Prerequisities ==

* libcairo2-dev
* lib64ncurses5-dev
* librsvg2-dev
* libxml2-dev
* rplidar_sdk (https://download.slamtec.com/api/download/rplidar-sdk/1.7.0?lang=netural)


== Installation ==

In order for the arduino base module, rplidar and asus xtion to be recognized
and assigned the expected symbolic links in the /dev/ file system,
you may add the following files to your system at /etc/udev/rules.d/


file /etc/udev/rules.d/10-base.rules

# recognize base module (Arduino) and put it at /dev/base
ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", SYMLINK="base"


file /etc/udev/rules.d/11-lidar.rules
# recognize RPLIDAR and put it at /dev/lidar
ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", SYMLINK="lidar"


file /etc/udev/rules.d/55-primesense-usb.rules
# Make primesense device mount with writing permissions (default is read only for unknown devices)
SUBSYSTEM=="usb", ATTR{idProduct}=="0200", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0300", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0401", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0500", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0600", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0601", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="0609", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="1280", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="2100", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="2200", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
SUBSYSTEM=="usb", ATTR{idProduct}=="f9db", ATTR{idVendor}=="1d27", MODE:="0666", OWNER:="root", GROUP:="video"
