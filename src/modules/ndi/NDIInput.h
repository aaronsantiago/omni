/*!
 * @file        NDIInput.h
 * @author      Jiri Melnikov <jiri@melnikoff.org>
 * @date        20.12.2023
 *              Distributed under modified BSD Licence, details in file doc/LICENSE
 */

#ifndef NDIINPUT_H_
#define NDIINPUT_H_

#include "yuri/core/thread/IOThread.h"
#include "yuri/event/BasicEventConsumer.h"
#include "yuri/core/thread/InputThread.h"
#include "yuri/event/BasicEventConsumer.h"
#include "yuri/event/BasicEventProducer.h"

#include <Processing.NDI.Lib.h>

namespace yuri {
namespace ndi {

const size_t ndi_source_max_wait_ms = 250;
const int ndi_source_max_queue_frames = 3;

class NDIInput:public core::IOThread, public event::BasicEventProducer, public event::BasicEventConsumer {
public:
	NDIInput(log::Log &log_,core::pwThreadBase parent, const core::Parameters &parameters);
	virtual ~NDIInput() noexcept;
	virtual void run() override;
	IOTHREAD_GENERATOR_DECLARATION
	static core::Parameters configure();
	static std::vector<core::InputDeviceInfo> enumerate();
	void sound_receiver();
private:
	const NDIlib_source_t* get_source(std::string name, int *position);

	virtual bool set_param(const core::Parameter &param) override;
	virtual bool do_process_event(const std::string& event_name, const event::pBasicEvent& event);

	void emit_events();

	std::string stream_;
	std::string backup_;
	std::string format_;
	std::string ndi_path_;
	int nodata_timout_;
	bool audio_enabled_;
	bool audio_running_;
	bool lowres_enabled_;
	int reference_level_;
	position_t audio_pipe_;
	std::atomic<size_t> stream_fail_;

	duration_t event_time_;
	Timer event_timer_;

	const NDIlib_v5* NDIlib_;
	NDIlib_recv_instance_t ndi_receiver_;
	NDIlib_find_instance_t ndi_finder_;
	bool ptz_supported_;

	float last_pan_val_;
	float last_tilt_val_;
	float last_pan_speed_;
	float last_tilt_speed_;
};

}

}

#endif /* NDIINPUT_H_ */
