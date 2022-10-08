#include "uv.hpp"

#include <poll.h>
#include <map>

#include "hlp_macros.h"

using namespace facebook;
using namespace holepunch::uv;

// https://github.com/nodejs/node/blob/5fad0b93667ffc6e4def52996b9529ac99b26319/src/uv.cc

#if defined(_MSC_VER) && _MSC_VER < 1900
#define arraysize(a) (sizeof(a) / sizeof(*a))  // Workaround for VS 2013.
#else
template <typename T, size_t N>
constexpr size_t arraysize(const T(&)[N]) { return N; }
#endif

namespace holepunch::uv {

static const struct UVError uv_errors_map[] = {
#define V(name, message) {UV_##name, #name, message},
    UV_ERRNO_MAP(V)
#undef V
};

}

UVError UVBinding::get_uv_error(int err) {
  static const std::map<int, UVError> error_map {
#define V(name, message) {UV_##name, {UV_##name, #name, message}},
    UV_ERRNO_MAP(V)
#undef V
  };
  return error_map.at(err);
}

static void
on_walk(uv_handle_t* handle, void*) {
  printf("*** will close uv_handle %s\n", uv_handle_type_name(handle->type));

  if (!uv_is_closing(handle)) {
    uv_close(handle, nullptr);
  }
}

static void
on_async(uv_async_t* handle) {
  uv_close((uv_handle_t*) handle, nullptr);
}

void UVBinding::worker() {
  uv_sem_t sem;
  uv_sem_init(&sem, 0);

  bool loop = true;
  while (loop) {
    int rc;
    do {
      struct pollfd p{};
      p.fd = uv_backend_fd(_uv_loop);
      p.events = POLLIN;
      p.revents = 0;
      rc = poll(&p, 1, uv_backend_timeout(_uv_loop));
    } while (rc == -1 && errno == EINTR);

    loop = uv_loop_alive(_uv_loop) && uv_is_active((uv_handle_t*)_uv_async);
    if (loop && _callInvoker->isValid()) {
      _callInvoker->get()->invokeAsync([this, &sem]() {
        uv_run(_uv_loop, UV_RUN_NOWAIT);
        uv_sem_post(&sem);
      });
      uv_sem_wait(&sem);
    }
  }
  HLP_LOG("*** did exit thread loop\n");

  uv_sem_destroy(&sem);
}

void UVBinding::reload(jsi::Runtime& rt) {
  LOG_THREAD_ID("reload");

  if (_uv_loop == nullptr) {
    return;
  }

  uv_walk(_uv_loop, on_walk, nullptr);

  if (_thread.joinable()) {
    HLP_LOG("*** will wait thread\n");
    _thread.join();
    HLP_LOG("*** thread finished\n");
  }

  // TODO: why do we need this?? closing handlers should close fds, right?
  for(auto & fd : fds) {
    uv_fs_t req;
    uv_fs_close(_uv_loop, &req, uv_get_osfhandle(fd), nullptr);
    HLP_LOG("*** did close fd\n");
  }
  fds.clear();

  HLP_LOG("*** will finish loop\n");
  uv_run(_uv_loop, UV_RUN_DEFAULT);
  HLP_LOG("*** did finish loop\n");

  int res = uv_loop_close(_uv_loop);
  HLP_LOG("*** loop did close (%d)\n", res);

  free(_uv_async);
  free(_uv_loop);
}

jsi::Value UVBinding::get(jsi::Runtime &rt, jsi::PropNameID const &name) {
  auto propertyName = name.utf8(rt);

  JSI_HOSTOBJECT_METHOD("setup", 0, {
    LOG_THREAD_ID("will setup");

    _uv_loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    uv_loop_init(_uv_loop);

    _uv_async = (uv_async_t*)malloc(sizeof(uv_async_t));
    uv_async_init(_uv_loop, _uv_async, on_async);

    uv_run(_uv_loop, UV_RUN_NOWAIT);

    _thread = std::thread(&UVBinding::worker, this);

    LOG_THREAD_ID("did setup");

    return jsi::Value::undefined();
  });

  return jsi::Value::undefined();
}

void holepunch::uv::install(jsi::Runtime& rt, std::shared_ptr<CallInvokerGetter> callInvoker) {
  jsi::Function mapCtor = rt.global().getPropertyAsFunction(rt, "Map");
  jsi::Function arrayCtor = rt.global().getPropertyAsFunction(rt, "Array");

  auto errorsArr = arrayCtor.callAsConstructor(rt).asObject(rt);
  auto pushFn = errorsArr.getPropertyAsFunction(rt, "push");
  size_t errors_len = arraysize(uv_errors_map);
  for (size_t i = 0; i < errors_len; ++i) {
    const auto& error = uv_errors_map[i];
    pushFn.callWithThis(rt, errorsArr, arrayCtor.callAsConstructor(rt, error.value, arrayCtor.callAsConstructor(rt, error.name, error.message).asObject(rt)).asObject(rt));
  }

  auto errorMapObj = mapCtor.callAsConstructor(rt, errorsArr).asObject(rt);
  rt.global().setProperty(rt, "__hlp_uv_error_map", std::move(errorMapObj));

  auto obj = std::make_shared<UVBinding>(callInvoker);
  jsi::Object hostObject = jsi::Object::createFromHostObject(rt, obj);
  rt.global().setProperty(rt, HLP_UV_GLOBAL, std::move(hostObject));
}

void holepunch::uv::reloadSignal(jsi::Runtime &rt) {
  if (!rt.global().hasProperty(rt, HLP_UV_GLOBAL)) {
    return;
  }

  auto uv_obj = rt.global().getProperty(rt, HLP_UV_GLOBAL).asObject(rt);
  auto uv = uv_obj.asHostObject<holepunch::uv::UVBinding>(rt);
  uv->reload(rt);
}
