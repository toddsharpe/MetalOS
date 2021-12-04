Trying to understand how windowing systems work. Mainly "how does the right window get the keypress"

## References:
1. https://unix.stackexchange.com/questions/25601/how-do-mouse-events-work-in-linux/25607#25607
2. https://unix.stackexchange.com/questions/116629/how-do-keyboard-input-and-text-output-work
3. https://jichu4n.com/posts/how-x-window-managers-work-and-how-to-write-one-part-i/


## Overview
![](https://jichu4n.com/content/images/2018/10/so1jXbe2d2Vvx917pbA5Cjw.png)

Taken from reference 3

Once xserver gets events, it can determine what window is in focused (as it owns the UI), what control, etc. How does xserver get mouse and keyboard?

Listing input devices
```
tsharpe@tsharpe-ubuntu:/dev/input$ cat /proc/bus/input/devices
I: Bus=0011 Vendor=0001 Product=0001 Version=ab00
N: Name="AT Translated Set 2 keyboard"
P: Phys=d34b2567-b9b6-42b9-8778-0a4ec0b
S: Sysfs=/devices/LNXSYSTM:00/LNXSYBUS:00/ACPI0004:00/VMBUS:00/d34b2567-b9b6-42b9-8778-0a4ec0b955bf/serio0/input/input0
U: Uniq=
H: Handlers=sysrq kbd event0
B: PROP=0
B: EV=100013
B: KEY=402000000 3803078f800d001 feffffdfffefffff fffffffffffffffe
B: MSC=10

I: Bus=0006 Vendor=045e Product=0621 Version=0001
N: Name="Microsoft Vmbus HID-compliant Mouse"
P: Phys=
S: Sysfs=/devices/0006:045E:0621.0001/input/input1
U: Uniq=
H: Handlers=mouse0 event1 js0
B: PROP=0
B: EV=1f
B: KEY=1f0000 0 0 0 0
B: REL=900
B: ABS=3
B: MSC=10
```

Each device has a list of handlers that you read from to get their events:
```
H: Handlers=sysrq kbd event0
H: Handlers=mouse0 event1 js0
```

X Server reads inputs from /dev/input (https://www.kernel.org/doc/Documentation/input/input.txt)
```
tsharpe@tsharpe-ubuntu:/dev/input$ ls -la
total 0
drwxr-xr-x  3 root root     160 Nov 17 18:52 .
drwxr-xr-x 17 root root    3900 Nov 17 18:52 ..
drwxr-xr-x  2 root root      60 Nov 17 18:52 by-path
crw-rw----  1 root input 13, 64 Nov 17 18:52 event0
crw-rw----  1 root input 13, 65 Nov 17 18:52 event1
crw-rw-r--  1 root input 13,  0 Nov 17 18:52 js0
crw-rw----  1 root input 13, 63 Nov 17 18:52 mice
crw-rw----  1 root input 13, 32 Nov 17 18:52 mouse0
```

Open(), then poll/select to be notified file descriptor has data, then read.

How does framebuffer work?
/dev/fb0?