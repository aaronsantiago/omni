/*!
 * @file        GPIO.cpp
 * @author      Jiri Melnikov <jiri@melnikoff.org>
 * @date        20.12.2023
 *              Distributed under modified BSD Licence, details in file doc/LICENSE
 */

#include "GPIO.h"
#include "yuri/core/Module.h"

#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>

namespace yuri {
namespace gpio {

IOTHREAD_GENERATOR(GPIO)


core::Parameters GPIO::configure() {
    core::Parameters p = core::IOThread::configure();
    p.set_description("GPIO");
    p["device"]["GPIO device, defaults to \"/dev/gpiochip0\""]="/dev/gpiochip0";  
    p["pin"]["GPIO pin number"]=0;
    p["value"]["GPIO value"]=0;
    p["force_refresh"]["Force setting the GPIO value even if the last value/pin was the same."]=true;
    return p;
}

GPIO::GPIO(log::Log &log_, core::pwThreadBase parent, const core::Parameters &parameters): core::IOThread(log_,parent,0,0,std::string("gpio")), event::BasicEventConsumer(log), event::BasicEventProducer(log), device_("/dev/gpiochip0"), fd_(-1), pin_(-1), value_(0), force_refresh_(false) {
    IOTHREAD_INIT(parameters);
    set_latency(5_ms);
}

GPIO::~GPIO() noexcept {
}

void GPIO::run() {
    fd_ = ::open(device_.c_str(), O_RDONLY);
    if (fd_ < 0) {
        log[log::error] << "Unabled to open: " << device_ << ", error: " << strerror(errno);
        return;
    }

    struct gpiochip_info info;
    int ret = ioctl(fd_, GPIO_GET_CHIPINFO_IOCTL, &info);
    if (ret == -1) {
        log[log::error] << "Unable to get chip info from ioctl: " << strerror(errno);
        ::close(fd_);
        return;
    }

    log[log::info] << "Chip name: " << info.name << ", label: " << info.label << ", lines: " << info.lines;

    if (pin_) {
        try {
            gpio_output(pin_);
            gpio_set(value_);
        } catch(const std::exception& e) {
            log[log::error] << "Error occured while setting GPIO: " << e.what();
        }
    }

    while (still_running()) {
        wait_for_events(get_latency());
        process_events();
    }

    if (req_out_.fd > 0) ::close(req_out_.fd);
    ::close(fd_);
}

void GPIO::gpio_output(int pin) {
    if (req_out_.fd > 0) ::close(req_out_.fd);
    req_out_.lineoffsets[0] = pin;
    req_out_.lines = 1;
    req_out_.flags = GPIOHANDLE_REQUEST_OUTPUT;
    int ret = ioctl(fd_, GPIO_GET_LINEHANDLE_IOCTL, &req_out_);
    if (ret == -1) {
        throw std::runtime_error("Unable to line handle from ioctl: "+std::string(strerror(errno)));
    }
}

void GPIO::gpio_set(uint8_t value) {
    struct gpiohandle_data data;
    data.values[0] = value;
    int ret = ioctl(req_out_.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
    if (ret == -1) {
        throw std::runtime_error("Unable to set line values from ioctl: "+std::string(strerror(errno)));
    }
}

bool GPIO::set_param(const core::Parameter &param) {
    if (assign_parameters(param)
            (device_, "device")
            (force_refresh_, "force_refresh")
            (pin_, "pin")
            (value_, "value"))
        return true;
    return IOThread::set_param(param);
}


bool GPIO::do_process_event(const std::string& event_name, const event::pBasicEvent& event) {
    if (iequals(event_name,"force_refresh")) {
        auto val = event::get_value<event::EventBool>(event);
        force_refresh_ = val;
    } else if (iequals(event_name,"pin")) {
        auto val = event::get_value<event::EventInt>(event);
        if (val != pin_ || force_refresh_) {
            pin_ = val;
            gpio_output(pin_);
        }
    } else if (iequals(event_name,"value")) {
        int val = 0;
        if (event->get_type() == event::event_type_t::integer_event) {
            val = event::get_value<event::EventInt>(event);
        } else if (event->get_type() == event::event_type_t::boolean_event) {
            val = event::get_value<event::EventBool>(event)?1:0;
        } else {
            log[log::warning] << "Unsupported event type for value!";
        }
        if (val < 0 || val > 255) log[log::warning] << "Value shoud be 0-255! Received: " << val;
        if (val != value_ || force_refresh_) {
            value_ = val;
            gpio_set(value_);
        }
    }
    return false;
}

} /* namespace gpio */
} /* namespace yuri */
