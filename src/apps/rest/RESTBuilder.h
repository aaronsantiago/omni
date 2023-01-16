/*
 * RESTBuilder.h
 *
 * Author: kafe
 */

#ifndef RESTBUILDER_H_
#define RESTBUILDER_H_

#include "yuri/core/thread/GenericBuilder.h"
#include <vector>
#include <string>

namespace yuri {
namespace web {

class RESTBuilder: public yuri::core::GenericBuilder {
public:
	RESTBuilder(const log::Log& log_, core::pwThreadBase parent);
	void request_start();
	bool add_node(std::string name, std::string class_name, core::Parameters params);
	bool add_link(std::string name, std::string class_name, core::Parameters params, std::string source, std::string destination);
	// void request_end(const int x);
private:
	core::node_map nodes_;
	core::link_map links_;
};


}
}




#endif /* RESTBUILDER_H_ */
