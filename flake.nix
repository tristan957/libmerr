# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# SPDX-FileCopyrightText: Tristan Partin <tristan@partin.io>
{
  description = "libmerr - C99+ library for packing error information into a 64-bit integer";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    systems.url = "github:nix-systems/default";
  };

  outputs = {
    nixpkgs,
    systems,
    ...
  }: let
    forEachSystem = nixpkgs.lib.genAttrs (import systems);
  in {
    packages = forEachSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      default = pkgs.stdenv.mkDerivation {
        pname = "libmerr";
        version = pkgs.lib.trim (builtins.readFile ./VERSION);

        src = ./.;

        nativeBuildInputs = with pkgs; [
          meson
          ninja
          pkg-config
          python3
        ];

        buildInputs = with pkgs; [
          glib
        ];

        mesonFlags = [
          "-Dplain=disabled"
        ];

        doCheck = true;

        meta = {
          description = "C99+ library for packing error information into a 64-bit integer";
          homepage = "https://github.com/tristan957/libmerr";
          license = with pkgs.lib.licenses; [asl20 mit];
          platforms = pkgs.lib.platforms.unix;
        };
      };
    });

    devShells = forEachSystem (system: let
      pkgs = import nixpkgs {inherit system;};
    in {
      default = let
        # On Darwin, glib's glib-2.0.pc lists sysprof-capture-4 in
        # Requires.private but sysprof doesn't exist on macOS. Provide a stub
        # .pc so pkgconf can satisfy the dependency without errors.
        sysprofStub = pkgs.lib.optionalString pkgs.stdenv.isDarwin (
          pkgs.writeTextFile {
            name = "sysprof-capture-4-stub-pc";
            destination = "/lib/pkgconfig/sysprof-capture-4.pc";
            text = ''
              Name: sysprof-capture-4
              Description: stub for macOS
              Version: 3.38.0
            '';
          }
        );
        extraPkgConfigDirs = pkgs.lib.optional pkgs.stdenv.isDarwin "${sysprofStub}/lib/pkgconfig";
      in
        pkgs.mkShell {
          packages = with pkgs;
            [
              alejandra
              clang
              clang-tools
              markdownlint-cli2
              meson
              ninja
              nixd
              pkgconf
              prettier
              reuse
            ]
            ++ pkgs.lib.optionals pkgs.stdenv.isLinux (with pkgs; [
              gcc
            ]);

          # glib and pcre2 in buildInputs so mkShell's pkg-config setup hook
          # adds their -dev outputs (and transitive deps) to PKG_CONFIG_PATH.
          buildInputs = with pkgs; [glib pcre2];

          env =
            pkgs.lib.optionalAttrs pkgs.stdenv.isDarwin {CC = "clang";}
            // {PKG_CONFIG_PATH = pkgs.lib.concatStringsSep ":" extraPkgConfigDirs;};
        };
    });
  };
}
