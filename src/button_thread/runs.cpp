#include "runs.h"
#include <chrono>
#include <thread>

std::array<std::array<std::atomic<bool>, 18>, 3> macro_running;
std::array<std::array<std::thread, 18>, 3>       macro_threads;
std::array<std::array<std::atomic<bool>, 18>, 3> macro_toggle_on;

static void exec_actions(const std::vector<action>& actfrun, int gkey) {
    const unsigned long bit = gkey_bit(gkey);

    for (const auto& act : actfrun) {
        if (act.delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(act.delay));
        }

        switch (act.type) {
            case action_type::wait_release:
                while (running && (keystate & bit)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
                break;
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
    }
}

static void interruptible_wait(uint64_t total_ms, const std::atomic<bool>& flag) {
    constexpr auto step = std::chrono::milliseconds(5);
    auto remaining = std::chrono::milliseconds(total_ms);

    while (remaining > std::chrono::milliseconds(0) && flag) {
        auto chunk = std::min(remaining, step);
        std::this_thread::sleep_for(chunk);
        remaining -= chunk;
    }
}

void run_once(int gkey, int mkey) {
    exec_actions(current_profile[mkey][gkey].actions, gkey);
}

void run_repeate(int gkey, int mkey) {
    const macro m = current_profile[mkey][gkey];
    while (macro_running[mkey][gkey]) {
        exec_actions(m.actions, gkey);
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