Download:

MonoBrick.zip from http://www.monobrick.dk/software/monobrick/
unpack MonoBrick.zip to lib/ subfolder

Install:

sudo apt-get install mono-complete
sudo apt-get install monodevelop
sudo apt-get install libusb-1.0-0-dev
lsusb|grep Lego
 // see Lego...
sudo groupadd nxt
sudo usermod -a -G nxt <username>
put:
SUBSYSTEM=="usb", ATTRS{idVendor}=="0694", GROUP="nxt", MODE="0660"
into /etc/udev/rules.d/70-lego.rules
reboot

Run (both PC and Raspberry Pi):

export MONO_PATH=/home/pi/src/mikes-generic/mikes-common/nxt/lib:$MONO_PATH
export LD_LIBRARY_PATH=/home/pi/src/mikes-generic/mikes-common/nxt/lib:$MONO_PATH
make
./TestMonoBrick

