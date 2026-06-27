#pragma once

class virtual_keyboard {
private:
    int uinput_fd = -1;

    void send_event(uint16_t type, uint16_t code, int32_t value) {
        struct input_event ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.type = type;
        ev.code = code;
        ev.value = value;
        write(uinput_fd, &ev, sizeof(ev));
    }

public:
    virtual_keyboard() = default;
    
    ~virtual_keyboard() {
        close_device();
    }

    bool init_device() {
        uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        if (uinput_fd < 0) {
            std::perror("Failed to open /dev/uinput (run as root or give your user the input group)");
            return false;
        }

        ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);

        for (int key = 1; key <= KEY_MIN_INTERESTING; ++key) {
            ioctl(uinput_fd, UI_SET_KEYBIT, key);
        }
        for (int key = KEY_F13; key <= KEY_F24; ++key) {
            ioctl(uinput_fd, UI_SET_KEYBIT, key);
        }

        struct uinput_setup usetup;
        std::memset(&usetup, 0, sizeof(usetup));
        usetup.id.bustype = BUS_USB;
        usetup.id.vendor  = 0x2944;
        usetup.id.product = 0x1944;
        std::strcpy(usetup.name, "G15 macro keyboard");

        ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
        ioctl(uinput_fd, UI_DEV_CREATE);

        return true;
    }

    void press_key(uint16_t key_code) {
        if (uinput_fd < 0) return;
        send_event(EV_KEY, key_code, 1);
        send_event(EV_SYN, SYN_REPORT, 0);
    }

    void release_key(uint16_t key_code) {
        if (uinput_fd < 0) return;
        send_event(EV_KEY, key_code, 0);
        send_event(EV_SYN, SYN_REPORT, 0);
    }

    void click_key(uint16_t key_code, int delay_ms = 50) {
        press_key(key_code);
        usleep(delay_ms * 1000);
        release_key(key_code);
    }

    void close_device() {
        if (uinput_fd >= 0) {
            ioctl(uinput_fd, UI_DEV_DESTROY);
            close(uinput_fd);
            uinput_fd = -1;
        }
    }
};