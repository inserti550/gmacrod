#pragma once

void button_thread();
int  map_gkey(unsigned long keystate);
void save_recorded_macro(int gkey, int mkey, const std::vector<action>& macro);

void on_gkey       (int gkey, int mkey_state);
//void on_mkey_change(int mkey_state);