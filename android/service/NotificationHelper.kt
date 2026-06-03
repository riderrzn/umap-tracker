package com.example.umap.tracker.service

import android.app.Notification
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import androidx.core.app.NotificationCompat
import com.example.umap.tracker.MainActivity

object NotificationHelper {
    const val CHANNEL_ID = "umap_tracker_service"
    const val CHANNEL_NAME = "uMap Tracker"
    const val NOTIFICATION_ID = 1

    fun createChannel(context: Context) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            val channel = android.app.NotificationChannel(
                CHANNEL_ID,
                CHANNEL_NAME,
                android.app.NotificationManager.IMPORTANCE_LOW,
            ).apply {
                description = "GPS tracking active"
            }
            val manager = context.getSystemService(Service.NOTIFICATION_SERVICE)
                as android.app.NotificationManager
            manager.createNotificationChannel(channel)
        }
    }

    fun buildNotification(
        context: Context,
        speedKmh: Double,
        distanceKm: Double,
        isTracking: Boolean,
    ): Notification {
        val pendingIntent = PendingIntent.getActivity(
            context,
            0,
            Intent(context, MainActivity::class.java),
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE,
        )

        val status = if (isTracking) "Tracking" else "Paused"
        val content = "%.0f km/h  |  %.1f km".format(speedKmh, distanceKm)

        return NotificationCompat.Builder(context, CHANNEL_ID)
            .setContentTitle("uMap Tracker — $status")
            .setContentText(content)
            .setSmallIcon(android.R.drawable.ic_menu_compass)
            .setOngoing(true)
            .setContentIntent(pendingIntent)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .build()
    }
}