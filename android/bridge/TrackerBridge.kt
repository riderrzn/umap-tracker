package com.example.umap.tracker.bridge

object TrackerBridge {
    init {
        System.loadLibrary("umap-jni")
    }

    external fun nativeOnLocationReceived(
        latitude: Double,
        longitude: Double,
        accuracy: Float,
        altitude: Double,
        hdop: Double,
        timestampMs: Long,
    )

    external fun nativeOnBatteryChanged(
        batteryPercent: Int,
        isCharging: Boolean,
    )

    external fun nativeOnError(
        errorCode: Int,
        message: String,
    )

    external fun nativeInitPipeline()

    external fun nativeProcessLocation(): Int

    external fun nativeProcessUpload(): Int

    external fun nativeGetQueueSize(): Int

    external fun nativeShutdownPipeline()
}