#pragma once

class macro_recorder {
private:
    std::vector<int> fds;
    std::thread worker_thread;
    std::atomic<bool> is_running{false};

    std::vector<action> recorded_macro;
    std::mutex macro_mtx;

    bool is_real_keyboard(int fd, const std::string& name) {
        if (name == "G15 macro keyboard") return false;

        unsigned long key_bitmask[257 / (sizeof(unsigned long) * 8) + 1];
        std::memset(key_bitmask, 0, sizeof(key_bitmask));

        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask) < 0) return false;

        bool has_q = (key_bitmask[KEY_Q / (sizeof(unsigned long) * 8)] & (1UL << (KEY_Q % (sizeof(unsigned long) * 8))));
        bool has_w = (key_bitmask[KEY_W / (sizeof(unsigned long) * 8)] & (1UL << (KEY_W % (sizeof(unsigned long) * 8))));
        return has_q && has_w;
    }

    void record_loop() {
        auto last_event_time = std::chrono::steady_clock::now();
        std::vector<struct pollfd> poll_fds(fds.size());

        for (size_t i = 0; i < fds.size(); ++i) {
            poll_fds[i].fd = fds[i];
            poll_fds[i].events = POLLIN;
        }

        struct input_event ev;

        while (is_running) {
            int ret = poll(poll_fds.data(), poll_fds.size(), 50);
            if (ret <= 0) continue;

            for (size_t i = 0; i < poll_fds.size(); ++i) {
                if (poll_fds[i].revents & POLLIN) {
                    while (read(poll_fds[i].fd, &ev, sizeof(ev)) > 0) {
                        if (ev.type == EV_KEY && (ev.value == 0 || ev.value == 1)) {
                            auto now = std::chrono::steady_clock::now();
                            uint64_t delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                now - last_event_time).count();
                            last_event_time = now;

                            action btn;
                            btn.key     = ev.code;
                            btn.release = (ev.value == 0);
                            btn.delay   = delta_ms;

                            std::lock_guard<std::mutex> lk(macro_mtx);
                            recorded_macro.push_back(btn);
                        }
                    }
                }
            }
        }
    }

public:
    macro_recorder() {
        namespace fs = std::filesystem;
        if (!fs::exists("/dev/input")) return;

        for (const auto& entry : fs::directory_iterator("/dev/input")) {
            std::string filename = entry.path().filename().string();
            if (filename.rfind("event", 0) == 0) {
                int fd = open(entry.path().c_str(), O_RDONLY | O_NONBLOCK);
                if (fd < 0) continue;

                char name[256] = "Unknown";
                if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
                    if (is_real_keyboard(fd, name)) {
                        fds.push_back(fd);
                    } else {
                        close(fd);
                    }
                } else {
                    close(fd);
                }
            }
        }

        if (fds.empty()) {
            std::cerr << "keyboard not found!\n";
            return;
        }

        is_running = true;
        worker_thread = std::thread(&macro_recorder::record_loop, this);
    }

    ~macro_recorder() {
        is_running = false;
        if (worker_thread.joinable())
            worker_thread.join();
        for (int fd : fds)
            if (fd >= 0) close(fd);
    }

    std::vector<action> take_macro() {
        is_running = false;
        if (worker_thread.joinable())
            worker_thread.join();
        std::lock_guard<std::mutex> lk(macro_mtx);
        return std::move(recorded_macro);
    }
};