#include <fbjni/fbjni.h>

#include "hlp_bindings.hpp"

using namespace facebook;

jint JNI_OnLoad(JavaVM *vm, void *) {
    return jni::initialize(vm, [] {
        JHolepunchBindings::registerNatives();
    });
}
