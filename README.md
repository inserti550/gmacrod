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
- shell command execution
- Cycles! Repetitions and delays

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
You can use IPC to controll **gmacrod**
```
# if u need reload profile list
echo "reload" | sudo tee /tmp/gmacrod.pipe > /dev/null

# if u need reload config
echo "reload_config" | sudo tee /tmp/gmacrod.pipe > /dev/null

# if u need resave config
echo "resave_config" | sudo tee /tmp/gmacrod.pipe > /dev/null

# if u need load config
echo "load:profilename.json" | sudo tee /tmp/gmacrod.pipe > /dev/null
# While load doesn't check for the presence of a config yet, you can create configs using it
```
You'll likely need to change the gmacrod configuration so that the G keys perform the actions you want
To do this, go to the configuration section you specified manually or that was selected automatically (~/.config/gmacrod)
You don't have to use all the parameters at the same time, use default.json as a guide.

```json
[
  // The initial form is set to [ ], there must be 3 [ ] inside
  [
    // Inside this block are all the macro blocks, the topmost macro block is G1, the bottom one is G18
    // G1
    {
      // actions - list of actions (omg) that will be performed from top to bottom in sequence
      "actions": 
      [
        {
          "type": 0-2,
          // 0: Key - presses the button from key
          // 1: Shell - execute shell from cmd
          // 2: wait release - pauses execution when reached and continues execution when released
          "key": 0-idk, // key for type 0
          "release": true/false, // release state for type 0, default false
          "cmd": "string", // example "xdg-open *link to rickroll*"
          "delay": 0-inf // delay before executing this particular block of actions in ms; if the first block is delay 0 and the second is delay 500, then the first block will be executed immediately and the second after 500 ms
        },
        {...},
        //repeat as many times as necessary
        {}
      ],
      "type": 0-2 // once - one press one start, repeate - repeate for press, toggle — switch macro state on press
      "delay": 0-inf // delay between macro repetitions in ms
    }
    {// G2}
  ],
  [
    ...
  ],
  [
    // repeat as many times as necessary
  ]
```

example:
```json
[
    [   // G1 - press ctrl+1
        {
            "actions": [
                {
                    "key": 29
                },
                {
                    "delay": 10,
                    "key": 2
                },
                {
                    "delay": 10,
                    "key": 2,
                    "release": true
                },
                {
                    "delay": 10,
                    "key": 29,
                    "release": true
                }
            ]
        },
        //G2 - press F13, use wait release
        {
          "actions": [
              {
                "key": 183
              },
              {
                "type": 2
              },
              {
                "delay": 10,
                "key": 183,
                "release": true
              }
          ]
        },
        {
          "actions": [
            {
              "cmd": "xdg-open ."
            },
            {
              "type": 2
            },
            {
              "cmd": "wall Hello world!"
            }
          ]
        }
    ]
]
```