#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/view.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "lamenes.h"
#include <thread>
#include <memory>
#include "system/buttons.h"
#include "system/display.h"
#include "system/sleep.h"
#include "palette.h"
#include <unistd.h>
#include <mutex>

struct HelloTutorialModule : public pp::Module 
{
  pp::Instance* CreateInstance(PP_Instance instance);
};

class HelloTutorialInstance : public pp::Instance {
public:
 	HelloTutorialInstance(PP_Instance instance) :
  	pp::Instance(instance),
	cb_factory_(this) {
		RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
		auto tptr = std::unique_ptr<std::thread>(new std::thread([](){
			lamenes_main();
		}));
		emuthread_ = std::move(tptr);
	}

	bool HandleInputEvent(const pp::InputEvent& event) {
		switch(event.GetType()) {
			case PP_INPUTEVENT_TYPE_KEYDOWN: {
				pp::KeyboardInputEvent key_event(event);
				Log("Keyboard input %d", key_event.GetKeyCode());
			}
			default:
				return false;
		}
	}

	void InitDisplay(uint32_t status, int width, int height)
	{
  		LogToConsole(PP_LOGLEVEL_LOG, "InitDisplay");
		pp::Size size(width, height);
		context_ = pp::Graphics2D(this, size, true);
		BindGraphics(context_);
		image_ = pp::ImageData(this, PP_IMAGEDATAFORMAT_BGRA_PREMUL, size,false);
		Redraw(0);
	}
	
	void SetPixelColor(int x, int y, uint8_t r, uint8_t g, uint8_t b)
	{
		if (x > 100 || y > 100) return;
		mutex_.lock();
		uint32_t* pixptr = image_.GetAddr32({x, y});
		*pixptr = 0xff000000 + 
			(((uint32_t)r) << 16) +
			(((uint32_t)g) << 8) +
			(((uint32_t)b));
		//Log("setting pixel at %d %d, addr 0x%x", x, y, pixptr);
		mutex_.unlock();
	}
	
	void Redraw(uint32_t status)
	{
		if (status != PP_OK) return;
		//Log("Redraw");
		mutex_.lock();
		context_.PaintImageData(image_, {0, 0});
		mutex_.unlock();
  		context_.Flush(cb_factory_.NewCallback(&HelloTutorialInstance::DidFlush));
	}

	void DidFlush(int32_t status)
	{
		//Log("DidFlush");
	}

	void Log(const char* format, ...) {
		static PPB_Console* console =
  		(PPB_Console*)pp::Module::Get()->GetBrowserInterface(PPB_CONSOLE_INTERFACE);

		if (NULL == console)
        		return;
		va_list args;
		va_start(args, format);
		char buf[512];
		vsnprintf(buf, sizeof(buf) - 1, format, args);
		buf[sizeof(buf) - 1] = '\0';
		va_end(args);
		pp::Var value(buf);
		console->Log(pp_instance(), PP_LOGLEVEL_LOG, value.pp_var());
	}

	static HelloTutorialInstance* Get()
	{
		HelloTutorialModule *module = static_cast<HelloTutorialModule*>(HelloTutorialModule::Get());
		auto inst_map = module->current_instances();
		assert(inst_map.size() == 1);
		auto inst_record = inst_map.begin();
		return static_cast<HelloTutorialInstance*>(inst_record->second);
	}

	pp::CompletionCallbackFactory<HelloTutorialInstance>& CallbackFactory()
	{
		return cb_factory_;
	}
private:
	pp::Graphics2D context_;
	pp::ImageData image_;
	pp::CompletionCallbackFactory<HelloTutorialInstance> cb_factory_;
	std::unique_ptr<std::thread> emuthread_;
	std::mutex mutex_;
};

void sleep_ms(uint32_t time_ms) 
{
	usleep(time_ms * 1000);
}

void display_init(uint16_t width, uint16_t height, DisplayType display_type, bool fullscreen)
{
	auto instance = HelloTutorialInstance::Get();
	instance->Log("display_init(%d, %d)", width, height);
	auto callback = instance->CallbackFactory().NewCallback(&HelloTutorialInstance::InitDisplay, width, height);
	pp::Module::Get()->core()->CallOnMainThread(0, callback);
}

void display_lock(void) {}
void display_unlock(void) {}

void display_set_pixel(uint16_t x, uint16_t y, uint8_t nes_color)
{
	auto instance = HelloTutorialInstance::Get();
	const Palette& p = ::palette[nes_color];
	instance->SetPixelColor(x, y, p.r, p.g, p.b);
}

void display_update(void)
{
	auto instance = HelloTutorialInstance::Get();
	auto callback = instance->CallbackFactory().NewCallback(&HelloTutorialInstance::Redraw);
	pp::Module::Get()->core()->CallOnMainThread(0, callback);
}

void poll_buttons(void) {}

pp::Instance* HelloTutorialModule::CreateInstance(PP_Instance instance)
{
	return new HelloTutorialInstance(instance);
}

namespace pp {
Module* CreateModule() {
  return new HelloTutorialModule();
}
}  // namespace pp
