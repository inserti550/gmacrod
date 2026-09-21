#include "../globals.hpp"

std::vector<std::string> profile_list;
int                      gui_select_idx = 0;

void to_json(nlohmann::json& j, const action& a) {
    j = nlohmann::json::object();
    if (a.type == action_type::shell) {
        j["cmd"] = a.cmd;
    } else if (a.type == action_type::wait_release) {
        j["type"] = static_cast<int>(a.type);
    } else {
        j["key"] = a.key;
        if (a.release)
            j["release"] = a.release;
    }
    if (a.delay != 0)
        j["delay"] = a.delay;
}

void from_json(const nlohmann::json& j, action& a) {
    a = action{};

    if (j.contains("type")) {
        a.type = static_cast<action_type>(j.at("type").get<int>());
        if (j.contains("key"))     a.key     = j.at("key").get<uint16_t>();
        if (j.contains("release")) a.release = j.at("release").get<bool>();
        if (j.contains("cmd"))     a.cmd     = j.at("cmd").get<std::string>();
    } else if (j.contains("cmd") && !j.at("cmd").get<std::string>().empty()) {
        a.type = action_type::shell;
        a.cmd  = j.at("cmd").get<std::string>();
    } else if (j.contains("key")) {
        a.type = action_type::key;
        a.key  = j.at("key").get<uint16_t>();
        if (j.contains("release")) a.release = j.at("release").get<bool>();
    }

    if (j.contains("delay"))
        a.delay = j.at("delay").get<uint64_t>();
}

void to_json(nlohmann::json& j, const macro& m) {
    j = nlohmann::json::object();
    j["actions"] = m.actions;
    if (m.type != macro_type::once)
        j["type"] = static_cast<int>(m.type);
    if (m.delay != 0)
        j["delay"] = m.delay;
}

void from_json(const nlohmann::json& j, macro& m) {
    m = macro{};
    if (j.contains("actions")) m.actions = j.at("actions").get<std::vector<action>>();
    if (j.contains("type"))    m.type    = static_cast<macro_type>(j.at("type").get<int>());
    if (j.contains("delay"))   m.delay   = j.at("delay").get<uint64_t>();
}

void scan_profiles() {
    namespace fs = std::filesystem;
    fs::path profiles_dir = config / "profiles";

    std::vector<std::string> list;

    if (fs::exists(profiles_dir / "default.json"))
        list.push_back("default.json");

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
        list.push_back(n);

    std::lock_guard<std::mutex> lk(gui_mtx);
    profile_list = std::move(list);

    gui_select_idx = 0;
    for (int i = 0; i < (int)profile_list.size(); ++i) {
        if (profile_list[i] == config_name) {
            gui_select_idx = i;
            break;
        }
    }
}

static void backup_profile(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return;
    auto backup = path;
    backup.replace_extension(".json.bak");
    std::filesystem::copy_file(path, backup, std::filesystem::copy_options::overwrite_existing);
}

void generate_config() {
    std::vector<uint16_t> keys = {
        KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
        12, 13, 26, 27, 40, 43, 110, 111
    };

    profile_data_t p{};
    for (size_t i = 0; i < keys.size() && i < 18; i++) {
        p[0][i] = macro{ macro_type::once, {
                {action_type::key, KEY_LEFTCTRL, false, "", 0},
                {action_type::key, keys[i],      false, "", 10},
                {action_type::key, keys[i],      true,  "", 10},
                {action_type::key, KEY_LEFTCTRL, true,  "", 10}
        }};
        p[1][i] = macro{ macro_type::once, {
                {action_type::key, KEY_LEFTSHIFT, false, "", 0},
                {action_type::key, keys[i],       false, "", 10},
                {action_type::key, keys[i],       true,  "", 10},
                {action_type::key, KEY_LEFTSHIFT, true,  "", 10}
        }};
        p[2][i] = macro{ macro_type::once, {
                {action_type::key, KEY_LEFTALT, false, "", 0},
                {action_type::key, keys[i],     false, "", 10},
                {action_type::key, keys[i],     true,  "", 10},
                {action_type::key, KEY_LEFTALT, true,  "", 10}
        }};
    }

    nlohmann::json j = p;

    std::ofstream file(config / "profiles" / "default.json");
    if (file.is_open())
        file << j.dump(4);
}

void load_config(std::string name) {
    //https://cppreference.com/cpp/filesystem/canonical
    std::filesystem::path fname = std::filesystem::path(name).filename();
    if (fname.empty() || fname == "." || fname == "..") {
        std::cerr << "invalid profile name " << name << "\n";
        return;
    }
    std::filesystem::path path = config / "profiles" / fname;
    std::ifstream file(path);
    if (!file.is_open()) return;

    nlohmann::json j;
    file >> j;
    try {
        if (j.is_array()) {
            size_t modes = std::min(j.size(), size_t(3));
            for (size_t m = 0; m < modes; ++m) {
                size_t available_keys = std::min(j[m].size(), size_t(18));
                for (size_t k = 0; k < available_keys; ++k)
                    current_profile[m][k] = j[m][k].get<macro>();
            }
        } else if (j.is_object()) {
            static constexpr const char* ltabs[3] = { "M1", "M2", "M3" };
            for (size_t m = 0; m < 3; ++m) {
                if (j.contains(ltabs[m]) && j[ltabs[m]].is_array()) {
                    size_t available_keys = std::min(j[ltabs[m]].size(), size_t(18));
                    for (size_t k = 0; k < available_keys; ++k)
                        current_profile[m][k] = j[ltabs[m]][k].get<macro>();
                }
            }
        }
    } catch (...) {
        file.close();
        if (name == "default.json") {
            backup_profile(path);
            generate_config();
            std::cerr << name << " config regenerate\nbackup was creater in " << path << "\n";
            load_config(name);
        } else {
            std::cerr << "Json fields error\ntry delete config " << name << " in " << config << "\n";
        }
    }
}

void save_config(std::string name) {
    std::filesystem::path fname = std::filesystem::path(name).filename();
    if (fname.empty() || fname == "." || fname == "..") {
        std::cerr << "invalid profile name " << name << "\n";
        return;
    }
    std::filesystem::path path = config / "profiles" / fname;
    nlohmann::json j = current_profile;
    std::ofstream file(path);
    if (file.is_open())
        file << j.dump(4);
}