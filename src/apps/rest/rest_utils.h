/*
 * rest_utils.h
 *
 * Author: kafe
 */

#include "yuri/core/thread/InputThread.h"
#include "yuri/core/thread/IOThreadGenerator.h"
#include "json.hpp"

#include <vector>

namespace yuri {
namespace web {

bool is_last_key(const std::vector<core::InputDeviceConfig>& cfgs, const std::string& key, const std::vector<std::string>& used);
std::vector<std::string> unused_keys(const core::InputDeviceConfig& cfg, const std::vector<std::string>& used);

void get_cfgs(nlohmann::json& json, const std::vector<core::InputDeviceConfig>& cfgs, std::vector<std::string> order, std::vector<std::string> used);
void get_classes(nlohmann::json& json);
void get_formats(nlohmann::json& json);
void get_converters(nlohmann::json& json);
void get_pipes(nlohmann::json& json);
void get_specifiers(nlohmann::json& json);


}
}