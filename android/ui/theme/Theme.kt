package com.example.umap.tracker.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable

private val UmapColorScheme = darkColorScheme(
    primary = Primary,
    secondary = Accent,
    background = BackgroundDark,
    surface = SurfaceDark,
    onPrimary = TextPrimary,
    onSecondary = TextPrimary,
    onBackground = TextPrimary,
    onSurface = TextPrimary,
    error = ErrorColor,
)

@Composable
fun UmapTrackerTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = UmapColorScheme,
        content = content,
    )
}
