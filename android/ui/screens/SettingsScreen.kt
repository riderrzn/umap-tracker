package com.example.umap.tracker.ui.screens

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.components.GlassCard
import com.example.umap.tracker.ui.theme.TextSecondary
import com.example.umap.tracker.viewmodel.SettingsViewModel

@Composable
fun SettingsScreen(viewModel: SettingsViewModel) {
    val state by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
            .verticalScroll(rememberScrollState()),
    ) {
        Text(
            text = "Settings",
            color = TextSecondary,
            style = MaterialTheme.typography.titleMedium,
            modifier = Modifier.padding(bottom = 16.dp),
        )

        GlassCard(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 12.dp),
            title = "Server",
        ) {
            OutlinedTextField(
                value = state.serverUrl,
                onValueChange = { viewModel.setServerUrl(it) },
                label = { Text("Server URL") },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
            )
            Spacer(modifier = Modifier.height(8.dp))
            OutlinedTextField(
                value = state.deviceId,
                onValueChange = { viewModel.setDeviceId(it) },
                label = { Text("Device ID") },
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
            )
        }

        GlassCard(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 12.dp),
            title = "Map Style",
        ) {
            Column(modifier = Modifier.selectableGroup()) {
                listOf(
                    "liberty" to "OpenFreeMap Liberty",
                    "osm" to "OpenStreetMap",
                ).forEach { (value, label) ->
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .selectable(
                                selected = state.tileSource == value,
                                onClick = { viewModel.setTileSource(value) },
                                role = Role.RadioButton,
                            )
                            .padding(vertical = 4.dp),
                        verticalAlignment = Alignment.CenterVertically,
                    ) {
                        RadioButton(
                            selected = state.tileSource == value,
                            onClick = null,
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text(
                            text = label,
                            color = MaterialTheme.colorScheme.onSurface,
                            style = MaterialTheme.typography.bodyMedium,
                        )
                    }
                }
            }
        }

        GlassCard(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 12.dp),
            title = "GPS",
        ) {
            Text(
                text = "Interval: ${state.gpsIntervalSec}s",
                color = MaterialTheme.colorScheme.onSurface,
                style = MaterialTheme.typography.bodyMedium,
            )
            Slider(
                value = state.gpsIntervalSec.toFloat(),
                onValueChange = { viewModel.setGpsInterval(it.toInt()) },
                valueRange = 1f..60f,
                steps = 59,
                modifier = Modifier.fillMaxWidth(),
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                text = "Accuracy threshold: ${state.accuracyThresholdM.toInt()}m",
                color = MaterialTheme.colorScheme.onSurface,
                style = MaterialTheme.typography.bodyMedium,
            )
            Slider(
                value = state.accuracyThresholdM.toFloat(),
                onValueChange = { viewModel.setAccuracyThreshold(it.toDouble()) },
                valueRange = 1f..100f,
                modifier = Modifier.fillMaxWidth(),
            )
        }

        GlassCard(
            modifier = Modifier.fillMaxWidth(),
            title = "Upload",
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(
                    text = "Upload enabled",
                    color = MaterialTheme.colorScheme.onSurface,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.weight(1f),
                )
                Switch(
                    checked = state.uploadEnabled,
                    onCheckedChange = { viewModel.setUploadEnabled(it) },
                )
            }
            Spacer(modifier = Modifier.height(8.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(
                    text = "Battery optimization",
                    color = MaterialTheme.colorScheme.onSurface,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.weight(1f),
                )
                Switch(
                    checked = state.batteryOptimization,
                    onCheckedChange = { viewModel.setBatteryOptimization(it) },
                )
            }
        }
    }
}