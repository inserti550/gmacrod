#pragma once
#include "../globals.hpp"

extern std::array<std::array<std::atomic<bool>, 18>, 3> macro_running;
extern std::array<std::array<std::thread, 18>, 3>       macro_threads;
extern std::array<std::array<std::atomic<bool>, 18>, 3> macro_toggle_on;

void run_repeate(int gkey, int mkey);
void run_once(int gkey, int mkey);

void start_macro(int gkey, int mkey);
void stop_macro(int gkey, int mkey);