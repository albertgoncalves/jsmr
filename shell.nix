with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_16
        cppcheck
        openjdk
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
