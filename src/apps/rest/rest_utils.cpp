/*
 * rest_utils.cpp
 *
 * Author: kafe
 */

#include "rest_utils.h"

#include "yuri/event/BasicEvent.h"
#include "yuri/core/thread/IOThreadGenerator.h"
#include "yuri/core/thread/ConverterRegister.h"
#include "yuri/core/pipe/PipeGenerator.h"
#include "yuri/core/thread/InputRegister.h"

#include "yuri/event/BasicEventConversions.h"

#include "yuri/core/frame/raw_frame_params.h"
#include "yuri/core/frame/compressed_frame_params.h"
#include "yuri/core/frame/raw_audio_frame_params.h"

#include "yuri/core/utils/string_generator.h"

namespace yuri {
namespace web {

bool is_last_key(const std::vector<core::InputDeviceConfig>& cfgs, const std::string& key, const std::vector<std::string>& used) {
	for (const auto& c: cfgs) {
		for (const auto& p: c.params) {
			if (p.first != key && !contains(used, p.first)) {
				return false;
			}
		}
	}
	return true;
}

std::vector<std::string> unused_keys(const core::InputDeviceConfig& cfg, const std::vector<std::string>& used) {
	std::vector<std::string> keys;
	for (const auto& p: cfg.params) {
		if (!contains(used, p.first)) {
			keys.push_back(p.first);
		}
	}
	return keys;
}

std::string param_value(const yuri::event::pBasicEvent& event) {
    if (event->get_type() == event::event_type_t::boolean_event) {
        return yuri::event::lex_cast_value<bool>(event) ? "True" : "False";
    } else if (event->get_type() == event::event_type_t::vector_event) {
        const auto val = std::dynamic_pointer_cast<event::EventVector>(event);
        std::string value = "[";
        for (const auto &ev: *val) {
            if (value.size() > 1) {
                value = value + ", ";
            }
            value = value + param_value(ev);
        }
        value = value + "]";
        return value;
    } else if (event->get_type() == event::event_type_t::dictionary_event){
        return "DICTIONARY VALUES NOT IMPLEMENTED";
    } else {
        return event::lex_cast_value<std::string>(event);
    }
}

const std::string& get_format_name_no_throw(yuri::format_t fmt) {
	try {
		return yuri::core::raw_format::get_format_name(fmt);
	}
	catch(std::exception&){}
	try {
		return yuri::core::compressed_frame::get_format_name(fmt);
	}
	catch(std::exception&){}
	return "Unknown";
}

void get_cfgs(nlohmann::json& json, const std::vector<core::InputDeviceConfig>& cfgs, std::vector<std::string> order, std::vector<std::string> used) {
	if (cfgs.empty()) return;
	if (order.empty()) {
		// We used up all keys and there's still some params, so lets print them here
		for (const auto& cfg: cfgs) {
			auto keys = unused_keys(cfg, used);
			for (const auto& key: keys) {
				json[key].push_back(cfg.params[key].get<std::string>());
			}
		}
		return;
	}
	const auto& key = order[0];
	auto used_next = used;
	used_next.push_back(key);
	if (is_last_key(cfgs, key, used)) {
		for (const auto& c: yuri::core::detail::find_map_key_values<std::string>(cfgs, key)) {
			json[key].push_back(c);
		}
	} else {
		for (const auto& c: yuri::core::detail::find_submap_by_key<std::string>(cfgs, key)) {
			nlohmann::json json_submap;
			json_submap["value"] = c.first;
			get_cfgs(json_submap, c.second, std::vector<std::string>(order.begin()+1, order.end()), used_next);
			json[key].push_back(json_submap);
		}
	}
}

void get_params(nlohmann::json& json, const std::string name, const yuri::core::Parameters& params) {
	for (const auto& p: params) {
		const auto& param = p.second;
		const std::string& pname = param.get_name();
		if (pname[0] != '_') {
			nlohmann::json params_json;
			params_json["name"] = pname;
			try {
				const auto value = param_value(param.get_value());
				params_json["value"] = value;
				const std::string &d = param.get_description();
				if (!d.empty()) {
					params_json["description"] = d;
				}
			} catch (const std::exception& e) {
				params_json["value"] = "Error getting value: \"" + std::string(e.what()) + "\"";
			}
			json["params"].push_back(params_json);
		}
	}
}

void get_classes(nlohmann::json& json) {
	auto& generator = yuri::IOThreadGenerator::get_instance();
	for (const auto& name: generator.list_keys()) {
		nlohmann::json class_json;
		class_json["name"] = name;
		const auto& params = generator.configure(name);
		const std::string& desc = params.get_description();
		if (!desc.empty()) {
			class_json["description"] = desc;
		}
		get_params(class_json, name, params);
		json.push_back(class_json);
	}
}

void get_formats(nlohmann::json& json) {
	using namespace yuri;
	for (const auto& fmt: core::raw_format::formats()) {
		const auto& info = fmt.second;
		for (const auto& s_name: info.short_names) {
			json["raw_video_formats"][info.name]["short_names"].push_back(s_name);
		}
		for (const auto& plane: info.planes) {
			auto bps = plane.bit_depth.first/plane.bit_depth.second;
			json["raw_video_formats"][info.name]["planes"][plane.components] = bps;
		}
	}
	for (const auto& fmt: core::compressed_frame::formats()) {
		const auto& info = fmt.second;
		for (const auto& s_name: info.short_names) {
			json["compressed_video_formats"][info.name]["short_names"].push_back(s_name);
		}
		for (const auto& mime: info.mime_types) {
			json["compressed_video_formats"][info.name]["mime_types"].push_back(mime);
		}
	}
	for (const auto& fmt: core::raw_audio_format::formats()) {
		const auto& info = fmt.second;
		for (const auto& s_name: info.short_names) {
			json["raw_audio_formats"][info.name]["short_names"].push_back(s_name);
		}
		json["raw_audio_formats"][info.name]["bits_per_sample"] = info.bits_per_sample;
		json["raw_audio_formats"][info.name]["endianness"] = (info.little_endian?"little":"big");
	}
}

void get_converters(nlohmann::json& json) {
	using namespace yuri;
	const auto& conv = core::ConverterRegister::get_instance();
	const auto& keys = conv.list_keys();
	
	for (const auto& k: keys) {
		nlohmann::json conv_json;
		conv_json["from"] = get_format_name_no_throw(k.first);
		conv_json["to"] = get_format_name_no_throw(k.second);
		const auto& details = conv.find_value(k);
		for (const auto& d: details) {
			nlohmann::json details_json;
			details_json["node"] = d.first;
			details_json["priority"] = d.second;
			conv_json["details"].push_back(details_json);
		}
	}
}

void get_pipes(nlohmann::json& json) {
	using namespace yuri;
	const auto& generator = core::PipeGenerator::get_instance();
	for (const auto& name: generator.list_keys()) {
		const auto& params = generator.configure(name);
		const std::string& desc = params.get_description();
		nlohmann::json pipe_json;
		pipe_json["name"] = name;
		pipe_json["description"] = desc;
		get_params(pipe_json, name, params);
		json.push_back(pipe_json);
	}
}

void get_specifiers(nlohmann::json& json) {
	const auto& spec = core::utils::enumerate_string_generator_specifiers();
	for (const auto& s: spec) {
		nlohmann::json spec_json;
		spec_json["specifier"] = s.placeholder;
		spec_json["description"] = s.description;
		spec_json["accepts_width"] = s.accepts_width;
		json.push_back(spec_json);
	}
}

}
}