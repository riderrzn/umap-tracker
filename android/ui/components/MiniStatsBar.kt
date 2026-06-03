package com.example.umap.tracker.ui.components

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.MyLocation
import androidx.compose.material.icons.filled.Timer
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import com.example.umap.tracker.ui.theme.Accent
import com.example.umap.tracker.ui.theme.SpeedColor
import com.example.umap.tracker.ui.theme.TextPrimary
import com.example.umap.tracker.ui.theme.TextSecondary

@Composable
fun MiniStatsBar(
    distanceKm: Double,
    durationSec: Long,
    isTracking: Boolean,
    modifier: Modifier = Modifier,
) {
    Row(
        modifier = modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically,
    ) {
        StatItem(
            icon = Icons.Default.MyLocation,
            value = if (distanceKm >= 1.0) "%.1f km".format(distanceKm) else "${(distanceKm * 1000).toInt()} m",
            isActive = isTracking,
        )

        StatItem(
            icon = Icons.Default.Timer,
            value = formatDuration(durationSec),
            isActive = isTracking,
        )
    }
}

private fun formatDuration(seconds: Long): String {
    val h = seconds / 3600
    val m = (seconds % 3600) / 60
    val s = seconds % 60
    return if (h > 0) "%d:%02d:%02d".format(h, m, s) else "%02d:%02d".format(m, s)
}

@Composable
private fun StatItem(
    icon: ImageVector,
    value: String,
    isActive: Boolean,
) {
    Row(
        verticalAlignment = Alignment.CenterVertically,
        modifier = Modifier
            .weight(1f)
    ) {
        Icon(
            imageVector = icon,
            contentDescription = null,
            tint = if (isActive) SpeedColor else TextSecondary,
            modifier = Modifier.padding(end = 6.dp),
        )
        Text(
            text = value,
            color = if (isActive) TextPrimary else TextSecondary,
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
        )
    }
}