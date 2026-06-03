package com.example.umap.tracker.viewmodel

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class MapUiState(
    val latitude: Double = 0.0,
    val longitude: Double = 0.0,
    val speedKmh: Double = 0.0,
    val bearingDeg: Double = 0.0,
    val altitudeM: Double = 0.0,
    val accuracyM: Double = 0.0,
    val distanceKm: Double = 0.0,
    val durationSec: Long = 0L,
    val isTracking: Boolean = false,
    val isGpsAcquiring: Boolean = false,
    val tileSource: String = TILE_SOURCE_LIBERTY,
    val trackPoints: List<Pair<Double, Double>> = emptyList(),
) {
    companion object {
        const val TILE_SOURCE_LIBERTY = "liberty"
        const val TILE_SOURCE_OSM = "osm"
    }
}

class MapViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(MapUiState())
    val uiState: StateFlow<MapUiState> = _uiState.asStateFlow()

    fun updateLocation(
        lat: Double,
        lon: Double,
        speed: Double,
        bearing: Double,
        altitude: Double,
        accuracy: Double,
    ) {
        _uiState.value = _uiState.value.copy(
            latitude = lat,
            longitude = lon,
            speedKmh = speed,
            bearingDeg = bearing,
            altitudeM = altitude,
            accuracyM = accuracy,
            isGpsAcquiring = false,
        )

        if (_uiState.value.isTracking) {
            val newPoints = _uiState.value.trackPoints + Pair(lat, lon)
            _uiState.value = _uiState.value.copy(trackPoints = newPoints)
        }
    }

    fun startTracking() {
        _uiState.value = _uiState.value.copy(
            isTracking = true,
            isGpsAcquiring = true,
            distanceKm = 0.0,
            durationSec = 0L,
            trackPoints = emptyList(),
        )
    }

    fun stopTracking() {
        _uiState.value = _uiState.value.copy(
            isTracking = false,
            isGpsAcquiring = false,
        )
    }

    fun pauseTracking() {
        _uiState.value = _uiState.value.copy(isTracking = false)
    }

    fun resumeTracking() {
        _uiState.value = _uiState.value.copy(isTracking = true)
    }

    fun updateDistanceAndTime(distanceKm: Double, durationSec: Long) {
        _uiState.value = _uiState.value.copy(
            distanceKm = distanceKm,
            durationSec = durationSec,
        )
    }

    fun setTileSource(source: String) {
        _uiState.value = _uiState.value.copy(tileSource = source)
    }
}