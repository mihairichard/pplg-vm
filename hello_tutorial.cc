#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/view.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/utility/completion_callback_factory.h"

struct HelloTutorialModule;

struct HelloTutorialInstance : public pp::Instance {

 	HelloTutorialInstance(PP_Instance instance) :
  	pp::Instance(instance),
	cb_factory_(this) {}

	void DidChangeView(const pp::View& view) {
  		LogToConsole(PP_LOGLEVEL_LOG, "DidChangeView");
		auto size = view.GetRect().size();
		context_ = pp::Graphics2D(this, size, true);
		BindGraphics(context_);
		image_ = pp::ImageData(this, PP_IMAGEDATAFORMAT_BGRA_PREMUL, size,false);
		for (int i=0; i < size.height(); i++) {
			for (int j = 0; j < size.width(); j++) {
				uint32_t* pptr = image_.GetAddr32({j, i});
				*pptr = 0xff000000 + ((i+j)&0x00ffffff);
			}
		}
		context_.ReplaceContents(&image_);
  		Log("Flush %d", context_.Flush(cb_factory_.NewCallback(&HelloTutorialInstance::DidFlush)));
	}

	void DidFlush(int32_t status) {
  		Log("DidFlush %d", status);
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

	pp::Graphics2D context_;
	pp::ImageData image_;
	pp::CompletionCallbackFactory<HelloTutorialInstance> cb_factory_;
};

struct HelloTutorialModule : public pp::Module {
  pp::Instance* CreateInstance(PP_Instance instance) {
    auto inst = new HelloTutorialInstance(instance);
    return inst;
  }
};

namespace pp {
Module* CreateModule() {
  return new HelloTutorialModule();
}
}  // namespace pp
