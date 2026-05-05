# ─── fuzzing/CMakeLists.txt ───────────────────────────────────────────────────
#
# Top-level entry point for all fuzzing-related targets.
#
# Add new fuzz targets here as additional add_subdirectory() calls.
# ─────────────────────────────────────────────────────────────────────────────

# Shared library: proto AST messages + Stella source serialiser.
add_subdirectory(proto_codegen)

# Structure-aware parser fuzz target (enabled with -DFUZZ_PARSER=ON).
add_subdirectory(fuzz_parser)

# Structure-aware type-checker fuzz target (enabled with -DFUZZ_TYPECHECKER=ON).
add_subdirectory(fuzz_typechecker)
