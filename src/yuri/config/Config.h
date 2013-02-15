#ifndef CONFIG_H_
#define CONFIG_H_
//#include "yuri/yuriconf.h"
#ifdef YURI_HAVE_LIBCONFIG
#include <libconfig.h++>
#endif
#include "yuri/log/Log.h"
#include <cstdlib>
#include <string>
#include <map>
#include <deque>
#include <algorithm>
#include <cctype>
#include "ConfigException.h"
#include <boost/filesystem.hpp>
#include "yuri/exception/InitializationFailed.h"
#include "yuri/exception/OutOfRange.h"
#include "Callback.h"
#include <boost/shared_ptr.hpp>

namespace yuri
{
namespace config
{

using namespace yuri::log;
class EXPORT Config
{
public:
	Config(Log &log);
	Config(Log &log, const char *confname);
	virtual ~Config();
	bool init_config(int argc, char **argv, bool use_file=true);
	int get_array_size(std::string &path);
	bool exists(std::string path);
	void set_callback(std::string name, pCallback func, pThreadBase data);
	shared_ptr<Callback> get_callback(std::string name);
	template<typename T> bool get_value(std::string path, T &out, T def);
	template<typename T> bool get_value(std::string path, T &out);
	template<typename T> bool get_value_from_array(std::string path, int index, T &out);
protected:
	bool read_config_file(std::string filename);
	const char * get_env(std::string path);
	Log log;
	boost::mutex config_mutex;
	std::map<std::string,shared_ptr<Callback> > callbacks;
#ifdef YURI_HAVE_LIBCONFIG
	std::deque<shared_ptr<libconfig::Config> > configs;
#endif
public: static Config* get_config(int index=0);
protected: static Config* default_config;
protected: static boost::mutex default_config_mutex;
protected: static std::string cf;
		   static std::string cfs;
};

template<typename T>  bool
	Config::get_value(std::string path, T &out, T def)
{
	if (get_value(path, out)) return true;
	out = def;
	return false;
}
template<typename T>  bool
	Config::get_value(std::string /*path*/, T &/*out*/)
{
	boost::mutex::scoped_lock l(config_mutex);
#ifdef YURI_HAVE_LIBCONFIG
	if (configs.empty()) throw (ConfigException());
	try {
		for (std::deque<shared_ptr<libconfig::Config> >::iterator i=configs.begin();
							i!=configs.end();++i) {
			if ((*i)->exists(path)) {
				(*i)->lookupValue(path,out);
				return true;
			}
		}
		return false;
	}
	catch (libconfig::SettingNotFoundException) {
		log[warning] << "Configuration option " << path << " not found" << std::endl;
	}
#endif
	return false;
}

template<>  inline bool
	Config::get_value<std::string> (std::string path, std::string &out)

{
	boost::mutex::scoped_lock l(config_mutex);
	const char * env = get_env(path);
	if (env) {
		out = env;
		return true;
	}
#ifdef YURI_HAVE_LIBCONFIG
	if (configs.empty()) throw (ConfigException());
	try {
		for (std::deque<shared_ptr<libconfig::Config> >::iterator i=configs.begin();
							i!=configs.end();++i) {
			if ((*i)->exists(path)) {
				(*i)->lookupValue(path,out);
				return true;
			}
		}
		return false;
	}
	catch (libconfig::SettingNotFoundException) {
		log[warning] << "Configuration option " << path << " not found" << std::endl;
	}
#endif
	return false;
}

template<typename T>  bool
	Config::get_value_from_array(std::string /*path*/, int /*index*/, T &/*out*/)
{
	boost::mutex::scoped_lock l(config_mutex);
#ifdef YURI_HAVE_LIBCONFIG
	if (configs.empty()) throw (ConfigException());
	try {
		for (std::deque<shared_ptr<libconfig::Config> >::iterator i=configs.begin();
				i!=configs.end();++i) {
			if ((*i)->exists(path)) {
				out = (T) (*i)->lookup(path)[index];
				return true;
			}
		}
	}
	catch (libconfig::SettingNotFoundException) {
		log[warning] << "Configuration option " << path << " not found" << std::endl;
	}
#endif
	return false;
}


template<>  inline bool
	Config::get_value_from_array<std::string> (std::string /*path*/, int /*index*/, std::string &/*out*/)
{
	boost::mutex::scoped_lock l(config_mutex);
#ifdef YURI_HAVE_LIBCONFIG
	if (configs.empty()) throw (ConfigException());
	try {
		for (std::deque<shared_ptr<libconfig::Config> >::iterator i=configs.begin();
				i!=configs.end();++i) {
			if ((*i)->exists(path)) {
				out = (const char *) (*i)->lookup(path)[index];
				return true;
			}
		}
	}
	catch (libconfig::SettingNotFoundException) {
		log[warning] << "Configuration option " << path << " not found" << std::endl;
	}
#endif
	return false;
}
}
}
#endif /*CONFIG_H_*/
