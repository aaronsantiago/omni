/*
 * DeckLinkOutput.h
 *
 *  Created on: Sep 15, 2011
 *      Author: worker
 */

#ifndef DECKLINKOUTPUT_H_
#define DECKLINKOUTPUT_H_

#include "yuri/decklink/DeckLinkBase.h"
#include "yuri/decklink/DeckLink3DVideoFrame.h"

namespace yuri {

namespace decklink {

class DeckLinkOutput:public DeckLinkBase, public IDeckLinkVideoOutputCallback  {
public:
	IO_THREAD_GENERATOR_DECLARATION
	static core::pParameters configure();

	DeckLinkOutput(log::Log &log_, core::pwThreadBase parent, core::Parameters &parameters)  IO_THREAD_CONSTRUCTOR;
	virtual ~DeckLinkOutput();
	void run();
protected:
	bool init();
	bool set_param(const core::Parameter &p);
	bool verify_display_mode();
	bool start_stream();
	bool stop_stream();
	inline bool enable_stream();
	bool step();
	inline shared_ptr<DeckLink3DVideoFrame> get_active_buffer();
	inline shared_ptr<DeckLink3DVideoFrame> get_next_buffer();
	inline shared_ptr<DeckLink3DVideoFrame> do_get_active_buffer();
	inline shared_ptr<DeckLink3DVideoFrame> do_get_next_buffer();
	inline void do_rotate_buffers();
	inline void rotate_buffers();
protected:
	// Methods to satisfy IUnknows interface
	virtual HRESULT STDMETHODCALLTYPE	QueryInterface (REFIID iid, LPVOID *ppv)	{return E_NOINTERFACE;}
	virtual ULONG STDMETHODCALLTYPE		AddRef ()									{return 1;}
	virtual ULONG STDMETHODCALLTYPE		Release ()									{return 1;}

	void schedule(bool force);
	virtual HRESULT STDMETHODCALLTYPE	ScheduledFrameCompleted (IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result);
	virtual HRESULT STDMETHODCALLTYPE	ScheduledPlaybackHasStopped ();
	bool fill_frame(core::pBasicFrame source, shared_ptr<DeckLink3DVideoFrame> output);
protected:
	IDeckLinkOutput* output;
	yuri::ushort_t device_index;
	BMDVideoConnection output_connection;
	yuri::uint_t width,height;
	BMDTimeValue value;
	BMDTimeScale scale;
	mutex schedule_mutex;
	core::pBasicFrame frame, frame2;
	//IDeckLinkMutableVideoFrame *act_oframe, *back_oframe;
	std::deque<shared_ptr<DeckLink3DVideoFrame> > out_frames;
	yuri::size_t frame_number;
	yuri::size_t buffer_num;
	yuri::ushort_t prebuffer;
	bool enabled;
	bool sync;
	bool stereo_mode, stereo_usable;
	bool detect_format;
	time_value last_time;
};

}

}

#endif /* DECKLINKOUTPUT_H_ */
