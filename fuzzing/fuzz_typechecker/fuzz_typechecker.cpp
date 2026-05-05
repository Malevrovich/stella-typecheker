// fuzzing/fuzz_typechecker/fuzz_typechecker.cpp
//
// Structure-aware fuzz test: proto AST  →  Stella source string  →  parse  →  typecheck.
//
// Property under test:
//   The type-checker must never crash (segfault, abort, std::terminate, or
//   unhandled exception) regardless of the semantic content of the program.
//
//   Semantic errors (TypeCheckError) are EXPECTED and are silently swallowed.
//   Only genuine internal failures — InternalTypeCheckError, NotSupportedError,
//   or any non-TypeCheckError exception — are treated as bugs.
//
// This mirrors the pipeline in typechecker/cli/src/main.cpp:
//   ParseProgramText(source)  →  CheckProgram(*ast)
//
// Build (unit-test mode, no sanitizers needed):
//   cmake -S . -B build -DFUZZ_TYPECHECKER=ON
//   cmake --build build --target fuzz_typechecker
//   ./build/fuzzing/fuzz_typechecker/fuzz_typechecker --gtest_filter=*
//
// Real fuzzing mode (clang + ASan + libFuzzer):
//   cmake -S . -B build -DFUZZ_TYPECHECKER=ON -DFUZZ_ENGINE=ON \
//         -DCMAKE_CXX_COMPILER=clang++ \
//         -DCMAKE_CXX_FLAGS="-fsanitize=address,fuzzer-no-link"
//   cmake --build build --target fuzz_typechecker
//   ./build/fuzzing/fuzz_typechecker/fuzz_typechecker \
//       --fuzz=StellaTypecheckerFuzz.TypecheckDoesNotCrash

#include <cstdlib>
#include <stdexcept>
#include <string>

// ── Google FuzzTest ───────────────────────────────────────────────────────────
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

// ── loguru — to install a fatal handler that prints the crashing source ───────
#include <loguru.hpp>

// ── Proto-generated AST + converter ──────────────────────────────────────────
#include "ast.pb.h"
#include "stella/fuzzing/proto_to_stella.h"

// ── Stella parser ─────────────────────────────────────────────────────────────
#include "stella/parser.hpp"

// ── Stella type-checker ───────────────────────────────────────────────────────
#include "stella/typecheck/check.hpp"
#include "stella/typecheck/error.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Thread-local storage for the Stella source currently under test.
// Updated at the start of each fuzz iteration (zero-copy: just a pointer).
// Read by the loguru fatal handler when loguru calls abort().
// ─────────────────────────────────────────────────────────────────────────────
namespace {

thread_local const std::string* g_current_source = nullptr;

// ── loguru fatal handler ──────────────────────────────────────────────────────
// Installed once in StellaFuzzEnvironment::SetUp().
// Called by loguru just before it calls abort() on FATAL log messages.
void FatalHandler(const loguru::Message& message) {
    std::fprintf(stderr, "\n[fuzz_typechecker] loguru FATAL: %s\n", message.message);
    if (g_current_source) {
        std::fprintf(stderr, "[fuzz_typechecker] Crashing Stella source:\n%s\n",
                     g_current_source->c_str());
    } else {
        std::fprintf(stderr,
                     "[fuzz_typechecker] (source not available — fatal outside fuzz loop)\n");
    }
    std::fflush(stderr);
    // loguru will call abort() after this handler returns.
}

// ── GTest environment — installs the fatal handler once per process ───────────
class StellaFuzzEnvironment : public ::testing::Environment {
public:
    void SetUp() override { loguru::set_fatal_handler(FatalHandler); }
};

// Registered via the global initialiser below.
const ::testing::Environment* const kEnv =
    ::testing::AddGlobalTestEnvironment(new StellaFuzzEnvironment());

// ─────────────────────────────────────────────────────────────────────────────
// Core property function
// ─────────────────────────────────────────────────────────────────────────────

void TypecheckDoesNotCrash(const stella::Program& proto) {
    // 1. Convert proto → Stella source string.
    const std::string source = stella::ToString(proto);

    // Publish the source pointer so the loguru fatal handler can print it
    // if abort() is called inside ParseProgramText or CheckProgram.
    g_current_source = &source;

    // 2. Parse.
    //    Some proto combinations produce fallback snippets that the parser
    //    cannot handle (e.g. unimplemented grammar constructs).  A parse
    //    failure is not a type-checker bug — skip silently.
    std::shared_ptr<const stella::ast::NodeProgram> program;
    try {
        program = stella::ParseProgramText(source);
    } catch (const std::exception&) {
        g_current_source = nullptr;
        return; // Not a type-checker bug.
    } catch (...) {
        g_current_source = nullptr;
        return; // Not a type-checker bug.
    }

    if (!program) {
        g_current_source = nullptr;
        return; // Not a type-checker bug.
    }

    // 3. Type-check.
    //    Expected outcome A: CheckProgram returns normally  → well-typed program.
    //    Expected outcome B: throws TypeCheckError          → semantic error, OK.
    //    Bug:               throws anything else            → internal crash.
    try {
        stella::typecheck::CheckProgram(*program);
        // Well-typed — nothing to do.
    } catch (const stella::typecheck::TypeCheckError&) {
        // Semantic error: expected for randomly generated programs.
        // Swallow silently — this is NOT a bug.
    } catch (const std::exception& ex) {
        std::fprintf(stderr,
                     "[fuzz_typechecker] UNEXPECTED exception from CheckProgram: %s\n"
                     "[fuzz_typechecker] Crashing Stella source:\n%s\n",
                     ex.what(), source.c_str());
        g_current_source = nullptr;
        std::abort();
    } catch (...) {
        std::fprintf(stderr,
                     "[fuzz_typechecker] UNKNOWN exception from CheckProgram\n"
                     "[fuzz_typechecker] Crashing Stella source:\n%s\n",
                     source.c_str());
        g_current_source = nullptr;
        std::abort();
    }

    g_current_source = nullptr;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// FuzzTest registration
//
// Domain: arbitrary proto Program with bounded repeated-field sizes.
// Depth is guarded in proto_to_stella.cpp (kMaxDepth=15).
// WithRepeatedFieldsMaxSize(3) keeps programs small enough for fast iteration.
// ─────────────────────────────────────────────────────────────────────────────
FUZZ_TEST(StellaTypecheckerFuzz, TypecheckDoesNotCrash)
    .WithDomains(fuzztest::Arbitrary<stella::Program>().WithRepeatedFieldsMaxSize(3));
