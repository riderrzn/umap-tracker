// Proguard rules for R8 minification

-keep class com.example.umap.tracker.** { *; }
-keep interface com.example.umap.tracker.** { *; }

# Preserve line numbers for crash reports
-keepattributes SourceFile,LineNumberTable
-renamesourcefileattribute SourceFile

# Jetpack Compose
-keep class androidx.compose.** { *; }

# MapLibre
-keep class com.mapbox.** { *; }

# JNI
-keepclasseswithmembernames class * {
    native <methods>;
}
