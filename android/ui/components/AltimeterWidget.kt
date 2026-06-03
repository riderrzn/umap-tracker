package com.example.umap.tracker.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.theme.Primary
import com.example.umap.tracker.ui.theme.TextPrimary
import com.example.umap.tracker.ui.theme.TextSecondary

@Composable
fun AltimeterWidget(
    altitudeM: Double,
    modifier: Modifier = Modifier,
) {
    Box(
        modifier = modifier
            .width(60.dp)
            .height(120.dp),
        contentAlignment = Alignment.Center,
    ) {
        Canvas(modifier = Modifier
            .width(8.dp)
            .height(100.dp)
        ) {
            drawRect(color = Color.White.copy(alpha = 0.1f))

            val fillRatio = ((altitudeM / 500.0).coerceIn(0.0, 1.0)).toFloat()
            drawRect(
                color = Primary,
                size = androidx.compose.ui.geometry.Size(
                    size.width,
                    size.height * fillRatio,
                ),
                topLeft = Offset(0f, size.height * (1f - fillRatio)),
            )
        }

        Text(
            text = "${altitudeM.toInt()}",
            color = TextPrimary,
            style = MaterialTheme.typography.bodyMedium,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.align(Alignment.Center),
        )

        Text(
            text = "m",
            color = TextSecondary,
            style = MaterialTheme.typography.labelSmall,
            modifier = Modifier.align(Alignment.BottomCenter),
        )
    }
}