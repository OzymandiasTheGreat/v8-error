#ifndef fs_hpp
#define fs_hpp

#include <uv.h>

#include <jsi/jsi.h>

#include "uv.hpp"
#include "hlp_CallInvokerGetter.h"

using namespace facebook;
using namespace holepunch;

namespace holepunch {
namespace fs {

typedef enum {
  FS_EXT_RDLOCK = 1,
  FS_EXT_WRLOCK,
} fs_ext_lock_type_t;

class FSBinding;

struct FSRequest {
  FSRequest() {
    uv_req.data = static_cast<void*>(this);
  }

  uv_fs_t uv_req;

  FSBinding* binding;
  jsi::Runtime* rt;
  std::unique_ptr<jsi::Function> callback;
  uv_buf_t buffer;
};

class FSBinding : public  jsi::HostObject {
private:
  std::shared_ptr<CallInvokerGetter> _callInvoker;
  std::shared_ptr<uv::UVBinding> _uvBinding;

public:
  FSBinding(std::shared_ptr<CallInvokerGetter> callInvoker): _callInvoker(callInvoker) {};
  ~FSBinding() = default;

  jsi::Value get(jsi::Runtime &rt, jsi::PropNameID const &name) override;
};

void install(jsi::Runtime& jsiRunntime, std::shared_ptr<CallInvokerGetter> callInvoker);

}}

#endif //fs_hpp
