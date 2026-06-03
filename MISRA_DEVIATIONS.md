# MISRA C++:2023 Deviations

**Document Version:** 2.0  
**Last Updated:** 2026-06-04  
**Project:** umap-tracker (Android GPS Tracker)

---

## Policy

This document records all deviations from MISRA C++:2023 compliance. Each deviation requires:

1. **Rule ID** — e.g., `R8-2-1` (MISRA C++:2023)
2. **Category** — Mandatory (M) / Required (R) / Advisory (A)
3. **Rationale** — Why the deviation is necessary
4. **Scope** — Which files/functions are affected
5. **Duration** — Temporary or permanent
6. **Reviewer** — Who approved the deviation

---

## Approved Deviations

### D001: Rule R8-2-1 (Explicit type conversion)

**Rule:** Avoid C-style casts; use `static_cast`, `dynamic_cast`, etc.

**Category:** Required

**Rationale:** JNI macros (`env->CallVoidMethod`, `env->GetFieldID`, etc.) use C-style casts internally. These are external to our code and cannot be changed.

**Scope:**
- `core/jni/jni_bridge.cpp`
- `core/src/platform/android_bridge.cpp`

**Exceptions:** JNI callback wrappers only

**Duration:** Permanent (inherent to JNI API)

**Approved by:** C++ Lead

---

### D002: Rule R8-2-2 (Avoid reinterpret_cast)

**Rule:** Avoid `reinterpret_cast`; use other conversion mechanisms.

**Category:** Required

**Rationale:** EGL/GLES rendering APIs (if used) require pointer reinterpretation for buffer handles and shader bytecode. No safer alternative exists in graphics APIs.

**Scope:**
- `core/jni/jni_bridge.cpp` (JNI_OnLoad GetEnv call)

**Exceptions:** JNI runtime initialization and graphics buffer handles

**Duration:** Permanent (inherent to JNI and OpenGL/EGL)

**Approved by:** C++ Lead

---

### D003: Rule M6-2-1 (Member initialization)

**Rule:** Aggregate classes must have in-class initializers for all members.

**Category:** Mandatory

**Rationale:** Some JNI structures cannot have in-class initializers due to Java calling conventions. These are wrapped in adapter classes with initializers.

**Scope:**
- `core/include/umap/platform/jni_types.h` (JNI callback signatures)

**Exceptions:** JNI callback struct fields only

**Duration:** Permanent (inherent to JNI callback calling conventions)

**Approved by:** C++ Lead

---

### D004: Macro usage in Google Test

**Rule:** R16-1-1 (Avoid preprocessor directives)

**Category:** Required

**Rationale:** Google Test framework uses `EXPECT_*` and `ASSERT_*` macros which may contain internal casts. These are test infrastructure, not production code.

**Scope:**
- `core/tests/*` (all test files)

**Exceptions:** Test assertions only

**Duration:** Permanent (test framework requirement)

**Approved by:** C++ Lead

---

### D005: Global variables in JNI bridge

**Rule:** M2-10-1 (Avoid global variables)

**Category:** Mandatory

**Rationale:** JNI bridge uses raw pointers as globals (`g_pipeline`, `g_message_queue`, etc.) to maintain pipeline state across JNI calls. Android JNI architecture requires static lifetime. These are not global variables in the classic sense — they represent singleton service objects allocated once in `nativeInitPipeline` and cleaned up in `nativeShutdownPipeline`.

**Scope:**
- `core/jni/jni_bridge.cpp` (anonymous namespace globals)

**Duration:** Permanent (Android JNI architecture constraint)

**Approved by:** C++ Lead

---

### D006: C-style format strings in snprintf

**Rule:** R21-1-1 (Avoid C library I/O functions)

**Category:** Required

**Rationale:** `std::snprintf` is used in `TraccarClient::build_json_payload()` and `GpsProcessingPipeline::process_location()` for JSON/URL construction. C++20 `std::format` would be MISRA-compliant but is not available with NDK r27+ and `-fno-exceptions -fno-rtti`. The usage is bounded (fixed buffer sizes) and checked (return value validated).

**Scope:**
- `core/src/data/traccar_client.cpp`
- `core/src/core/gps_pipeline.cpp`

**Duration:** Permanent (until C++20 std::format becomes available on Android NDK)

**Approved by:** C++ Lead

---

## Monitoring

All deviations are monitored via:

1. **clang-tidy** — with custom `.clang-tidy` config
2. **cppcheck MISRA addon** — weekly scans
3. **Code review** — each PR must reference this document
4. **CI/CD gate** — any unlisted deviation fails the build

---

## Change Log

| Date | Deviation | Action | Reviewer |
|------|-----------|--------|----------|
| 2026-06-03 | D001–D004 | Initial approval | C++ Lead |
| 2026-06-04 | D002 | Updated scope to include JNI reinterpret_cast | C++ Lead |
| 2026-06-04 | D003 | Changed to permanent (JNI bridge finalized) | C++ Lead |
| 2026-06-04 | D005 | Added: JNI globals for pipeline lifetime | C++ Lead |
| 2026-06-04 | D006 | Added: snprintf format string exception | C++ Lead |

---

## Future Reviews

- **Quarterly audit** of all active deviations
- **Removal of temporary deviations** once alternative approaches are found
- **Addition of new deviations** requires architecture review
