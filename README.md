# gmacrod
 
![Platform](https://img.shields.io/badge/platform-Linux-blue?logo=linux)
![C++](https://img.shields.io/badge/C++-17-blue?logo=c%2B%2B)
![License](https://img.shields.io/badge/license-MIT-green)
 
This project aims to become a replacement for g15macro. g15macro is deprecated and does not support Wayland directly. Even with XWayland, it doesn't work correctly due to the use of X11-specific solutions like XTest. gmacrod works with both Wayland and X11.

## Features
- Works on **Wayland** and **X11**
- Records and plays back macros on G1–G18 keys
- Virtual keyboard via `/dev/uinput` low-level input emulation
- Profile support with JSON config
- LCD display support (G15 128×43)
- M-key switching

## Dependencies
 
- [g15daemon](https://github.com/mike-petersen/g15daemon)
- [libg15](https://github.com/netfab/libg15)
- [libg15render](https://github.com/vividnightmare/libg15render)

## Usage

You can run this program as root, but you can also run it as a user. **you need to give your user the input group** to gain access to **/dev/uinput**
```bash
# Run with default config (~/.config/gmacrod)
gmacrod
 
# Run with custom config directory
gmacrod -c /home/user/.config/gmacrod/
 
# Help
gmacrod --help

# to add user into input group
usermod -a -G input $USER
```