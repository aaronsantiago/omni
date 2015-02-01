/*!
 * @file 		UVAlsaOutput.h
 * @author 		Zdenek Travnicek <travnicek@iim.cz>
 * @date 		21.10.2013
 * @copyright	CESNET, z.s.p.o, 2013
 * 				Distributed under modified BSD Licence, details in file doc/LICENSE
 *
 */

#ifndef UVALSAOUTPUT_H_
#define UVALSAOUTPUT_H_


#include "yuri/core/thread/SpecializedIOFilter.h"
#include "yuri/core/frame/RawAudioFrame.h"
extern "C" {
#include "audio/audio.h"
// Ugh, for some reason there are these ugly macros defined in config_unix.h
// Compatibility with windows? It certainly breaks lots of stuff here...
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
}
namespace yuri {
namespace uv_alsa_output {

class UVAlsaOutput: public core::SpecializedIOFilter<core::RawAudioFrame>
{
public:
	IOTHREAD_GENERATOR_DECLARATION
	static core::Parameters configure();
	UVAlsaOutput(const log::Log &log_, core::pwThreadBase parent, const core::Parameters &parameters);
	virtual ~UVAlsaOutput() noexcept;
private:
	
	virtual core::pFrame do_special_single_step(core::pRawAudioFrame frame) override;
	virtual bool set_param(const core::Parameter& param);
	bool format_changed(const core::pRawAudioFrame& frame);
	bool reconfigure(const core::pRawAudioFrame& frame);
	void* device_;
	std::string device_name_;

	format_t format_;
//	size_t channels_;
//	size_t sampling_frequency_;

	audio_frame frame_;


};

} /* namespace uv_alsa_output */
} /* namespace yuri */
#endif /* UVALSAOUTPUT_H_ */
