{
  description = "Dev shell for Laboratorio 03: C++ Dynamic Module Loading";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      allSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs allSystems (system: f system);
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
        {
          default = pkgs.mkShell {
            name = "laboratorio-03-dev";

            packages = with pkgs; [
              # --- Build tools (only need PATH) ---
	            gcc
              cmake
              gnumake

              # --- Debug & Analysis ---
              gdb
              valgrind
            ];

            buildInputs = with pkgs; [
              # --- Compilers (need NIX_CFLAGS/NIX_LDFLAGS for headers) ---
              #gcc
              clang-tools               # clangd, clang-tidy, clang-format
              #glibc
            ];

            shellHook = ''
              echo ""
              echo "┌──────────────────────────────────────────────┐"
              echo "│  Laboratorio 03 — Dev Environment Active     │"
              echo "├──────────────────────────────────────────────┤"
              echo "│  g++  : $(g++ --version | head -1)            "
              echo "│  make : $(make --version | head -1)           "
              echo "│  cmake: $(cmake --version | head -1)          "
              echo "│  gdb  : $(gdb --version | head -1)            "
              echo "│  nix  : $(nix --version)                      "
              echo "└──────────────────────────────────────────────┘"
              echo ""
              echo "Quick start:"
              echo "  bash build.sh           # build library + executable"
              echo "  ./hostApp.bin              # run the app"
              echo ""
            '';
          };
        });
    };
}
