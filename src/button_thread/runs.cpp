//
// Created by nazar on 01.07.2026.
//

#include "runs.h"

std::array<std::array<std::atomic<bool>, 18>, 3> macro_running;
std::array<std::array<std::thread, 18>, 3>       macro_threads;
std::array<std::array<std::atomic<bool>, 18>, 3> macro_toggle_on;

static void exec_actions(const std::vector<action>& actfrun) {
    for (size_t i = 0; i < actfrun.size(); ++i) {
        const auto& act = actfrun[i];
        switch (act.type) {
            case action_type::key:
                act.release ? vk_bd.release_key(act.key) : vk_bd.press_key(act.key);
                break;
            case action_type::shell:
                if (!act.cmd.empty()) {
                    if (fork() == 0) {
                        execl("/bin/sh", "sh", "-c", act.cmd.c_str(), nullptr);
                        _exit(0);
                    }
                }
                break;
        }
        if (i < actfrun.size() - 1)
            usleep(act.delay);
    }
}

static void interruptible_wait(uint64_t total_useconds, const std::atomic<bool>& flag) {
    constexpr uint64_t step = 5000; // 5 ms
    while (total_useconds > 0 && flag) {
        uint64_t chunk = std::min<uint64_t>(total_useconds, step);
        usleep(chunk);
        total_useconds -= chunk;
    }
}

void run_once(int gkey, int mkey) {
    exec_actions(current_profile[mkey][gkey].actions);
}

void run_repeate(int gkey, int mkey) {
    const macro m = current_profile[mkey][gkey];
    while (macro_running[mkey][gkey]) {
        exec_actions(m.actions);
        if (!macro_running[mkey][gkey]) break;
        if (m.delay > 0)
            interruptible_wait(m.delay, macro_running[mkey][gkey]);
    }
}

void start_macro(int gkey, int mkey) {
    if (gkey < 0 || gkey >= 18 || mkey < 0 || mkey >= 3) return;

    switch (current_profile[mkey][gkey].type) {
        case macro_type::once: {
            std::thread t(run_once, gkey, mkey);
            t.detach();
            break;
        }
        case macro_type::repeate: {
            if (macro_running[mkey][gkey]) break;
            if (macro_threads[mkey][gkey].joinable())
                macro_threads[mkey][gkey].join();
            macro_running[mkey][gkey] = true;
            macro_threads[mkey][gkey] = std::thread(run_repeate, gkey, mkey);
            break;
        }
        case macro_type::toggle: {
            bool now_on = !macro_toggle_on[mkey][gkey];
            macro_toggle_on[mkey][gkey] = now_on;
            if (now_on) {
                if (macro_threads[mkey][gkey].joinable())
                    macro_threads[mkey][gkey].join();
                macro_running[mkey][gkey] = true;
                macro_threads[mkey][gkey] = std::thread(run_repeate, gkey, mkey);
            } else {
                macro_running[mkey][gkey] = false;
            }
            break;
        }
    }
}

void stop_macro(int gkey, int mkey) {
    if (gkey < 0 || gkey >= 18 || mkey < 0 || mkey >= 3) return;
    if (current_profile[mkey][gkey].type == macro_type::repeate)
        macro_running[mkey][gkey] = false;
}