package com.example.umap.tracker.service

import android.app.Notification
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import com.example.umap.tracker.bridge.TrackerBridge

class TrackerForegroundService : Service() {

    private val handler = Handler(Looper.getMainLooper())
    private var isTracking = false
    private var speedKmh = 0.0
    private var distanceKm = 0.0

    private val processRunnable = object : Runnable {
        override fun run() {
            if (isTracking) {
                val processed = TrackerBridge.nativeProcessLocation()
                val uploaded = TrackerBridge.nativeProcessUpload()

                if (processed > 0) {
                    updateNotification()
                }

                handler.postDelayed(this, 1000L)
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        NotificationHelper.createChannel(this)
        TrackerBridge.nativeInitPipeline()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            ACTION_START -> start()
            ACTION_PAUSE -> pause()
            ACTION_RESUME -> resume()
            ACTION_STOP -> stop()
        }
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private fun start() {
        isTracking = true
        startForeground(NotificationHelper.NOTIFICATION_ID, buildNotification())
        handler.post(processRunnable)
    }

    private fun pause() {
        isTracking = false
        updateNotification()
    }

    private fun resume() {
        isTracking = true
        handler.post(processRunnable)
    }

    private fun stop() {
        isTracking = false
        handler.removeCallbacks(processRunnable)
        TrackerBridge.nativeShutdownPipeline()
        stopForeground(STOP_FOREGROUND_REMOVE)
        stopSelf()
    }

    private fun updateNotification() {
        val notification = buildNotification()
        val manager = getSystemService(Context.NOTIFICATION_SERVICE)
            as android.app.NotificationManager
        manager.notify(NotificationHelper.NOTIFICATION_ID, notification)
    }

    private fun buildNotification(): Notification {
        return NotificationHelper.buildNotification(this, speedKmh, distanceKm, isTracking)
    }

    override fun onDestroy() {
        handler.removeCallbacks(processRunnable)
        super.onDestroy()
    }

    companion object {
        const val ACTION_START = "com.example.umap.tracker.START"
        const val ACTION_PAUSE = "com.example.umap.tracker.PAUSE"
        const val ACTION_RESUME = "com.example.umap.tracker.RESUME"
        const val ACTION_STOP = "com.example.umap.tracker.STOP"
    }
}