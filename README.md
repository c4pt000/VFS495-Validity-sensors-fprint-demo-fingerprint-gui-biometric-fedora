# REALLY,REALLY,REALLY-> (with VFS495) BUGGY **WIP


# setenforce -> 0 (unless you train SELinux to catch all the permissions errors to allow lightdm, the sensor, dbus, /user/.fprint still working on a SELinux auto label to allow these fprint functions within "enforcing"

requires
```
pam_passwdqc-1.4.0-3.fc34.x86_64
gnome-keyring-pam-40.0-1.fc34.x86_64
systemd-pam-248.2-1.fc34.x86_64
pam-devel-1.5.1-5.fc34.x86_64
fprintd-pam-1.90.9-2.fc34.x86_64
pam_fprint-0.2-1.x86_64
pam-1.5.1-5.fc34.x86_64


authselect-libs-1.2.3-1.fc34.x86_64
authselect-1.2.3-1.fc34.x86_64
authselect-compat-1.2.3-1.fc34.x86_64

```
```
wget https://github.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora/raw/master/pam.d.tar.gz-FPRINT-current-lightdm-sudo-xscreensaver.tar.gz
```
```
wget https://github.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora/raw/master/authselect-FPRINT-current-lightdm-sudo-xscreensaver.tar.gz
```
/etc/pam.d
/etc/authselect

restart script for VFS495 sensor for crond
<br>
since sensor times out and has weird issues for VFS495 
<br>
so far with
<br>
Bus 001 Device 031: ID 138a:003f Validity Sensors, Inc. VFS495 Fingerprint Reader
<br>



* crond script to autorun restart for VFS495 sensor for fingerprint sensor to be more reactive to work
* script runs continuously every 5 minutes restarting the systemctl function to load the usb fingerprint module 
* since once the systemctl starts "once" the usb module can timeout and lose functionality causing issues where the usb module has to be
* reloaded hence this crond script to reload the usb module every 5 minutes,

```
export EDITOR=nano
crontab -e
```
```
@reboot                                /usr/bin/restart-fingerprint.sh &
*/15    *   *   *   *                                  /usr/bin/restart-fingerprint.sh &

```
<br>
cat /usr/bin/restart-fingerprint.sh 
<br>
chmod +x /usr/bin/restart-fingerprint.sh
<br>
/usr/bin/restart-fingerprint.sh &

```
#!/bin/bash
systemctl stop vcsFPServiceDaemon
systemctl daemon-reload
systemctl start vcsFPServiceDaemon

```



xscreensaver fprintd support
https://github.com/c4pt000/xscreensaver-fingerprint-atomic

to disable fingerprint functions in system as a regular user move the .fprint dir out of the way
<br>
$ mv /home/c4pt/.fprint /home/c4pt/.fprint.null
<br>
see restart-fingerprint.sh script








# Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora
revitalized fprint_demo-gui for fprintd-enroll gui (dinosaurs from the dead of fedora ~20)

![s1](https://raw.githubusercontent.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora/master/finger-detect-image.png)
![s1](https://raw.githubusercontent.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora/master/success-enroll.png)

# * dont use Validity-Sensor-Setup-4.5-136.0.x86_64.rpm creates more bugs?


then sh install.sh must install on openssl 1.1.1k



install.sh as root "chmod +x install.sh"

```
#!/bin/bash

yum reinstall redhat-lsb-core-4.1-52.fc34.x86_64 -y
yum install fprintd-pam-1.90.9-2.fc34.x86_64

cd /opt
git clone https://github.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora
cd Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora
cd RPMS

export LC_ALL=C >> /root/.bashrc
source /root/.bashrc

ln -s /usr/lib64/libcrypto.so.1.1 /usr/lib64/libcrypto.so.0.9.8
ln -s /usr/lib64/libssl.so.1.1 /usr/lib64/libssl.so.0.9.8

rpm2cpio libfprint-0.0.6-2.x86_64.rpm | cpio -idmv /
ldconfig

rpm -Uvh pam_fprint-0.2-1.x86_64.rpm
ln -s /usr/lib/security/pam_fprint.so /usr/lib64/security/pam_fprint.so
cp -rf lib/security/pam_fprint.so /usr/lib64/
mkdir /usr/lib/security
cp -rf lib/security/pam_fprint.so /usr/lib/security/pam_fprint.so



rpm -Uvh --force Validity-Sensor-Setup-4.5-118.00.x86_64.rpm
rm -rf /etc/rc.d/init.d/vcsFPServiceDaemon 
cp -rf vcsFPServiceDaemon /etc/rc.d/init.d/vcsFPServiceDaemon
systemctl enable vcsFPServiceDaemon
systemctl start vcsFPServiceDaemon



echo "auth      sufficient pam_fprint.so" >>  /etc/pam.d/sudo
echo "auth      sufficient pam_fprintd.so" >>  /etc/pam.d/sudo

```

/etc/pam.d/sudo
---------------------
```
auth		sufficient  	pam_fprint.so
auth		sufficient  	pam_fprintd.so


auth       include      system-auth
account    include      system-auth
password   include      system-auth
session    optional     pam_keyinit.so revoke
session    required     pam_limits.so
session    include      system-auth

--------------------------------------





```


```
systemctl stop vcsFPServiceDaemon
systemctl start vcsFPServiceDaemon
```

restart script
<br>
nano /usr/bin/restart-fingerprint.sh
<br>
chmod +x /usr/bin/restart-fingerprint.sh
<br>

restart-fingerprint.sh
----------------------
```
systemctl daemon-reload
systemctl stop vcsFPServiceDaemon
systemctl start vcsFPServiceDaemon
```

command line enroll cli:
<br>
```
pam_fprint_enroll
```
gui based enroll:
```
fprint_demo
```

```
lsusb 
```
Bus 001 Device 006: ID 138a:003f Validity Sensors, Inc. VFS495 Fingerprint Reader
```
<br>
* allow regular user to access fingerprint sensor replace <Your User Name> with username of user

```
groupadd fingerprint
usermod -a -G fingerprint <Your User Name>
usermod -a -G input <Your User Name>


```
/etc/udev/rules.d/45-fingerprint.rules

your vendor id, device id
-----------------------------------

SUBSYSTEM=="usb_device", ACTION=="add",
SYSFS{idVendor}=="138a" ,
SYSFS{idProduct}=="003f",
SYMLINK+="fingerprint-%k",
GROUP="fingerprint",
MODE="666"

KERNEL=="uinput", MODE="666", GROUP="fingerprint"

```


<br>
<br>
<br>
<br>









# really buggy implementation

requires restarting vcsFPService

```
systemctl stop vcsFPServiceDaemon
systemctl start vcsFPServiceDaemon
```
cli:

pam_fprint_enroll

gui:
fprint_demo



```
rpm -qa | grep fprint && rpm -qa | grep Val

fprint-demo-0.4-2.x86_64
pam_fprint-0.2-1.x86_64
Validity-Sensor-Setup-4.5-118.00.x86_64
```



<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>


This project aims on reverse engineering protocol of Validity 138a:0090, 138a:0094, 138a:0097, 06cb:0081, 06cb:009a fingerprint readers, creating specification and FLOSS libfprint driver.

## Discussions

Main chat of this project: [![Gitter](https://img.shields.io/gitter/room/nwjs/nw.js.svg)](https://gitter.im/Validity90/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

libfrprint issue: [https://gitlab.freedesktop.org/libfprint/libfprint/issues/54](https://gitlab.freedesktop.org/libfprint/libfprint/issues/54)

Lenovo forums: [https://forums.lenovo.com/t5/Linux-Discussion/Validity-Fingerprint-Reader-Linux/td-p/3352145](https://forums.lenovo.com/t5/Linux-Discussion/Validity-Fingerprint-Reader-Linux/td-p/3352145)

## Notable files

1. [spec.md](spec.md) - Specification draft, the main work goes here right now.
2. [dissector.lua](dissector.lua) - Wireshark dissector for decrypting communication after key exchange.
3. ~[libfprint directory](libfprint) - libfprint repo with this driver integrated~ Not ready at the moment.
4. [prototype](prototype) - Standalone prototype(extremly ugly code, would be completly rewritten for driver)

## Testing

[Prototype](prototype) testers are needed, please share your result and join us in our [Gitter](https://gitter.im/Validity90/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link).

## Status

|   Device  | Status |
|-----------|--------|
| 138a:0090 | Scan, Image output, Internal DB check works in prototype. There is also a match-on-host [libfprint driver](https://github.com/3v1n0/libfprint) by @3v1n0 based on prototype source(138a:0090 only). You can use it untill proper unified driver is available. |
| 138a:0097 | Scan, Internal DB check works in prototype |
| 138a:0094 | Doesn't work yet, but looks promising. I will try to work on it after 90/97 |
| 06cb:0081 | So far looks identical to 138a:0094 |
| 06cb:009a | Init works, leds work, scan doesn't work yet |
| 138a:0091 | Different protocol, out of scope for this project. Check out [Validity91](https://github.com/hmaarrfk/Validity91) which aims to reverse engineer it.|


| 		      Task       			| Specification/Analysis  | Prototype   | Driver 	    |
|---------------------------|-------------------------|-------------|-------------|
| Initialization  		      | Done 					          | Done	 	    | Not Started |
| Configuration/Reconfig    | In progress 	          | In progress | Not Started |
| Pre TLS key exchange 	    | In progress 				    | Done        | Not Started |
| TLS 			                | Done 						        | Done  	    | Not Started |
| 90: Ops: scan, LED, etc| In progress  			      | Scan, LED works  | Not Started |
| 97: Ops: enroll, check, reset, LED, etc| In progress  			      | Check works  | Not Started |
| Image format  		        | In progress  			      | Done        | Not Started |


original code here
<br>
https://github.com/nmikhailov/Validity90
<br>
https://balintbanyasz.wordpress.com/2015/03/27/get-validity-vfs-495-fingerprint-reader-working-in-ubuntu-14-04/
<br>
https://forums.linuxmint.com/viewtopic.php?t=302947
<br>
