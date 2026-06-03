package com.example.umap.tracker.ui.navigation

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Map
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.filled.Timeline
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.graphics.vector.ImageVector

enum class Screen(val route: String, val label: String, val icon: ImageVector) {
    Map("map", "Map", Icons.Default.Map),
    Stats("stats", "Stats", Icons.Default.Timeline),
    Settings("settings", "Settings", Icons.Default.Settings),
}

@Composable
fun BottomNavigationBar(
    currentRoute: String,
    onNavigate: (Screen) -> Unit,
) {
    NavigationBar {
        Screen.entries.forEach { screen ->
            NavigationBarItem(
                selected = currentRoute == screen.route,
                onClick = { onNavigate(screen) },
                icon = {
                    Icon(imageVector = screen.icon, contentDescription = screen.label)
                },
                label = {
                    Text(text = screen.label)
                },
            )
        }
    }
}