/*!
 * @file        register.cpp
 * @author      Jiri Melnikov <jiri@melnikoff.org>
 * @date        20.12.2023
 *              Distributed under modified BSD Licence, details in file doc/LICENSE
 */

#include "yuri/core/thread/IOThreadGenerator.h"

#if YURI_LINUX

#include "GPIO.h"

namespace yuri {

MODULE_REGISTRATION_BEGIN("gpio")
    REGISTER_IOTHREAD("gpio", gpio::GPIO)
MODULE_REGISTRATION_END()

}

#endif
