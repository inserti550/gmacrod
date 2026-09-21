#include "ipc.hpp"

void ipc_thread() {
    umask(0077);
    // ~/.config/gmacrod/gmacrod.pipe maybe  
    std::filesystem::path pipe = config / "gmacrod.pipe";
    mkfifo(pipe.c_str(), 0600);
    while (running) {
        int fd = open(pipe.c_str(), O_RDONLY);
        if (fd < 0) continue;

        char buf[64] = {};
        int n = read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n <= 0) continue;

        std::string cmd(buf, n);
        if (!cmd.empty() && cmd.back() == '\n') cmd.pop_back();

        if (cmd == "reload") {
            scan_profiles();
            lcd_mark_dirty();
        }
        else if (cmd == "reload_config") {
            load_config(config_name);
            lcd_mark_dirty();
        }
        else if (cmd == "resave_config") {
            save_config(config_name);
            lcd_mark_dirty();
        }
        else if (cmd.rfind("load:", 0) == 0) {
            std::string name = cmd.substr(5);
            save_config(config_name);
            load_config(name);
            {
                std::lock_guard<std::mutex> lk(gui_mtx);
                config_name = name;
            }
            scan_profiles();
            lcd_mark_dirty();
        }
    }
    unlink((const char*)"/tmp/gmacrod.pipe");
}