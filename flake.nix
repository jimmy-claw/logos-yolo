{
  description = "YOLO community board — Logos UI plugin";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder/4fa6816bc065f974169150448c066ef4047a2e43";
    nixpkgs.follows = "logos-module-builder/nixpkgs";
    logos-cpp-sdk = {
      url = "github:logos-co/logos-cpp-sdk";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    logos-liblogos = {
      url = "github:logos-co/logos-liblogos";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.logos-cpp-sdk.follows = "logos-cpp-sdk";
    };
    logos-pipe-src = {
      url = "path:/home/jimmy/logos-pipe";
      flake = false;
    };
  };

  outputs = { self, logos-module-builder, nixpkgs, logos-cpp-sdk, logos-liblogos, logos-pipe-src, ... }:
    let
      moduleOutputs = logos-module-builder.lib.mkLogosModule {
        src = ./.;
        configFile = ./module.yaml;
      };
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosSdk = logos-cpp-sdk.packages.${system}.default;
        logosLiblogos = logos-liblogos.packages.${system}.default;
      });
    in
    moduleOutputs // {
      packages = forAllSystems ({ pkgs, logosSdk, logosLiblogos }:
        let
          base = moduleOutputs.packages.${pkgs.system} or {};

          commonCmakeFlags = [
            "-DLOGOS_CPP_SDK_ROOT=${logosSdk}"
            "-DLOGOS_LIBLOGOS_ROOT=${logosLiblogos}"
            "-DLOGOS_PIPE_ROOT=${logos-pipe-src}"
          ];

          # Headless plugin built with LOGOS_PIPE_ROOT so YOLO_HAS_BOARD is enabled
          headless-plugin = pkgs.stdenv.mkDerivation {
            pname = "yolo-plugin";
            version = "0.1.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
            ];

            buildInputs = [
              pkgs.qt6.qtbase
              pkgs.qt6.qtremoteobjects
            ];

            cmakeFlags = commonCmakeFlags ++ [
              "-DBUILD_MODULE=ON"
              "-DBUILD_UI_PLUGIN=OFF"
              "-DBUILD_TESTS=OFF"
              "-GNinja"
            ];

            buildPhase = ''
              runHook preBuild
              ninja yolo_plugin -j''${NIX_BUILD_CORES:-1}
              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out/lib
              cp libyolo_plugin${pkgs.stdenv.hostPlatform.extensions.sharedLibrary} $out/lib/ 2>/dev/null || \
              cp yolo_plugin${pkgs.stdenv.hostPlatform.extensions.sharedLibrary} $out/lib/ 2>/dev/null || true
              runHook postInstall
            '';

            dontWrapQtApps = true;
          };

          ui-plugin = pkgs.stdenv.mkDerivation {
            pname = "yolo-ui-plugin";
            version = "0.1.0";
            src = ./.;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
              pkgs.qt6.wrapQtAppsHook
            ];

            buildInputs = [
              pkgs.qt6.qtbase
              pkgs.qt6.qtdeclarative
              pkgs.qt6.qtremoteobjects
            ];

            cmakeFlags = commonCmakeFlags ++ [
              "-DBUILD_UI_PLUGIN=ON"
            ];

            buildPhase = ''
              runHook preBuild
              cmake --build . --target yolo_ui -j''${NIX_BUILD_CORES:-1}
              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out/lib
              cp libyolo_ui${pkgs.stdenv.hostPlatform.extensions.sharedLibrary} $out/lib/
              runHook postInstall
            '';

            dontWrapQtApps = true;
          };

          # .lgx bundle: metadata + headless plugin (with board support) + UI plugin + QML
          lgx = pkgs.runCommand "yolo.lgx" {} ''
            mkdir -p $out/yolo

            # Headless module plugin (built with LOGOS_PIPE_ROOT -> YOLO_HAS_BOARD)
            cp ${headless-plugin}/lib/yolo_plugin* $out/yolo/ 2>/dev/null || true

            # UI plugin
            cp ${ui-plugin}/lib/libyolo_ui* $out/yolo/ 2>/dev/null || true

            # QML files
            mkdir -p $out/yolo/qml
            cp -r ${./qml}/* $out/yolo/qml/

            # Metadata
            cp ${./metadata.json} $out/yolo/metadata.json
            cp ${./manifest.json} $out/yolo/manifest.json
            cp ${./ui_metadata.json} $out/yolo/ui_metadata.json

            # Create the .lgx archive (tar.gz with .lgx extension)
            cd $out
            tar czf $out/yolo.lgx -C $out yolo
          '';

        in
        base // {
          ui = pkgs.runCommand "yolo-ui" {} ''
            mkdir -p $out/qml
            cp -r ${./qml}/* $out/qml/
          '';
          inherit headless-plugin ui-plugin lgx;
        }
      );
    };
}
