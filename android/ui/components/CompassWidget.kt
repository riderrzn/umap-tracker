package com.example.umap.tracker.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.size
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.rotate
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.theme.Accent
import com.example.umap.tracker.ui.theme.TextPrimary
import com.example.umap.tracker.ui.theme.TextSecondary

@Composable
fun CompassWidget(
    bearingDeg: Double,
    modifier: Modifier = Modifier,
) {
    Box(
        modifier = modifier.size(80.dp),
        contentAlignment = Alignment.Center,
    ) {
        Canvas(modifier = Modifier.size(70.dp)) {
            val center = Offset(size.width / 2f, size.height / 2f)
            val radius = size.width / 2f - 4f

            drawCircle(color = Color.White.copy(alpha = 0.1f), radius = radius, center = center)

            rotate(-bearingDeg.toFloat() + 180f, center) {
                val needleLength = radius * 0.8f
                val needleWidth = 3f

                drawLine(
                    color = Accent,
                    start = Offset(center.x, center.y - needleLength),
                    end = Offset(center.x, center.y + needleLength * 0.3f),
                    strokeWidth = needleWidth,
                )

                drawCircle(
                    color = Color.White.copy(alpha = 0.3f),
                    radius = needleWidth * 1.5f,
                    center = Offset(center.x, center.y - needleLength),
                )
            }

            drawCircle(color = Color.White, radius = 3f, center = center)
        }

        Text(
            text = "${bearingDeg.toInt()}°",
            color = TextPrimary,
            style = MaterialTheme.typography.labelSmall,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.align(Alignment.BottomCenter),
        )
    }
}