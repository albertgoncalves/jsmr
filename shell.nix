with import <nixpkgs> {};
mkShell {
    buildInputs = [
        clang_10
        cppcheck
        glibcLocales
        openjdk
        shellcheck
        valgrind
    ];
    shellHook = ''
        . .shellhook
    '';
}
