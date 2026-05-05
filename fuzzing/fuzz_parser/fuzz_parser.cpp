// fuzzing/fuzz_parser/fuzz_parser.cpp
//
// Structure-aware fuzz test: proto AST  →  Stella source string  →  ANTLR parse.
//
// Property under test:
//   Every string produced by stella::ToString(Program) must be accepted by the
//   Stella parser without any lexer or syntax error.
//
// If the parser reports an error the test calls std::abort() so that the fuzzer
// records the input as a crash / failing corpus entry.

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

// ── Google FuzzTest ───────────────────────────────────────────────────────────
#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

// ── Proto-generated AST + converter ──────────────────────────────────────────
#include "ast.pb.h"
#include "stella/fuzzing/proto_to_stella.h"

// ── ANTLR4 runtime ───────────────────────────────────────────────────────────
#include <ANTLRInputStream.h>
#include <CommonTokenStream.h>
#include <antlr4-runtime.h>

// ── Generated Stella lexer / parser ──────────────────────────────────────────
#include "StellaLexer.h"
#include "StellaParser.h"

// ─────────────────────────────────────────────────────────────────────────────
// Strict error listener: any lexer or parser error → std::abort()
// ─────────────────────────────────────────────────────────────────────────────
namespace {

class AbortOnErrorListener : public antlr4::BaseErrorListener {
public:
    void syntaxError(antlr4::Recognizer* /*recognizer*/, antlr4::Token* /*offendingSymbol*/,
                     size_t line, size_t charPositionInLine, const std::string& msg,
                     std::exception_ptr /*e*/) override {
        // Print the offending source so the fuzzer corpus entry is readable.
        std::fprintf(stderr,
                     "[fuzz_parser] Syntax error at %zu:%zu — %s\n"
                     "[fuzz_parser] Source:\n%s\n",
                     line, charPositionInLine, msg.c_str(), current_source_.c_str());
        std::abort();
    }

    // Call this before each parse so the error message includes the source.
    void SetSource(const std::string& src) { current_source_ = src; }

private:
    std::string current_source_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Core property function
// ─────────────────────────────────────────────────────────────────────────────
void ParseRoundTrip(const stella::Program& proto) {
    // 1. Convert proto → Stella source string.
    const std::string source = stella::ToString(proto);

    // 2. Feed into ANTLR.
    antlr4::ANTLRInputStream input(source);

    antlr4_stella::StellaLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    antlr4_stella::StellaParser parser(&tokens);

    // 3. Remove the default console error listeners and install our strict one.
    AbortOnErrorListener strict_listener;
    strict_listener.SetSource(source);

    lexer.removeErrorListeners();
    lexer.addErrorListener(&strict_listener);

    parser.removeErrorListeners();
    parser.addErrorListener(&strict_listener);

    // 4. Parse.  Any error triggers abort() inside the listener.
    //    Wrap in try/catch so that exceptions from the ANTLR runtime itself
    //    (e.g. LexerNoViableAltException) are also caught and turned into
    //    test failures rather than silent exits.
    try {
        parser.start_Program();
    } catch (const std::exception& ex) {
        std::fprintf(stderr,
                     "[fuzz_parser] Exception during parse: %s\n"
                     "[fuzz_parser] Source:\n%s\n",
                     ex.what(), source.c_str());
        std::abort();
    }
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// FuzzTest registration
//
// Domain: arbitrary proto Program with bounded repeated-field sizes.
// Depth is already guarded in proto_to_stella.cpp (kMaxDepth=15): any expr /
// type / pattern that would exceed the limit is replaced by a safe terminal
// ("0", "Nat", "v0"), so the generated source is always syntactically valid.
//
// WithRepeatedFieldsMaxSize(3) keeps the generated programs small enough that
// the fuzzer can explore many distinct structures per second.
// ─────────────────────────────────────────────────────────────────────────────
FUZZ_TEST(StellaParserFuzz, ParseRoundTrip)
    .WithDomains(fuzztest::Arbitrary<stella::Program>().WithRepeatedFieldsMaxSize(3));
