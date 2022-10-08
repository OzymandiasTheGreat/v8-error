#ifndef hlp_macros_h
#define hlp_macros_h

#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdint>
#include <stdio.h>

#include <jsi/jsi.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

using namespace facebook;

// JSI

#define JSI_ARG_FUNCTION(arg) arguments[arg].getObject(rt).getFunction(rt)
#define JSI_ARG_INT(arg) static_cast<int>(arguments[arg].asNumber())
#define JSI_ARG_UINT32(arg) static_cast<uint32_t>(arguments[arg].asNumber())
#define JSI_ARG_STRING(arg) arguments[arg].asString(rt).utf8(rt)
#define JSI_ARG_OBJECT(arg) arguments[arg].asObject(rt)

#define JSI_NUMBER(value)\
  jsi::Value(static_cast<double>(value))

#define JSI_UTF8(value)\
  jsi::String::createFromUtf8(rt, value)

#define JSI_BOOL(value)\
  jsi::Value(value)

#define JSI_THROW(message)\
  throw jsi::JSError(rt, message); // NOLINT(cert-err60-cpp)

#define JSI_THROW_ERR(method_name, err)\
  {\
    std::stringstream fmt;\
    fmt << #method_name" error (" << err << ")";\
    JSI_THROW(fmt.str());\
  }

#define JSI_ARGV_TYPEDARRAY(name, index)\
  JSI_TYPEDARRAY(name, arguments[index])

#define JSI_TYPEDARRAY(name, val) \
  auto name##_obj = val.asObject(rt);\
  auto name##_ab = name##_obj.getProperty(rt, "buffer").asObject(rt).getArrayBuffer(rt);\
  auto name##_ptr = name##_ab.data(rt);\
  auto name##_byteOffset = static_cast<size_t>(name##_obj.getProperty(rt, "byteOffset").asNumber());\
  auto name##_data = name##_ptr + name##_byteOffset;

#define JSI_TYPEDARRAY_BYTELENGTH(name)\
  auto name##_byteLength = static_cast<size_t>(name##_obj.getProperty(rt, "byteLength").asNumber());

#define JSI_TYPEDARRAY_SIZE(name)\
  auto name##_size = static_cast<size_t>(name##_obj.getProperty(rt, "length").asNumber());

#define JSI_MAKE_UINT8ARRAY(name, base, len)\
  jsi::Function name##_ctor = rt.global().getPropertyAsFunction(rt, "Uint8Array");\
  jsi::Object name = name##_ctor.callAsConstructor(rt, static_cast<double>(len)).getObject(rt);\
  jsi::ArrayBuffer name##_buffer = name\
    .getProperty(rt, jsi::PropNameID::forAscii(rt, "buffer"))\
    .asObject(rt)\
    .getArrayBuffer(rt);\
  memcpy(name##_buffer.data(rt), base, len);

#define JSI_HOSTOBJECT_METHOD(arg_name, arg_argc, code)\
  if (propertyName == arg_name) {\
    return jsi::Function::createFromHostFunction(rt, name, arg_argc, [this](jsi::Runtime &rt, jsi::Value const&, jsi::Value const *arguments, size_t argc) \
    code \
    );\
  }

#define JSI_GLOBAL_METHOD(arg_name, arg_argc, code)\
  rt.global().setProperty(rt, arg_name, jsi::Function::createFromHostFunction(rt, jsi::PropNameID::forAscii(rt, arg_name), arg_argc, [](jsi::Runtime &rt, jsi::Value const&, jsi::Value const *arguments, size_t argc) \
  code \
  ));

#define JSI_HOSTOBJECT_UTF8(arg_name, str)\
  if (propertyName == arg_name) {\
    return JSI_UTF8(str);\
  }

#define JSI_HOSTOBJECT_NUMBER(arg_name, n)\
  if (propertyName == arg_name) {\
    return JSI_NUMBER(n);\
  }

#define JSI_UNDEFINED jsi::Value::undefined()

#define JSI_NULL jsi::Value::null()

#define JSI_ERROR(error_str)\
  rt.global().getPropertyAsFunction(rt, "Error").callAsConstructor(rt, error_str).getObject(rt)

#define JSI_MAKE_UV_ERROR(pname, code, binding)\
  auto pname##_errData = binding->get_uv_error(code);\
  auto pname##_err = rt.global().getPropertyAsFunction(rt, "Error").callAsConstructor(rt, pname##_errData.message).asObject(rt);\
  pname##_err.setProperty(rt, "errno", code);\
  pname##_err.setProperty(rt, "code", pname##_errData.name);

#define JSI_CALL_THROWER(err)\
  rt.global().getPropertyAsFunction(rt, "__hlp_thrower").call(rt, err);

// logs

#ifdef __ANDROID__
#define LOG_THREAD_ID(msg) \
  {\
    std::stringstream fmt;\
    fmt << "*** (" << std::this_thread::get_id() << ") " << msg << "\n"; \
    HLP_LOG("%s", fmt.str().c_str());\
  }
#else
#define LOG_THREAD_ID(msg) std::cout << "*** (" << std::this_thread::get_id() << ") " << msg << "\n";
#endif

#ifdef __ANDROID__
#define HLP_LOG(...) __android_log_print(ANDROID_LOG_INFO, "Holepunch", __VA_ARGS__);
#else
#define HLP_LOG(...) printf(__VA_ARGS__);
#endif

#endif /* hlp_macros_h */
