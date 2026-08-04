#pragma once

constexpr unsigned long ALL_G_KEYS =
    G15_KEY_G1  | G15_KEY_G2  | G15_KEY_G3  | G15_KEY_G4  |
    G15_KEY_G5  | G15_KEY_G6  | G15_KEY_G7  | G15_KEY_G8  |
    G15_KEY_G9  | G15_KEY_G10 | G15_KEY_G11 | G15_KEY_G12 |
    G15_KEY_G13 | G15_KEY_G14 | G15_KEY_G15 | G15_KEY_G16 |
    G15_KEY_G17 | G15_KEY_G18;

extern std::atomic<unsigned long> keystate;

void button_thread();

int  map_gkey(unsigned long keystate);
std::array<bool, 18> map_gkeys(unsigned long keystate);
unsigned long gkey_bit(int gkey);

void save_recorded_macro(int gkey, int mkey, const std::vector<action>& macro);

void on_gkey (int gkey, int mkey_state);
void off_gkey(int gkey, int mkey_state);