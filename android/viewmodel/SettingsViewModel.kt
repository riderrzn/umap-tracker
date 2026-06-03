package com.example.umap.tracker.viewmodel

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class SettingsUiState(
    val serverUrl: String = "http://map.bd62.ru:5055",
    val deviceId: String = "umap-tracker-001",
    val gpsIntervalSec: Int = 5,
    val tileSource: String = "liberty",
    val accuracyThresholdM: Double = 10.0,
    val uploadEnabled: Boolean = true,
    val batteryOptimization: Boolean = true,
)

class SettingsViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(SettingsUiState())
    val uiState: StateFlow<SettingsUiState> = _uiState.asStateFlow()

    fun setServerUrl(url: String) {
        _uiState.value = _uiState.value.copy(serverUrl = url)
    }

    fun setDeviceId(id: String) {
        _uiState.value = _uiState.value.copy(deviceId = id)
    }

    fun setGpsInterval(seconds: Int) {
        _uiState.value = _uiState.value.copy(gpsIntervalSec = seconds.coerceIn(1, 60))
    }

    fun setTileSource(source: String) {
        _uiState.value = _uiState.value.copy(tileSource = source)
    }

    fun setAccuracyThreshold(meters: Double) {
        _uiState.value = _uiState.value.copy(accuracyThresholdM = meters.coerceIn(1.0, 100.0))
    }

    fun setUploadEnabled(enabled: Boolean) {
        _uiState.value = _uiState.value.copy(uploadEnabled = enabled)
    }

    fun setBatteryOptimization(enabled: Boolean) {
        _uiState.value = _uiState.value.copy(batteryOptimization = enabled)
    }
}