package com.example.umap.tracker.viewmodel

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class StatsUiState(
    val totalDistanceKm: Double = 0.0,
    val totalDurationSec: Long = 0L,
    val avgSpeedKmh: Double = 0.0,
    val maxSpeedKmh: Double = 0.0,
    val totalPoints: Int = 0,
    val sessions: List<TrackingSession> = emptyList(),
)

data class TrackingSession(
    val id: Long,
    val startTime: Long,
    val endTime: Long,
    val distanceKm: Double,
    val avgSpeedKmh: Double,
    val maxSpeedKmh: Double,
)

class StatsViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(StatsUiState())
    val uiState: StateFlow<StatsUiState> = _uiState.asStateFlow()

    fun updateStats(
        totalDistanceKm: Double,
        totalDurationSec: Long,
        avgSpeedKmh: Double,
        maxSpeedKmh: Double,
        totalPoints: Int,
    ) {
        _uiState.value = _uiState.value.copy(
            totalDistanceKm = totalDistanceKm,
            totalDurationSec = totalDurationSec,
            avgSpeedKmh = avgSpeedKmh,
            maxSpeedKmh = maxSpeedKmh,
            totalPoints = totalPoints,
        )
    }

    fun loadSessions(sessions: List<TrackingSession>) {
        _uiState.value = _uiState.value.copy(sessions = sessions)
    }
}