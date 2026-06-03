// Inclusion guard: only compile for Android
#ifdef __ANDROID__

#include <jni.h>
#include <umap/platform/android_bridge.h>
#include <umap/core/gps_pipeline.h>
#include <umap/data/message_queue.h>
#include <umap/data/traccar_client.h>
#include <umap/data/upload_worker.h>
#include <umap/data/http_client.h>
#include <umap/utils/logger.h>

namespace {

constexpr const char* TAG = "umap-jni";

umap::core::GpsProcessingPipeline* g_pipeline = nullptr;
umap::data::MessageQueue* g_message_queue = nullptr;
umap::data::UploadWorker* g_upload_worker = nullptr;
umap::data::MockHttpClient* g_mock_http = nullptr;
umap::data::TraccarClient* g_traccar_client = nullptr;

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
    jfloat accuracy, jdouble altitude,
    jdouble hdop, jlong timestamp_ms) {

    umap::platform::LocationEngine::on_location_received(
        static_cast<double>(latitude),
        static_cast<double>(longitude),
        static_cast<float>(accuracy),
        static_cast<double>(altitude),
        static_cast<double>(hdop),
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

JNIEXPORT void JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeInitPipeline(
    JNIEnv* /* env */, jobject /* thiz */) {

    if (g_message_queue != nullptr) {
        return;
    }

    g_message_queue = new umap::data::MessageQueue(":memory:");
    g_pipeline = new umap::core::GpsProcessingPipeline(*g_message_queue);

    g_mock_http = new umap::data::MockHttpClient();
    g_mock_http->set_response(200, "{}");

    g_traccar_client = new umap::data::TraccarClient(
        *g_mock_http, "http://map.bd62.ru:5055", "umap-tracker-001");

    g_upload_worker = new umap::data::UploadWorker(
        *g_message_queue, *g_traccar_client);

    LOG_TAG_INFO(TAG, "Pipeline initialized");
}

JNIEXPORT jint JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeProcessLocation(
    JNIEnv* /* env */, jobject /* thiz */) {

    if (g_pipeline == nullptr) {
        return 0;
    }

    auto raw = umap::platform::LocationEngine::instance().get_location(0U);
    if (!raw.has_value()) {
        return 0;
    }

    auto result = g_pipeline->process_location(raw.value());
    return result.has_value() ? 1 : 0;
}

JNIEXPORT jint JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeProcessUpload(
    JNIEnv* /* env */, jobject /* thiz */) {

    if (g_upload_worker == nullptr) {
        return 0;
    }

    return static_cast<jint>(g_upload_worker->process_pending(5U));
}

JNIEXPORT jint JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeGetQueueSize(
    JNIEnv* /* env */, jobject /* thiz */) {

    if (g_message_queue == nullptr) {
        return 0;
    }

    return static_cast<jint>(g_message_queue->pending_count());
}

JNIEXPORT void JNICALL
Java_com_example_umap_tracker_TrackerBridge_nativeShutdownPipeline(
    JNIEnv* /* env */, jobject /* thiz */) {

    delete g_upload_worker;
    g_upload_worker = nullptr;

    delete g_traccar_client;
    g_traccar_client = nullptr;

    delete g_mock_http;
    g_mock_http = nullptr;

    delete g_pipeline;
    g_pipeline = nullptr;

    delete g_message_queue;
    g_message_queue = nullptr;

    LOG_TAG_INFO(TAG, "Pipeline shutdown");
}

#endif  // __ANDROID__
