package com.example.umap.tracker

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.example.umap.tracker.ui.navigation.BottomNavigationBar
import com.example.umap.tracker.ui.navigation.Screen
import com.example.umap.tracker.ui.screens.MapScreen
import com.example.umap.tracker.ui.screens.SettingsScreen
import com.example.umap.tracker.ui.screens.StatsScreen
import com.example.umap.tracker.ui.theme.UmapTrackerTheme
import com.example.umap.tracker.viewmodel.MapViewModel
import com.example.umap.tracker.viewmodel.SettingsViewModel

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            UmapTrackerTheme {
                UmapTrackerApp()
            }
        }
    }
}

@Composable
private fun UmapTrackerApp() {
    val navController = rememberNavController()
    val navBackStackEntry by navController.currentBackStackEntryAsState()
    val currentRoute = navBackStackEntry?.destination?.route ?: Screen.Map.route

    val mapViewModel: MapViewModel = viewModel()
    val settingsViewModel: SettingsViewModel = viewModel()

    Scaffold(
        bottomBar = {
            BottomNavigationBar(
                currentRoute = currentRoute,
                onNavigate = { screen ->
                    navController.navigate(screen.route) {
                        popUpTo(Screen.Map.route) { saveState = true }
                        launchSingleTop = true
                        restoreState = true
                    }
                },
            )
        },
    ) { innerPadding ->
        Box(modifier = Modifier.padding(innerPadding)) {
            NavHost(
                navController = navController,
                startDestination = Screen.Map.route,
            ) {
                composable(Screen.Map.route) {
                    MapScreen(viewModel = mapViewModel)
                }
                composable(Screen.Stats.route) {
                    StatsScreen(viewModel = mapViewModel)
                }
                composable(Screen.Settings.route) {
                    SettingsScreen(viewModel = settingsViewModel)
                }
            }
        }
    }
}