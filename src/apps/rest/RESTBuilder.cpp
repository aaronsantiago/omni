/*
 * RESTBuilder.cpp
 *
 * Author: kafe
 */

#include "RESTBuilder.h"
#include "yuri/core/thread/builder_utils.h"
#include <boost/regex.hpp>
#include <stdexcept>
#include <algorithm>

namespace yuri {
namespace web {



RESTBuilder::RESTBuilder(const log::Log& log_, core::pwThreadBase parent):GenericBuilder(log_, parent,"rest") {
	core::builder::load_builtin_modules(log);
}

void RESTBuilder::request_start() {
	log[log::info] << "Graph start requested.";
	set_graph(nodes_, links_);
}

// void RESTBuilder::request_end(const int signal) {
// 	log[log::info] << "Graph stop requested.";
// 	GenericBuilder::request_end(signal);
// }

bool RESTBuilder::add_node(std::string name, std::string class_name, core::Parameters params) {
	core::node_record_t node = {name, class_name, params, {}};
	nodes_[node.name]=node;

	return true;
}

bool RESTBuilder::add_link(std::string name, std::string class_name, core::Parameters params, std::string source, std::string destination) {
	links_[name]={name, class_name, params, source, destination, 0, 0, {}};

	return true;
}


}
}


