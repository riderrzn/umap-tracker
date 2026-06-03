package com.example.umap.tracker.ui.screens

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.umap.tracker.ui.components.AltimeterWidget
import com.example.umap.tracker.ui.components.CompassWidget
import com.example.umap.tracker.ui.components.GlassCard
import com.example.umap.tracker.ui.components.Speedometer
import com.example.umap.tracker.ui.theme.TextSecondary
import com.example.umap.tracker.viewmodel.MapViewModel

@Composable
fun StatsScreen(viewModel: MapViewModel) {
    val state by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
    ) {
        Text(
            text = "Trip Statistics",
            color = TextSecondary,
            style = MaterialTheme.typography.titleMedium,
            modifier = Modifier.padding(bottom = 16.dp),
        )

        GlassCard(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 12.dp),
            title = "Speed",
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Speedometer(
                    speedKmh = state.speedKmh,
                    modifier = Modifier.weight(1f),
                )
                Column(modifier = Modifier.weight(1f)) {
                    DetailRow("Max", "%.0f km/h".format(state.speedKmh))
                    DetailRow("Avg", "%.0f km/h".format(state.speedKmh * 0.8))
                    DetailRow("Accuracy", "%.1f m".format(state.accuracyM))
                }
            }
        }

        GlassCard(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 12.dp),
            title = "Navigation",
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                CompassWidget(
                    bearingDeg = state.bearingDeg,
                    modifier = Modifier.weight(1f),
                )
                AltimeterWidget(
                    altitudeM = state.altitudeM,
                    modifier = Modifier.weight(1f),
                )
            }
        }

        GlassCard(
            modifier = Modifier.fillMaxWidth(),
            title = "Trip",
        ) {
            Column {
                DetailRow("Distance", "%.1f km".format(state.distanceKm))
                DetailRow("Points", "${state.trackPoints.size}")
                DetailRow("Speed", "%.0f km/h".format(state.speedKmh))
            }
        }
    }
}

@Composable
private fun DetailRow(label: String, value: String) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
    ) {
        Text(
            text = label,
            color = TextSecondary,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.weight(1f),
        )
        Text(
            text = value,
            color = MaterialTheme.colorScheme.onSurface,
            style = MaterialTheme.typography.bodyMedium,
        )
    }
}