#include "globals.hpp"

std::atomic<bool> running         = true;
std::atomic<int>  m_state         = 0;
std::atomic<bool> recording_state = false;

int g15screen_fd = -1;
g15canvas* canvas = nullptr;

int  mkey_state = 0;
int  mled_state = G15_LED_M1;
bool recording  = false;

virtual_keyboard vk_bd;

std::filesystem::path config;
std::string config_name;

int main(int argc, char* argv[]) {

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config = argv[i + 1];
                ++i;
            } else {
                std::cerr << "Error: after " << arg << " need to specify a path!\n";
                return 1;
            }
        }
        if (arg == "-h" || arg == "--help") {
            std::cout << "GMacroD\n";
            std::cout << "-h --help  - shows this message\n";
            std::cout << "-c --config - path to configuration and profiles directory\n";
            return 0;
        }
    }

    if (config.empty()) {
        const char* home = std::getenv("HOME");
        if (home && *home) {
            config = std::filesystem::path(home) / ".config" / "gmacrod";
            std::filesystem::create_directories(config);
        } else {
            std::cerr << "Error: Unable to determine home directory, use -c / --config\n";
            return 1;
        }
    }

    std::filesystem::create_directories(config / "profiles");

    if (!std::filesystem::exists(config / "profiles" / "default.json"))
        generate_config();

    load_config("default.json");
    config_name = "default.json";

    scan_profiles();

    if (!vk_bd.init_device()) {
        std::cerr << "Error: init virtual keyboard failed.\n";
        return 1;
    }

    canvas = (g15canvas*)malloc(sizeof(g15canvas));
    if (!canvas) {
        std::cerr << "Unable to initialise libg15render canvas\n";
        return 1;
    }
    g15r_initCanvas(canvas);

    do {
        g15screen_fd = new_g15_screen(G15_G15RBUF);
        if (g15screen_fd < 0) {
            std::cout << "Waiting for g15daemon...\n";
            sleep(1);
        }
    } while (g15screen_fd < 0);
    std::cout << "[G15] Initialized!\n";

    int dummy = 0;
    g15_send_cmd(g15screen_fd, G15DAEMON_KEY_HANDLER, dummy);
    g15_send_cmd(g15screen_fd, G15DAEMON_MKEYLEDS,    mled_state);

    std::thread buttont(button_thread);
    std::thread lcdt(lcd_thread);

    buttont.join();
    lcdt.join();

    free(canvas);
    return 0;
}