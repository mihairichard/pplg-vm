#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/core.h"
#include "ppapi/cpp/completion_callback.h"

struct HelloTutorialModule;

struct HelloTutorialInstance : public pp::Instance {

  HelloTutorialInstance(PP_Instance instance);
  ~HelloTutorialInstance() {}
  
  void HandleMessage(const pp::Var& var_message) {
  		
  }
};

struct HelloTutorialModule : public pp::Module {
  HelloTutorialModule() {}
  ~HelloTutorialModule() {}
  
  pp::Instance* CreateInstance(PP_Instance instance) {
    auto inst = new HelloTutorialInstance(instance);
    return inst;
  }
};

  HelloTutorialInstance::HelloTutorialInstance(PP_Instance instance) :
  pp::Instance(instance) 
  {
  }

namespace pp {
Module* CreateModule() {
  return new HelloTutorialModule();
}
}  // namespace pp
