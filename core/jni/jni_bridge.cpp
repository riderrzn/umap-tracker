// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// JNI Bridge: connects Kotlin/Java to C++ core via LocationEngine

#ifdef __ANDROID__

#include <jni.h>
#include <umap/platform/android_bridge.h>
#include <umap/utils/logger.h>

namespace {

constexpr const char* TAG = "umap-jni";

}  // namespace

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    if (vm == nullptr) {
        return JNI_ERR;
    }

    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    LOG_TAG_INFO(TAG, "JNI bridge loaded");

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeOnLocationReceived(
    JNIEnv* /* env */, jobject /* thiz */,
    jdouble latitude, jdouble longitude,
    jfloat accuracy, jlong timestamp_ms) {

    umap::platform::LocationEngine::on_location_received(
        static_cast<double>(latitude),
        static_cast<double>(longitude),
        static_cast<float>(accuracy),
        static_cast<uint32_t>(timestamp_ms));
}

JNIEXPORT void JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeOnBatteryChanged(
    JNIEnv* /* env */, jobject /* thiz */,
    jint battery_percent, jboolean is_charging) {

    umap::platform::LocationEngine::on_battery_changed(
        static_cast<uint32_t>(battery_percent),
        static_cast<bool>(is_charging));
}

JNIEXPORT void JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeOnError(
    JNIEnv* env, jobject /* thiz */,
    jint error_code, jstring message) {

    if (env == nullptr) {
        return;
    }

    const char* msg_str = nullptr;
    if (message != nullptr) {
        msg_str = env->GetStringUTFChars(message, nullptr);
    }

    umap::platform::LocationEngine::on_error_occurred(
        static_cast<int>(error_code), msg_str);

    if ((message != nullptr) && (msg_str != nullptr)) {
        env->ReleaseStringUTFChars(message, msg_str);
    }
}

#endif  // __ANDROID__
