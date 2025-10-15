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
