#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <array>

enum class action_type : uint8_t { key, shell };

struct action {
    action_type type    = action_type::key;
    uint16_t    key     = 0;
    bool        release = false;
    std::string cmd;
    uint64_t    delay   = 0;
};

enum class macro_type : uint8_t { once, repeate, toggle };

struct macro {
    macro_type type = macro_type::once;
    std::vector<action> actions;
    uint64_t delay = 0;
};

void to_json(nlohmann::json& j, const action& a);
void from_json(const nlohmann::json& j, action& a);

void to_json(nlohmann::json& j, const macro& m);
void from_json(const nlohmann::json& j, macro& m);

using profile_data_t = std::array<std::array<macro, 18>, 3>;
inline profile_data_t current_profile;

void generate_config();
void load_config(std::string name);
void save_config(std::string name);

extern std::vector<std::string> profile_list;
extern int                      gui_select_idx;
void scan_profiles();