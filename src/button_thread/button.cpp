#include "../globals.hpp"

static int macro_state = 0;
static int target_gkey = -1;
static int target_mkey = 0;

static std::unique_ptr<macro_recorder> recorder_ptr = nullptr;

void button_thread() {
    struct pollfd fds;
    fds.fd     = g15screen_fd;
    fds.events = POLLIN;

    while (running)
    {
        fds.revents = 0;
        unsigned long keystate = 0;

        if (poll(&fds, 1, 1000) > 0) {
            read(g15screen_fd, &keystate, sizeof(keystate));
        }

        if (keystate == 0)
            continue;

        if (keystate == G15_KEY_L2) {
            gui_select_default();
            continue;
        }
        if (keystate == G15_KEY_L3) {
            gui_select_up();
            continue;
        }
        if (keystate == G15_KEY_L4) {
            gui_select_down();
            continue;
        }
        if (keystate == G15_KEY_L5) {
            gui_apply_selection();
            scan_profiles();
            continue;
        }

        // m
        if (keystate == G15_KEY_M1 || keystate == G15_KEY_M2 || keystate == G15_KEY_M3) {
            if (keystate == G15_KEY_M1) { mkey_state = 0; mled_state = G15_LED_M1; }
            if (keystate == G15_KEY_M2) { mkey_state = 1; mled_state = G15_LED_M2; }
            if (keystate == G15_KEY_M3) { mkey_state = 2; mled_state = G15_LED_M3; }

            g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);

            if (macro_state != 0) {
                macro_state = 0;
                recording   = false;
                recorder_ptr.reset();
            }
            lcd_mark_dirty();
            continue;
        }

        // mr
        if (keystate == G15_KEY_MR) {
            if (macro_state == 0) {
                macro_state = 1;
                target_gkey = -1;
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, G15_LED_MR | mled_state);
            }
            else if (macro_state == 1) {
                macro_state = 0;
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);
            }
            else if (macro_state == 2) {
                macro_state = 0;
                recording   = false;
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);
                recorder_ptr.reset();
            }
            lcd_mark_dirty();
            continue;
        }

        // g
        if (keystate >= G15_KEY_G1 && keystate <= G15_KEY_G18) {
            int gkey = map_gkey(keystate);
            if (gkey < 0)
                continue;

            if (macro_state == 1) {
                target_gkey = gkey;
                target_mkey = mkey_state;
                macro_state = 2;
                recording   = true;

                recorder_ptr = std::make_unique<macro_recorder>(
                    [g = target_gkey, m = target_mkey](const std::vector<action>& macro) {
                        save_recorded_macro(g, m, macro);
                    }
                );
            }
            else if (macro_state == 2) {

            }
            else {
                on_gkey(gkey, mkey_state);
            }
            continue;
        }
    }

    close(g15screen_fd);
}

void save_recorded_macro(int gkey, int mkey, const std::vector<action>& macro) {
    if (gkey < 0 || gkey >= 18 || mkey < 0 || mkey >= 3)
        return;

    current_profile[mkey][gkey] = macro;
    save_config(config_name);
}

int map_gkey(unsigned long keystate) {
    switch (keystate) {
        case G15_KEY_G1:  return 0;
        case G15_KEY_G2:  return 1;
        case G15_KEY_G3:  return 2;
        case G15_KEY_G4:  return 3;
        case G15_KEY_G5:  return 4;
        case G15_KEY_G6:  return 5;
        case G15_KEY_G7:  return 6;
        case G15_KEY_G8:  return 7;
        case G15_KEY_G9:  return 8;
        case G15_KEY_G10: return 9;
        case G15_KEY_G11: return 10;
        case G15_KEY_G12: return 11;
        case G15_KEY_G13: return 12;
        case G15_KEY_G14: return 13;
        case G15_KEY_G15: return 14;
        case G15_KEY_G16: return 15;
        case G15_KEY_G17: return 16;
        case G15_KEY_G18: return 17;
        default:          return -1;
    }
}

void on_gkey(int gkey, int mkey) {
    const std::vector<action>& actfrun = current_profile[mkey][gkey];

    for (size_t i = 0; i < actfrun.size(); ++i) {
        const auto& act = actfrun[i];
        if (act.release)
            vk_bd.release_key(act.key);
        else
            vk_bd.press_key(act.key);

        if (i < actfrun.size() - 1)
            usleep(act.delay);
    }
}

void on_mkey_change(int mkey) {
    // TODO
}