package com.example.umap.tracker.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.draw.shadow
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.theme.GlassBackground
import com.example.umap.tracker.ui.theme.GlassBorder
import com.example.umap.tracker.ui.theme.TextPrimary
import com.example.umap.tracker.ui.theme.TextSecondary

@Composable
fun GlassCard(
    modifier: Modifier = Modifier,
    title: String? = null,
    content: @Composable () -> Unit,
) {
    Box(
        modifier = modifier
            .shadow(8.dp, RoundedCornerShape(16.dp))
            .clip(RoundedCornerShape(16.dp))
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        GlassBackground,
                        GlassBackground.copy(alpha = 0.2f),
                    )
                )
            )
            .border(1.dp, GlassBorder, RoundedCornerShape(16.dp))
            .padding(16.dp)
    ) {
        if (title != null) {
            Text(
                text = title,
                color = TextSecondary,
                style = androidx.compose.material3.MaterialTheme.typography.labelMedium,
                modifier = Modifier.padding(bottom = 8.dp),
            )
        }
        content()
    }
}
