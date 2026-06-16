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

## Building with VSCode (CMake Tools extension)

The project includes `.vscode/settings.json` that configures the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension automatically:

- Toolchain: `cmake/toolchains/clang-libc++.cmake` (clang-18 + libc++)
- Build directory: `build/`
- Parallel jobs: auto-detected (all available CPU cores)

Just open the project folder in VSCode, install the **CMake Tools** extension, and click **Build** (or press `F7`). No additional configuration needed.

---

## Building with Docker

```bash
# Build the image
docker build -t stella-typechecker .

# Run the type checker on a Stella source file (reads from stdin)
docker run --rm -v /path/to/your/files:/data stella-typechecker < /data/program.stella

# Or using cat
docker run --rm -v /path/to/your/files:/data stella-typechecker -i /data/program.stella
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
# Check a well-typed program with sum types
cat stella-examples/sum-types/ok/test_ok_sum_basic.stella | ./build/typechecker/cli/typechecker
# Output: Well-typed

# Check a program with type reconstruction
cat stella-examples/type-reconstruction/ok/test_ok_identity.stella | ./build/typechecker/cli/typechecker
# Output: Well-typed

# Check a program with a type error
cat stella-examples/sum-types/error/test_err_nonexhaustive_match_inl_only.stella | ./build/typechecker/cli/typechecker
# Output: ERROR_NONEXHAUSTIVE_MATCH: ...

# Or use input redirection
./build/typechecker/cli/typechecker < stella-examples/sum-types/ok/test_ok_sum_basic.stella
# Output: Well-typed
```

### Example Stella Programs

#### Sum Types
```stella
language core;
extend with #sum-types;

fn main(n : Nat) -> Nat + Bool {
  return inl(n)
}
```

#### Type Reconstruction
```stella
language core;
extend with #type-reconstruction;

fn id(x : auto) -> auto {
  return x
}

fn main(n : auto) -> auto {
  return id(n)
}
```

#### Variants with Nested Matching
```stella
language core;
extend with #variants, #natural-literals, #integers;

fn main(x : <| outer : <| a : Nat, b : Bool |>, other : Nat |>) -> Nat {
  return match x {
      <| outer = inner |> => match inner {
          <| a = n |> => n
        | <| b = f |> => if f then 1 else 0
      }
    | <| other = n |> => succ(n)
  }
}
```

#### Sequencing
```stella
language core;
extend with #unit-type, #sequencing;

fn main(x : Nat) -> Nat {
  return (unit ; x)
}
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
├── stella-examples/             # Example Stella programs organized by feature
│   ├── sum-types/               # Sum type examples (ok/ and error/)
│   ├── variants/                # Variant type examples
│   ├── nested-match/            # Nested pattern matching examples
│   ├── sequencing/              # Sequencing examples
│   ├── type-reconstruction/     # Type inference examples
│   └── type-ascriptions/        # Type annotation examples
├── tests/
│   ├── golden/                  # Expected outputs for golden tests
│   ├── golden_tester.py         # Golden test runner
│   └── verify_golden.py         # Golden test verification
├── thirdparty/
│   └── antlr4/                  # ANTLR4 jar (grammar compiler)
├── typechecker/
│   ├── ast/                     # AST node definitions and visitors
│   │   ├── include/stella/ast/  # AST header files
│   │   └── src/                 # AST implementation
│   ├── checker/                 # Type checking logic
│   │   ├── include/stella/typecheck/
│   │   └── src/                 # Type checker implementation
│   ├── cli/                     # CLI entry point (main.cpp)
│   └── parser/                  # ANTLR4-based Stella parser
├── CMakeLists.txt
├── Dockerfile
└── README.md
```

---

## Architecture

### Type Checking Pipeline

1. **Parsing**: ANTLR4-based parser converts Stella source code into an AST
2. **AST Construction**: Visitor pattern builds strongly-typed AST nodes
3. **Type Checking**: 
   - Name resolution and context management
   - Type reconstruction with unification
   - Subtype checking
   - Pattern matching exhaustiveness
4. **Error Reporting**: Detailed error messages with source locations

### Key Components

- **AST Nodes**: Defined in [`typechecker/ast/include/stella/ast/`](typechecker/ast/include/stella/ast/) with visitor pattern support
- **Type Checker**: Core logic in [`typechecker/checker/src/`](typechecker/checker/src/) with modular handlers for each language construct
- **Unifier**: Implementation in [`typechecker/checker/src/unifier.cpp`](typechecker/checker/src/unifier.cpp)
- **Error Handling**: Comprehensive error types in [`typechecker/checker/include/stella/typecheck/error.hpp`](typechecker/checker/include/stella/typecheck/error.hpp)

---

## License

See [LICENSE](LICENSE) file for details.
