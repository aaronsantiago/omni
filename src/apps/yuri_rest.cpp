/*
 * yuri_rest.cpp
 *
 * Author: kafe
 */

#include "rest/RESTBuilder.h"
#include "rest/rest_utils.h"
#include "yuri/exception/InitializationFailed.h"
#include "yuri/core/thread/FixedMemoryAllocator.h"
#include "yuri/core/utils/array_range.h"
#include "yuri/core/thread/InputRegister.h"
#include "yuri/core/thread/InputThread.h"
#include "yuri/yuri_listings.h"
#include <string>
#include <memory>

#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

yuri::log::Log l(std::clog);
std::shared_ptr<yuri::web::RESTBuilder> builder;
bool running = true;

#if defined YURI_POSIX
#include <signal.h>
#include <string.h>
static void sigHandler(int sig, siginfo_t *siginfo, void *context);
static struct sigaction act;


void sigHandler(int sig, siginfo_t */*siginfo*/, void */*context*/) {
#if !defined YURI_APPLE && !defined YURI_BSD
	if (sig==SIGRTMIN) {
		l[yuri::log::warning] << "Realtime signal 0! Ignoring...";
		return;
	}
#endif
	if (sig==SIGPIPE) {
		// Sigpipe needs to be ignored, otherwise application may get killed randomly
		return;
	}
	if (builder)
		builder->request_end(yuri::core::yuri_exit_interrupted);
	running = false;
	act.sa_handler = SIG_DFL;
	act.sa_flags &= ~SA_SIGINFO;
	sigaction(SIGINT,&act,0);
}
#endif

using namespace yuri;

void fill_response(Pistache::Http::ResponseWriter& response) {
	using namespace Pistache::Http;
	auto mime = Mime::MediaType::fromString("application/json");
    response.headers().add<Header::AccessControlAllowOrigin>("*");
    response.headers().add<Header::AccessControlAllowHeaders>("Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With");
    response.headers().add<Header::AccessControlAllowMethods>("POST, GET, PUT, DELETE, OPTIONS");
    response.headers().add<Header::ContentType>(mime);
}

void add_node(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	if (!builder)
		builder = std::make_shared<yuri::web::RESTBuilder> (l, yuri::core::pwThreadBase{});
	
	nlohmann::json j_request = nlohmann::json::parse(request.body());

	yuri::core::Parameters params;	
	for (auto& el : j_request["params"].items()) {
		params[el.key()] = el.value().dump();
	}
	builder->add_node(j_request["name"], j_request["class"], params);
	fill_response(response);
    response.send(Pistache::Http::Code::Ok, "{\"message\":\"Node added.\"}");
}

void add_link(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	if (!builder)
		builder = std::make_shared<yuri::web::RESTBuilder> (l, yuri::core::pwThreadBase{});
	
	nlohmann::json j_request = nlohmann::json::parse(request.body());

	yuri::core::Parameters params;	
	for (auto& el : j_request["params"].items()) {
		params[el.key()] = el.value().dump();
	}
	builder->add_link(j_request["src"].get<std::string>()+"_"+j_request["dst"].get<std::string>(), j_request["class"], params, j_request["src"], j_request["dst"]);
	fill_response(response);
    response.send(Pistache::Http::Code::Ok, "{\"message\":\"Link added.\"}");
}

void start_impl() {
	builder->request_start();
	try {
		(*builder)();
		l[yuri::log::info] << "Application successfully finished";
		builder.reset();
	} catch (yuri::exception::Exception &e) {
		l[yuri::log::fatal] << "Application failed to start: " << e.what();
	} catch(std::exception &e) {
		l[yuri::log::fatal] << "An error occurred during execution: " << e.what();
	}
	if (builder) builder.reset();
}

void start(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
	if (!builder)
		builder = std::make_shared<yuri::web::RESTBuilder> (l, yuri::core::pwThreadBase{});

	fill_response(response);
	if (builder && builder->running()) {
    	response.send(Pistache::Http::Code::Bad_Request, "{\"message\":\"Application is already running.\"}");
	} else {
		std::thread {start_impl}.detach();
		response.send(Pistache::Http::Code::Ok, "{\"message\":\"Application started.\"}");
	}
}

void stop(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
	if (builder)
		builder->request_end(yuri::core::yuri_exit_interrupted);
	fill_response(response);
    response.send(Pistache::Http::Code::Ok, "{\"message\":\"Application stopped.\"}");
}

void inputs(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
	if (!builder)
		builder = std::make_shared<yuri::web::RESTBuilder> (l, yuri::core::pwThreadBase{});
	nlohmann::json j_answer;
	const auto& conv = core::InputRegister::get_instance();
	const auto& keys = conv.list_keys();
	for (const auto& k: keys) {
		j_answer[k] = {};
		try {
			auto enumerate = conv.find_value(k);
			auto devices = enumerate();
			for (const auto &d: devices) {
				j_answer[k]["name"] = d.device_name;
				yuri::web::get_cfgs(j_answer[k], d.configurations, d.main_param_order, std::vector<std::string>{});
			}
		} catch (...) {
			l[log::info] << "No input thread found for " << k;
		}
	}
	fill_response(response);
    response.send(Pistache::Http::Code::Ok, j_answer.dump());
}

typedef void (*listing_function)(nlohmann::json& json);

void generic_answer(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter& response, listing_function p_func) {
	if (!builder)
		builder = std::make_shared<yuri::web::RESTBuilder> (l, yuri::core::pwThreadBase{});
	nlohmann::json j_answer;
	p_func(j_answer);
	fill_response(response);
    response.send(Pistache::Http::Code::Ok, j_answer.dump());
}

void classes(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	generic_answer(request, response, yuri::web::get_classes);
}

void formats(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	generic_answer(request, response, yuri::web::get_formats);
}

void converters(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	generic_answer(request, response, yuri::web::get_converters);
}

void pipes(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	generic_answer(request, response, yuri::web::get_pipes);
}

void specifiers(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	generic_answer(request, response, yuri::web::get_specifiers);
}

int main(int argc, char** argv) {
#if defined YURI_POSIX
	memset (&act, '\0', sizeof(act));
	act.sa_sigaction = &sigHandler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGINT,&act,0);
    sigaction(SIGTERM,&act,0);
	sigaction(SIGPIPE,&act,0);
#if !defined YURI_APPLE && !defined YURI_BSD
	sigaction(SIGRTMIN,&act,0);
#endif
#endif

	l.set_flags(yuri::log::info|yuri::log::show_level|yuri::log::use_colors|yuri::log::show_time|yuri::log::show_date);
	int ret = 0;

	auto opts = Pistache::Http::Endpoint::options().threads(2).flags(Pistache::Tcp::Options::ReuseAddr);
	Pistache::Port port(8888);
    Pistache::Address addr("0.0.0.0", port);
	Pistache::Rest::Router router;

    auto http_endpoint = std::make_shared<Pistache::Http::Endpoint>(addr);
    http_endpoint->init(opts);

	{
		using namespace Pistache::Rest;
		Routes::Post(router, "/add_node",   Routes::bind(&add_node   ));
		Routes::Post(router, "/add_link",   Routes::bind(&add_link   ));
		Routes::Get (router, "/start",      Routes::bind(&start      ));
		Routes::Get (router, "/stop",       Routes::bind(&stop       ));
		Routes::Get (router, "/inputs",     Routes::bind(&inputs     ));
		Routes::Get (router, "/classes",    Routes::bind(&classes    ));
		Routes::Get (router, "/formats",    Routes::bind(&formats    ));
		Routes::Get (router, "/converters", Routes::bind(&converters ));
		Routes::Get (router, "/pipes",      Routes::bind(&pipes      ));
		Routes::Get (router, "/specifiers", Routes::bind(&specifiers ));
	}

	http_endpoint->setHandler(router.handler());
	http_endpoint->serveThreaded();

	l[yuri::log::info] << "Server started on port " << http_endpoint->getPort();

	while (running){
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	http_endpoint->shutdown();

	auto mp = yuri::core::FixedMemoryAllocator::clear_all();
	l[yuri::log::info] << "Memory pool cleared ("<< mp.first << " blocks, " << mp.second << " bytes)";

	return ret;
}


