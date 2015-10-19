/*!
 * @file 		register.cpp
 * @author 		Zdenek Travnicek <travnicek@iim.cz>
 * @date 		19. 10. 2015
 * @copyright	Institute of Intermedia, CTU in Prague, 2015
 * 				Distributed under BSD Licence, details in file doc/LICENSE
 *
 */

#include "DelayEstimation.h"
#include "TimestampObserver.h"

namespace yuri {
namespace synchronization {

MODULE_REGISTRATION_BEGIN("synchronization")
		REGISTER_IOTHREAD("delay_estimation", DelayEstimation)
		REGISTER_IOTHREAD("timestamp_observer",TimestampObserver)
MODULE_REGISTRATION_END()

}
}


