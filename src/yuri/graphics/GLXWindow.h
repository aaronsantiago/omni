#ifndef GLXWINDOW_H_
#define GLXWINDOW_H_
#include "yuri/config/Config.h"
#include "WindowBase.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#ifndef GLX_GLXEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glx.h>
#include <map>
#include <boost/shared_ptr.hpp>

//define GLXWINDOW_USING_GLOBAL_MUTEX

#ifdef YURI_HAVE_MOTIF
	#include <Xm/MwmUtil.h>
#endif


namespace yuri
{
namespace graphics
{
using namespace yuri::log;
using yuri::threads::ThreadBase;
using yuri::threads::pThreadBase;

class GLXWindow: public WindowBase
{
protected:
	static std::map<GLXContext,shared_ptr<GLXWindow> > used_contexts;
	static boost::mutex contexts_map_mutex;
public:
	static void add_used_context(GLXContext ctx,shared_ptr<GLXWindow> win);
	static void remove_used_context(GLXContext ctx);
	static bool is_context_used(GLXContext ctx);


	GLXWindow(Log &log_,pThreadBase parent, Parameters &p);
	virtual ~GLXWindow();
	static shared_ptr<Parameters> configure();

	virtual bool create();
	virtual bool create_window();
	virtual void setHideDecoration(bool value);
	virtual void addAttributes(int no,GLint *attrs);
	virtual void addAttribute(GLint attr);
	virtual void show(bool /*value*/=true);

	inline Display *getDisplay() { return display.get(); }
	inline Window getWindow() { return win; }
	virtual void swap_buffers();
	virtual bool process_events();
	std::string get_keyname(int key);
	virtual bool check_key(int keysym);
	virtual void exec(shared_ptr<yuri::config::Callback> c);
	virtual bool have_stereo();
	virtual bool set_vsync(bool state);
protected:
	void initAttr();
	bool load_config();
	void move();
	virtual void do_move();
	virtual void do_show();
	std::string do_get_keyname(int key);
	bool resize(unsigned int w, unsigned int h);

	std::string 			screen;
	shared_ptr<Display>		display;
	Window					root, win;
	shared_array<GLint>		attributes;
	int 					noAttr;
	XVisualInfo*			vi;
	Colormap				cmap;
	XSetWindowAttributes    swa;
	GLXContext				glc;
	XWindowAttributes       gwa;
	XEvent                  xev;
	bool 					override_redirect;
	bool 					hideDecoration;
	bool 					use_stereo;
	bool 					show_cursor;
	int 					screen_number;
	std::string				winname;
#ifdef GLXWINDOW_USING_GLOBAL_MUTEX
	static boost::mutex	global_mutex;
#else
	boost::mutex	local_mutex;
#endif
	bool vsync;
};
/*
struct VIDeleter{
	VIDeleter(shared_ptr<Display> d):d(d) {}
	void operator()(XVisualInfo*) {}
	shared_ptr<Display> d;
};
*/
struct DisplayDeleter{
	void operator()(Display*d) { XCloseDisplay(d); }
};
}
}
#endif
#endif /*GLXWINDOW_H_*/
