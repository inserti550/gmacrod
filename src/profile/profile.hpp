#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <array>

struct action {
    uint16_t key;
    bool release;
    uint64_t delay;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(action, key, release, delay)

using profile_data_t = std::array<std::array<std::vector<action>, 18>, 3>;
inline profile_data_t current_profile;

void generate_config();
void load_config(std::string name);
void save_config(std::string name);

extern std::vector<std::string> profile_list;
extern int                      gui_select_idx;
void scan_profiles();