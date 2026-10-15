# AGENTS.md

clox is a C++23 implementation of Lox, the bytecode virtual machine from Part III of
[Crafting Interpreters](https://craftinginterpreters.com/). It follows the book's design
but uses modern C++ in place of the book's C idioms.

## Commands

Windows, Clang via the `x64-debug-llvm` CMake preset. Dependencies come from vcpkg
(`vcpkg.json`). `sccache` must be on `PATH`.

```sh
just build                                   # configure + build (Ninja)
just run [script.lox]                        # run a script, default test.lox
ctest --test-dir out/build/x64-debug-llvm    # run all tests
just format                                  # clang-format all sources in place
just check                                   # format check + clang-tidy (warnings are errors)
just install-hooks                           # once per clone: run `just check` on pre-commit
```

`.clang-format` owns layout and include order. `.clang-tidy` enables `cppcoreguidelines-*`,
`modernize-*`, `performance-*` and enforces the naming rules below. `just check` must pass
before committing; CI only checks formatting.

## Layout

- `core/lox/syntax/`: scanner, tokens, source locations
- `core/lox/ast/`: parser that produces an AST
- `core/lox/vm/`: compiler (AST → bytecode `Chunk`), values, `VirtualMachine`
- `app/main.cpp`: the `lox` executable
- `tests/`: one executable per area, no test framework; `main` returns nonzero on failure

Sources are listed explicitly. Add new files to `core/CMakeLists.txt`, and register new
test executables with `add_test` in `tests/CMakeLists.txt`.

The compiler goes through an AST instead of the book's single-pass Pratt compiler. This
is deliberate; don't "fix" it to match the book.

## Priorities

1. **Performance first.** On the hot paths (the dispatch loop in `vm.cpp`, the value
   representation, the scanner) performance beats every other concern. Don't add
   allocations, virtual calls, or exceptions to them. Back performance claims with a
   measurement on the Release preset, not intuition.
2. **Readability everywhere else.** Outside the hot paths, prefer clear code over clever
   code, even if it takes more lines.
3. **Follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).**
   Use standard C++23 and the standard library before writing your own.

## Conventions

Match the existing code:

- `snake_case` functions and variables, `PascalCase` types, `m_` prefix for private members
- `enum class` with `snake_case` enumerators
- Trailing return types: `auto f() -> T`
- `[[nodiscard]]` on functions that return values, `noexcept` where it holds
- Errors are returned as `std::expected<T, std::string>`, not thrown
- Logging goes through `spdlog`
- Include guards are named `LOX_<DIR>_<FILE>_HPP`; namespaces mirror directories (`lox::vm`, ...)
- Include project headers (`"lox/..."`) first, then a blank line, then `<...>` headers

## Working rules

- If something is ambiguous (semantics, design, or scope), ask before you write code.
- Re-read a file before each edit; it may have changed since you last read it.
- When you deviate from the book, say so and explain why.
- Build and run the tests before you report a change as done. If you couldn't, say so.
- Be direct. No flattery, no filler.
