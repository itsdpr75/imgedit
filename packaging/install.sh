#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

PREFIX="/usr/local"
BIN_DIR="$PREFIX/bin"
SHARE_DIR="$PREFIX/share"

BUILD_TYPE="Release"
CMAKE_GENERATOR="Unix Makefiles"

usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --prefix PREFIX       Installation prefix (default: /usr/local)"
    echo "  --build-type TYPE     CMake build type: Release, Debug (default: Release)"
    echo "  --help                Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Install to /usr/local (requires root)"
    echo "  $0 --prefix ~/.local  # Install to ~/.local (user install)"
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix)
            PREFIX="$2"
            shift 2
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

BIN_DIR="$PREFIX/bin"
SHARE_DIR="$PREFIX/share"

echo "Building imgedit..."
echo "  Prefix: $PREFIX"
echo "  Build type: $BUILD_TYPE"

BUILD_DIR="$PROJECT_ROOT/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

if command -v cmake &> /dev/null; then
    cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
             -DCMAKE_INSTALL_PREFIX="$PREFIX" \
             -DCMAKE_CXX_COMPILER=clang++ \
             -DCMAKE_C_COMPILER=clang
else
    echo "Error: CMake not found. Please install CMake."
    exit 1
fi

echo "Compiling..."
cmake --build . -j$(nproc)

echo "Installing..."
if [[ "$PREFIX" == "/usr/local" ]] && [[ ! -w "/usr/local/bin" ]]; then
    echo "Note: Installing to /usr/local requires root. Using sudo..."
    sudo cmake --install .
else
    cmake --install .
fi

echo ""
echo "Creating desktop integration..."

DESKTOP_DIR="$SHARE_DIR/applications"
mkdir -p "$DESKTOP_DIR"

DESKTOP_FILE="$DESKTOP_DIR/imgedit.desktop"

if [ -f "$SCRIPT_DIR/imgedit.desktop" ]; then
    cp "$SCRIPT_DIR/imgedit.desktop" "$DESKTOP_FILE"
    echo "  Desktop file installed to: $DESKTOP_FILE"
else
    cat > "$DESKTOP_FILE" << 'EOF'
[Desktop Entry]
Name=imgedit
Comment=A production-ready image editor for Linux
Exec=imgedit %F
Icon=imgedit
Terminal=false
Type=Application
Categories=Graphics;2DGraphics;RasterGraphics;
MimeType=image/png;image/jpeg;image/bmp;image/gif;image/tiff;application/x-imgedit;
Keywords=image;photo;editor;graphics;design;
StartupNotify=true
EOF
    echo "  Desktop file created at: $DESKTOP_FILE"
fi

ICON_DIR="$SHARE_DIR/icons/hicolor/256x256/apps"
mkdir -p "$ICON_DIR"

echo "  Note: You may need to install an icon manually to: $ICON_DIR"

if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
fi

if command -v update-mime-database &> /dev/null; then
    MIME_DIR="$SHARE_DIR/mime"
    mkdir -p "$MIME_DIR/packages"
    echo "application/x-imgedit" > "$MIME_DIR/packages/imgedit.xml"
    update-mime-database "$MIME_DIR" 2>/dev/null || true
    echo "  MIME type registered"
fi

echo ""
echo "Installation complete!"
echo "  Binary: $BIN_DIR/imgedit"
echo "  Desktop file: $DESKTOP_FILE"
echo ""
echo "Run 'imgedit' to start the application."

if [[ "$PREFIX" != "/usr/local" ]] && [[ "$PATH" != *"$PREFIX/bin"* ]]; then
    echo ""
    echo "Warning: $PREFIX/bin is not in your PATH."
    echo "You may want to add it: export PATH=\"$PREFIX/bin:\$PATH\""
fi