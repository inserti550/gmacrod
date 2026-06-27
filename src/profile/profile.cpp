#include "../globals.hpp"

std::vector<std::string> profile_list;
int                      gui_select_idx = 0;

inline constexpr const char* JSON_MODES[3] = { "M1", "M2", "M3" };

void scan_profiles() {
    profile_list.clear();

    namespace fs = std::filesystem;
    fs::path profiles_dir = config / "profiles";

    if (fs::exists(profiles_dir / "default.json"))
        profile_list.push_back("default.json");

    std::vector<std::string> others;
    for (const auto& entry : fs::directory_iterator(profiles_dir)) {
        if (entry.path().extension() == ".json") {
            std::string name = entry.path().filename().string();
            if (name != "default.json")
                others.push_back(name);
        }
    }
    std::sort(others.begin(), others.end());
    for (auto& n : others)
        profile_list.push_back(n);

    gui_select_idx = 0;
    for (int i = 0; i < (int)profile_list.size(); ++i) {
        if (profile_list[i] == config_name) {
            gui_select_idx = i;
            break;
        }
    }
}

void generate_config() {
    nlohmann::json j;
    std::vector<uint16_t> keys = {
        KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
        12, 13, 26, 27, 40, 43, 110, 111
    };

    for (size_t i = 0; i < keys.size(); i++) {
        j["M1"][i] = std::vector<action>{
            {KEY_LEFTCTRL,  false, 0},
            {keys[i],       false, 10},
            {keys[i],       true,  10},
            {KEY_LEFTCTRL,  true,  10}
        };
        j["M2"][i] = std::vector<action>{
            {KEY_LEFTSHIFT, false, 0},
            {keys[i],       false, 10},
            {keys[i],       true,  10},
            {KEY_LEFTSHIFT, true,  10}
        };
        j["M3"][i] = std::vector<action>{
            {KEY_LEFTALT,   false, 0},
            {keys[i],       false, 10},
            {keys[i],       true,  10},
            {KEY_LEFTALT,   true,  10}
        };
    }

    std::ofstream file(config / "profiles" / "default.json");
    if (file.is_open())
        file << j.dump(4);
}

void load_config(std::string name) {
    std::filesystem::path path = config / "profiles" / name;
    if (!std::filesystem::exists(path))
        generate_config();

    std::ifstream file(path);
    if (!file.is_open()) return;

    nlohmann::json j;
    file >> j;

    for (size_t m = 0; m < 3; ++m) {
        if (j.contains(JSON_MODES[m]) && j[JSON_MODES[m]].is_array()) {
            size_t available_keys = std::min(j[JSON_MODES[m]].size(), size_t(18));
            for (size_t k = 0; k < available_keys; ++k)
                current_profile[m][k] = j[JSON_MODES[m]][k].get<std::vector<action>>();
        }
    }
}

void save_config(std::string name) {
    nlohmann::json j;
    for (size_t m = 0; m < 3; ++m)
        for (size_t k = 0; k < 18; ++k)
            j[JSON_MODES[m]][k] = current_profile[m][k];

    std::ofstream file(config / "profiles" / name);
    if (file.is_open())
        file << j.dump(4);
}