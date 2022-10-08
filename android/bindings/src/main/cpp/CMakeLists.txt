cmake_minimum_required(VERSION 3.22)

set (PACKAGE_NAME "hlp_bindings")

set (RNLIBS_SO_DIR ${CMAKE_SOURCE_DIR}/../../../build/native_deps)
file (GLOB RNLIBS "${RNLIBS_SO_DIR}/jni/${ANDROID_ABI}")

set (LIBUDX_DIR "${ROOT_DIR}/prebuilt/android/udx/${ANDROID_ABI}")
set (LIBUV_DIR "${LIBUDX_DIR}/vendor/libuv")

project(${PACKAGE_NAME})

add_library(
        ${PACKAGE_NAME}
        SHARED
        native_install.cpp
        "${ROOT_DIR}/cpp/uv/uv.cpp"
        "${ROOT_DIR}/cpp/fs/fs.cpp"
)

include_directories(
        "${RNLIBS_SO_DIR}/include"
        "${LIBUV_DIR}/include"
        "${ROOT_DIR}/node_modules/react-native/ReactAndroid/src/main/java/com/facebook/react/turbomodule/core/jni"
        "${ROOT_DIR}/node_modules/react-native/ReactAndroid/src/main/jni/react/jni"
        "${ROOT_DIR}/node_modules/react-native/ReactCommon"
        "${ROOT_DIR}/node_modules/react-native/ReactCommon/jsi"
        "${ROOT_DIR}/node_modules/react-native/ReactCommon/callinvoker"
        "${ROOT_DIR}/node_modules/react-native/ReactCommon/runtimeexecutor"
        "${ROOT_DIR}/node_modules/react-native/ReactCommon/react/nativemodule/core"
)

target_include_directories(
        ${PACKAGE_NAME}
        PRIVATE
        ./
        "${ROOT_DIR}/cpp"
        "${ROOT_DIR}/cpp/uv"
        "${ROOT_DIR}/cpp/fs"
)

add_library(hlp_uv SHARED IMPORTED)
set_target_properties(
        hlp_uv
        PROPERTIES IMPORTED_LOCATION
        "${LIBUV_DIR}/libuv.so"
)

find_library(
        log-lib
        log
)
find_library(
        jsi-lib
        jsi
        PATHS ${RNLIBS}
        NO_CMAKE_FIND_ROOT_PATH
)
find_library(
        fbjni-lib
        fbjni
        PATHS ${RNLIBS}
        NO_CMAKE_FIND_ROOT_PATH
)
find_library(
        reactnativejni-lib
        reactnativejni
        PATHS ${RNLIBS}
        NO_CMAKE_FIND_ROOT_PATH
)

target_link_libraries(
        ${PACKAGE_NAME}
        ${log-lib}
        ${jsi-lib}
        ${fbjni-lib}
        ${reactnativejni-lib}
        hlp_uv
        android
)
