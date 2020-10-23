with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        openjdk
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
