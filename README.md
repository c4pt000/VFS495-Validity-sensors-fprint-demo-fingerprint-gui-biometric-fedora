**WIP

# Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora
revitalized fprint_demo-gui for fprintd-enroll gui (dinosaurs from the dead of fedora ~20)

![s1](https://raw.githubusercontent.com/c4pt000/Validity-sensors-fprint-demo-fingerprint-gui-biometric-fedora/master/finger-detect-image.png)

```
cp -rf vcsFPServiceDaemon /etc/rc.d/init.d/vcsFPServiceDaemon
systemctl enable vcsFPServiceDaemon
systemctl start vcsFPServiceDaemon
export LC_ALL=C >> /root/.bashrc
source /root/.bashrc
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
