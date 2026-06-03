package com.example.umap.tracker.ui.screens

import android.view.ViewGroup
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import com.example.umap.tracker.ui.components.GlassCard
import com.example.umap.tracker.ui.components.MiniStatsBar
import com.example.umap.tracker.ui.theme.Accent
import com.example.umap.tracker.ui.theme.ErrorColor
import com.example.umap.tracker.ui.theme.SpeedColor
import com.example.umap.tracker.ui.theme.TextSecondary
import com.example.umap.tracker.viewmodel.MapUiState
import com.example.umap.tracker.viewmodel.MapViewModel
import com.mapbox.mapboxsdk.Mapbox
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.location.LocationComponentOptions
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.Style

private const val TILE_URL_OSM = "https://tile.openstreetmap.org/{z}/{x}/{y}.png"
private const val TILE_URL_LIBERTY = "https://tiles.openfreemap.org/styles/liberty/{z}/{x}/{y}.png"

@Composable
fun MapScreen(viewModel: MapViewModel) {
    val state by viewModel.uiState.collectAsState()
    val context = LocalContext.current

    Box(modifier = Modifier.fillMaxSize()) {
        MapViewComposable(
            state = state,
            modifier = Modifier.fillMaxSize(),
        )

        Column(
            modifier = Modifier
                .align(Alignment.TopCenter)
                .padding(16.dp)
                .fillMaxWidth(),
        ) {
            GlassCard(modifier = Modifier.fillMaxWidth()) {
                MiniStatsBar(
                    distanceKm = state.distanceKm,
                    durationSec = state.durationSec,
                    isTracking = state.isTracking,
                )
            }
        }

        Row(
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(16.dp),
        ) {
            if (!state.isTracking) {
                FloatingActionButton(
                    onClick = { viewModel.startTracking() },
                    containerColor = SpeedColor,
                    modifier = Modifier.padding(end = 12.dp),
                ) {
                    Icon(Icons.Default.PlayArrow, contentDescription = "Start", tint = androidx.compose.ui.graphics.Color.White)
                }
            } else {
                FloatingActionButton(
                    onClick = { viewModel.pauseTracking() },
                    containerColor = Accent,
                    modifier = Modifier.padding(end = 12.dp),
                ) {
                    Icon(Icons.Default.Pause, contentDescription = "Pause", tint = androidx.compose.ui.graphics.Color.White)
                }
                FloatingActionButton(
                    onClick = { viewModel.stopTracking() },
                    containerColor = ErrorColor,
                ) {
                    Icon(Icons.Default.Stop, contentDescription = "Stop", tint = androidx.compose.ui.graphics.Color.White)
                }
            }
        }

        if (state.isGpsAcquiring) {
            Text(
                text = "Acquiring GPS...",
                color = TextSecondary,
                style = MaterialTheme.typography.labelMedium,
                modifier = Modifier
                    .align(Alignment.Center)
                    .padding(16.dp),
            )
        }
    }
}

@Composable
private fun MapViewComposable(
    state: MapUiState,
    modifier: Modifier = Modifier,
) {
    val context = LocalContext.current
    Mapbox.getInstance(context, null)

    var mapView by remember { mutableStateOf<MapView?>(null) }
    var mapboxMap by remember { mutableStateOf<MapboxMap?>(null) }

    AndroidView(
        factory = { ctx ->
            MapView(ctx).apply {
                layoutParams = ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT,
                )
                onCreate(null)

                getMapAsync { map ->
                    mapboxMap = map

                    val styleUri = when (state.tileSource) {
                        MapUiState.TILE_SOURCE_LIBERTY -> Style.Builder().fromUrl(TILE_URL_LIBERTY)
                        else -> Style.Builder().fromUrl(TILE_URL_OSM)
                    }

                    map.setStyle(styleUri) {
                        if (state.latitude != 0.0) {
                            map.animateCamera(
                                CameraUpdateFactory.newCameraPosition(
                                    CameraPosition.Builder()
                                        .target(LatLng(state.latitude, state.longitude))
                                        .zoom(15.0)
                                        .build()
                                )
                            )
                        }
                    }
                }
            }.also { mapView = it }
        },
        modifier = modifier,
    )

    DisposableEffect(Unit) {
        onDispose {
            mapView?.onDestroy()
        }
    }
}