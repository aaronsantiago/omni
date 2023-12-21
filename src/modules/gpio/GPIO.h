/*!
 * @file        GPIO.h
 * @author      Jiri Melnikov <jiri@melnikoff.org>
 * @date        20.12.2023
 *              Distributed under modified BSD Licence, details in file doc/LICENSE
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "yuri/core/thread/IOThread.h"
#include "yuri/event/BasicEventConsumer.h"
#include "yuri/event/BasicEventProducer.h"

#include <linux/gpio.h>

namespace yuri {
namespace gpio {

class GPIO: public core::IOThread, public event::BasicEventConsumer, event::BasicEventProducer {
public:
    IOTHREAD_GENERATOR_DECLARATION
    static core::Parameters configure();
    GPIO(log::Log &log_, core::pwThreadBase parent, const core::Parameters &parameters);
    virtual ~GPIO() noexcept;
private:
    virtual void run();
    virtual bool set_param(const core::Parameter& param);
    virtual bool do_process_event(const std::string& event_name, const event::pBasicEvent& event);

    void gpio_output(int pin);
    void gpio_set(uint8_t value);

    std::string device_;
    int fd_;
    gpiohandle_request req_out_;
    int pin_;
    int value_;
    bool force_refresh_;
};


} /* namespace gpio */
} /* namespace yuri */
#endif /* GPIO_H_ */
