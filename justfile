set windows-shell := ["powershell"]

alias b := build
alias r := run
alias c := clean

build:
    @cmake --preset x64-debug-llvm
    @ninja -C out/build/x64-debug-llvm

run script="test.lox":
    @out/build/x64-debug-llvm/lox.exe {{ script }}

clean:
    @cmake --build out/build/x64-debug-llvm --target clean

format:
    @clang-format -i $(git ls-files '*.cpp' '*.hpp')

format-check:
    @clang-format --dry-run --Werror $(git ls-files '*.cpp' '*.hpp')

# Needs out/build/x64-debug-llvm/compile_commands.json, so run `just build` first.
lint:
    @python (Get-Command run-clang-tidy).Source -p out/build/x64-debug-llvm -quiet -warnings-as-errors='*'

check: format-check lint

install-hooks:
    @git config core.hooksPath .githooks
