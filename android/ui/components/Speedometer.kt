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
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.theme.SpeedColor
import com.example.umap.tracker.ui.theme.TextPrimary
import com.example.umap.tracker.ui.theme.TextSecondary

@Composable
fun Speedometer(
    speedKmh: Double,
    modifier: Modifier = Modifier,
    size: Dp = 140.dp,
    maxSpeed: Double = 200.0,
) {
    Box(
        modifier = modifier.size(size),
        contentAlignment = Alignment.Center,
    ) {
        Canvas(modifier = Modifier.size(size)) {
            val strokeWidth = size.toPx() * 0.12f
            val arcSize = Size(size.toPx() - strokeWidth, size.toPx() - strokeWidth)
            val topLeft = Offset(strokeWidth / 2f, strokeWidth / 2f)

            drawArc(
                color = Color.White.copy(alpha = 0.1f),
                startAngle = 135f,
                sweepAngle = 270f,
                useCenter = false,
                topLeft = topLeft,
                size = arcSize,
                style = Stroke(width = strokeWidth, cap = StrokeCap.Round),
            )

            val sweep = ((speedKmh / maxSpeed) * 270f).coerceIn(0f, 270f)
            drawArc(
                color = SpeedColor,
                startAngle = 135f,
                sweepAngle = sweep,
                useCenter = false,
                topLeft = topLeft,
                size = arcSize,
                style = Stroke(width = strokeWidth, cap = StrokeCap.Round),
            )
        }

        Text(
            text = "${speedKmh.toInt()}",
            color = TextPrimary,
            style = MaterialTheme.typography.headlineLarge,
            fontWeight = FontWeight.Bold,
        )

        Text(
            text = "km/h",
            color = TextSecondary,
            style = MaterialTheme.typography.labelSmall,
            modifier = Modifier.align(Alignment.BottomCenter),
        )
    }
}