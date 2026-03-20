{
  description = "YOLO - Your Own Local Opinion: censorship-resistant community board";
  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
    nixpkgs.follows = "logos-module-builder/nixpkgs";
  };
  outputs = { self, logos-module-builder, nixpkgs, ... }:
    let
      moduleOutputs = logos-module-builder.lib.mkLogosModule {
        src = ./.;
        configFile = ./module.yaml;
      };
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
      });
    in
    moduleOutputs // {
      packages = forAllSystems ({ pkgs }:
        let
          base = moduleOutputs.packages.${pkgs.system} or {};
          # Add metadata.json to the lib output for lgx bundler
          lib-with-metadata = (base.lib or pkgs.emptyDirectory).overrideAttrs (old: {
            postInstall = (old.postInstall or "") + ''
              cp ${./src/metadata.json} $out/lib/metadata.json
            '';
          });
        in
        base // { lib = lib-with-metadata; }
      );
    };
}
