# Typechecker for Stella programming language

A type checker for the [Stella](https://fizruk.github.io/stella/) programming language, implemented in C++20.

## Requirements

### Local build
- CMake ≥ 3.14
- Clang 18 + libc++ 18 (`clang-18`, `libc++-18-dev`, `libc++abi-18-dev`)
- Java ≥ 11 (for ANTLR4 grammar generation)
- Internet access (CMake fetches `loguru`, `magic_enum`, `googletest`, `antlr4` automatically)

### Docker build
- Docker

---

## Building locally

```bash
# Install dependencies (Ubuntu 22.04)
wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
    | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" \
    | sudo tee /etc/apt/sources.list.d/llvm.list
sudo apt-get update
sudo apt-get install -y clang-18 libc++-18-dev libc++abi-18-dev cmake openjdk-17-jre-headless

# Configure and build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-libc++.cmake
cmake --build build -j$(nproc)
```

The binary will be at `build/typechecker/cli/typechecker`.

---

## Building with Docker

```bash
# Build the image
docker build -t stella-typechecker .

# Run the type checker on a Stella source file
docker run --rm -v /path/to/your/files:/data stella-typechecker /data/program.stella
```

---

## Usage

```
typechecker [OPTIONS] <input-file>

Options:
  -v, --verbose    Enable verbose debug output
```

### Examples

```bash
# Check a well-typed program
./build/typechecker/cli/typechecker stella-examples/ok/simple.stella
# Output: Well-typed

# Check a program with a type error
./build/typechecker/cli/typechecker stella-examples/error/test_err_missing_main.stella
# Output: ERROR_MISSING_MAIN: ...
```

---

## Running tests

```bash
# Unit tests (GoogleTest)
cd build
ctest --output-on-failure

# Golden tests (requires Python 3)
python3 tests/golden_tester.py \
    --binary build/typechecker/cli/typechecker \
    --golden-dir tests/golden \
    --examples-dir stella-examples
```

---

## Project structure

```
.
├── cmake/
│   ├── antlr4/                  # ANTLR4 CMake integration
│   └── toolchains/
│       └── clang-libc++.cmake   # Clang 18 + libc++ toolchain
├── stella-examples/             # Example Stella programs
│   ├── ok/                      # Well-typed programs
│   └── error/                   # Programs with type errors
├── tests/
│   ├── golden/                  # Expected outputs for golden tests
│   └── golden_tester.py         # Golden test runner
├── thirdparty/
│   └── antlr4/                  # ANTLR4 jar (grammar compiler)
├── typechecker/
│   ├── ast/                     # AST node definitions
│   ├── checker/                 # Type checking logic
│   ├── cli/                     # CLI entry point (main.cpp)
│   └── parser/                  # ANTLR4-based Stella parser
├── CMakeLists.txt
└── Dockerfile
```
