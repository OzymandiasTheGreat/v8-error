#ifndef bindings_hpp
#define bindings_hpp

#include <memory>
#include <fbjni/fbjni.h>
#include <ReactCommon/CallInvokerHolder.h>
#include <jni.h>
#include <android/log.h>

#include "uv.hpp"
#include "fs.hpp"

using namespace facebook;

struct JHolepunchCatalyst : jni::JavaClass<JHolepunchCatalyst> {
    static constexpr auto kJavaDescriptor = "Lcom/facebook/react/bridge/CatalystInstanceImpl;";

    bool isDestroyed() {
      static const auto method = getClass()->getMethod<jboolean()>("isDestroyed");
      return method(self());
    }
};

struct AndroidCallInvokerGetter : holepunch::CallInvokerGetter {
    std::shared_ptr<react::CallInvoker> _callInvoker;
    jni::global_ref<JHolepunchCatalyst> _catalyst;

    bool isValid() override {
      jni::ThreadScope scope;
      return !_catalyst->isDestroyed();
    }

    std::shared_ptr<react::CallInvoker> get() override {
      return _callInvoker;
    }
};

struct JHolepunchBindings : jni::JavaClass<JHolepunchBindings> {
    static constexpr auto kJavaDescriptor = "Lto/holepunch/HolepunchBindingsPackage;";

    // java > cpp

    static void registerNatives() {
      javaClassStatic()->registerNatives({
                                                 makeNativeMethod("nativeInstall", JHolepunchBindings::nativeInstall),
                                                 makeNativeMethod("nativeReloadSignal", JHolepunchBindings::nativeReloadSignal)
                                         });
    }

    static void nativeInstall(
            jni::alias_ref<jni::JClass>, jlong jsi,
            jni::alias_ref<react::CallInvokerHolder::javaobject> j_callInvoker,
            jni::alias_ref<JHolepunchCatalyst::javaobject> j_catalyst) {
      auto rt = reinterpret_cast<jsi::Runtime*>(jsi);
      auto getter = std::make_shared<AndroidCallInvokerGetter>();
      getter->_callInvoker = j_callInvoker->cthis()->getCallInvoker();
      getter->_catalyst = jni::make_global(j_catalyst);

      holepunch::uv::install(*rt, getter);
      holepunch::fs::install(*rt, getter);
    }

    static void nativeReloadSignal(jni::alias_ref<jni::JClass>, jlong jsi) {
      auto rt = reinterpret_cast<jsi::Runtime*>(jsi);
      holepunch::uv::reloadSignal(*rt);
    }
};

#endif //bindings_hpp
