#ifndef uv_hpp
#define uv_hpp

#include <uv.h>
#include <unordered_set>
#include <thread>
#include <memory>

#include <jsi/jsi.h>

#include "hlp_CallInvokerGetter.h"

#define HLP_UV_GLOBAL "__hlp_uv"

namespace holepunch {
namespace uv {

using namespace holepunch;
using namespace facebook;

struct UVError {
  int value;
  const char* name;
  const char* message;
};

class UVBinding : public jsi::HostObject {
private:
  std::shared_ptr<CallInvokerGetter> _callInvoker;
  std::thread _thread;

  uv_loop_t* _uv_loop = nullptr;
  uv_async_t* _uv_async = nullptr;
  
public:
  UVBinding(std::shared_ptr<CallInvokerGetter> callInvoker): _callInvoker(callInvoker) {};
  
  void worker();

  void reload(jsi::Runtime& rt);

  uv_loop_t* uv_loop() {
    return _uv_loop;
  }
  
  // TODO: this should be private
  std::unordered_set<uv_file> fds;
  
  static UVError get_uv_error(int);
  jsi::Value get(jsi::Runtime &rt, jsi::PropNameID const &name) override;
};

void install(jsi::Runtime& jsiRuntime, std::shared_ptr<CallInvokerGetter> callInvoker);
void reloadSignal(jsi::Runtime& jsiRuntime);

}}

#endif /* uv_hpp */
