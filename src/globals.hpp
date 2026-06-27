#pragma once
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <poll.h>
#include <libg15.h>
#include <libg15render.h>
#include <g15daemon_client.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <linux/uinput.h>
#include <linux/input.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <fstream>

#include "json.hpp"
#include "profile/profile.hpp"
#include "button_thread/recorder.hpp"
#include "button_thread/button.hpp"
#include "button_thread/virtual_keyboard.hpp"
#include "lcd_thread/lcd.hpp"

extern std::atomic<bool> running;
extern std::atomic<int>  m_state;
extern std::atomic<bool> recording_state;

extern int g15screen_fd;
extern g15canvas* canvas;

extern int  mkey_state;
extern int  mled_state;
extern bool recording;

extern virtual_keyboard vk_bd;

extern std::filesystem::path config;
extern std::string config_name;

extern std::vector<std::string> profile_list;
extern int gui_select_idx;

void scan_profiles();