#include "../globals.hpp"

static int target_mkey = 0;

static std::unique_ptr<macro_recorder> recorder_ptr = nullptr;

static constexpr unsigned long G_KEY_BITS[18] = {
    G15_KEY_G1,  G15_KEY_G2,  G15_KEY_G3,  G15_KEY_G4,
    G15_KEY_G5,  G15_KEY_G6,  G15_KEY_G7,  G15_KEY_G8,
    G15_KEY_G9,  G15_KEY_G10, G15_KEY_G11, G15_KEY_G12,
    G15_KEY_G13, G15_KEY_G14, G15_KEY_G15, G15_KEY_G16,
    G15_KEY_G17, G15_KEY_G18
};

static std::array<bool, 18> g_prev_gkeys{};
std::atomic<unsigned long> keystate{0};

void button_thread() {
    static bool l2release = false, l3release = false, l4release = false, l5release = false;

    struct pollfd fds;
    fds.fd     = g15screen_fd;
    fds.events = POLLIN;

    while (running)
    {
        if (poll(&fds, 1, 50) > 0) {
            read(g15screen_fd, &keystate, sizeof(keystate));
        }

        std::array<bool, 18> cur_gkeys = map_gkeys(keystate);

        if (macro_state == 1) {
            for (int gk = 0; gk < 18; ++gk) {
                if (cur_gkeys[gk] && !g_prev_gkeys[gk]) {
                    macro_state = 0;
                    auto recorded = recorder_ptr->take_macro();
                    recorder_ptr.reset();
                    save_recorded_macro(gk, target_mkey, recorded);
                    g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);
                    lcd_mark_dirty();
                    break;
                }
            }
        } else {
            for (int gk = 0; gk < 18; ++gk) {
                if (cur_gkeys[gk] && !g_prev_gkeys[gk]) {
                    on_gkey(gk, mkey_state);
                } else if (!cur_gkeys[gk] && g_prev_gkeys[gk]) {
                    off_gkey(gk, mkey_state);
                }
            }
        }
        g_prev_gkeys = cur_gkeys;

        if ((keystate & G15_KEY_L2) && !l2release) {
            l2release = true;
            gui_select_default();
            continue;
        } else if (!(keystate & G15_KEY_L2) && l2release) {
            l2release = false;
        }

        if ((keystate & G15_KEY_L3) && !l3release) {
            l3release = true;
            gui_select_up();
            continue;
        } else if (!(keystate & G15_KEY_L3) && l3release) {
            l3release = false;
        }

        if ((keystate & G15_KEY_L4) && !l4release) {
            l4release = true;
            gui_select_down();
            continue;
        } else if (!(keystate & G15_KEY_L4) && l4release) {
            l4release = false;
        }

        if ((keystate & G15_KEY_L5) && !l5release) {
            l5release = true;
            gui_apply_selection();
            scan_profiles();
            continue;
        } else if (!(keystate & G15_KEY_L5) && l5release) {
            l5release = false;
        }

        // m
        if (keystate & G15_KEY_M1 || keystate & G15_KEY_M2 || keystate & G15_KEY_M3) {
            if (keystate & G15_KEY_M1) { mkey_state = 0; mled_state = G15_LED_M1; }
            if (keystate & G15_KEY_M2) { mkey_state = 1; mled_state = G15_LED_M2; }
            if (keystate & G15_KEY_M3) { mkey_state = 2; mled_state = G15_LED_M3; }
            if (macro_state == 0)
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);
            lcd_mark_dirty();
            continue;
        }

        // mr
        if (keystate & G15_KEY_MR) {
            if (macro_state == 0) {
                target_mkey = mkey_state;
                macro_state = 1;
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, G15_LED_MR);
                recorder_ptr = std::make_unique<macro_recorder>();
                //record start
            }
            else if (macro_state == 1) {
                macro_state = 0;
                recorder_ptr.reset();
                g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS, mled_state);
                //cancel
            }
            lcd_mark_dirty();
            continue;
        }
    }

    close(g15screen_fd);
}

void save_recorded_macro(int gkey, int mkey, const std::vector<action>& macro) {
    if (gkey < 0 || gkey >= 18 || mkey < 0 || mkey >= 3)
        return;

    current_profile[mkey][gkey].actions = macro;
    save_config(config_name);
}

std::array<bool, 18> map_gkeys(unsigned long keystate) {
    std::array<bool, 18> result{};
    for (int i = 0; i < 18; ++i)
        result[i] = (keystate & G_KEY_BITS[i]) != 0;
    return result;
}

int map_gkey(unsigned long keystate) {
    for (int i = 0; i < 18; ++i)
        if (keystate & G_KEY_BITS[i]) return i;
    return -1;
}

unsigned long gkey_bit(int gkey) {
    if (gkey < 0 || gkey >= 18) return 0;
    return G_KEY_BITS[gkey];
}

void on_gkey(int gkey, int mkey) {
    start_macro(gkey, mkey);
}

void off_gkey(int gkey, int mkey) {
    stop_macro(gkey, mkey);
}