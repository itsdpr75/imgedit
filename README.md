# imgedit

A image editor for Linux built with C++17 and Qt 6.

## Features

- **Image Viewer**: Zoom (mouse wheel with Ctrl, pinch) and pan (drag or middle mouse button)
- **Crop Tool**: Interactive cropping with freeform or fixed ratios
- **Drawing Brushes**: Adjustable size, color (with alpha), opacity, smooth stroke simulation
- **Blur Brush**: Brush mode and rectangle region mode with adjustable radius
- **Pixelate Brush**: Brush mode and rectangle region mode with adjustable pixel size
- **Text Tool**: Place editable text on image, system fonts via fontconfig
- **Eraser Tool**: Erase brush strokes and drawings
- **Sharpen Filter**: Laplacian-based filter optimized for documents and text
- **Color Adjustments** (real-time preview):
  - Basic: brightness, contrast, saturation, hue, temperature
  - Special: Highlights, Shadows
- **Undo/Redo**: Unlimited history per session
- **Project Files (.imgedit)**: Non-destructive editing with JSON format

## Requirements

### Build Dependencies

- **Qt 6** (Core, Gui, Widgets, Svg)
- **fontconfig**
- **CMake** 3.16 or higher
- **C++17** compatible compiler (clang or gcc)
- **Font Awesome 6 Free** (embedded in code)

### Runtime Dependencies

- Qt 6 runtime libraries
- fontconfig
- System fonts

## Build Instructions

### Using CMake (recommended)

```bash
# Clone or navigate to project directory
cd /path/to/imgedit

# Create build directory
mkdir build && cd build

# Configure with clang (or gcc)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

# Build
cmake --build . -j$(nproc)

# Run without installing
./imgedit image.png

# Install (optional)
sudo cmake --install .
```

### Using the install script

```bash
cd packaging
./install.sh

# Or specify install prefix
./install.sh --prefix=$HOME/.local

# System-wide (requires root)
sudo ./install.sh --prefix=/usr/local
```

## Usage

```bash
# Open an image
imgedit image.png

# Open a project file
imgedit project.imgedit
```

### Mouse Controls

- **Scroll wheel**: Vertical scroll
- **Ctrl + Scroll wheel**: Zoom in/out
- **Middle mouse button** or **Shift + Left click + drag**: Pan
- **Left click**: Use current tool
- **Right click**: (reserved)

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New project (opens image selector) |
| Ctrl+O | Open file |
| Ctrl+S | Save project |
| Ctrl+Shift+S | Save project as |
| Ctrl+Z | Undo |
| Ctrl+Y | Redo |
| Ctrl+0 | Zoom fit |
| Ctrl+1 | Actual size (100%) |
| Ctrl++ | Zoom in |
| Ctrl+- | Zoom out |
| C | Crop tool |
| B | Brush tool |
| E | Eraser tool |
| U | Blur tool |
| P | Pixelate tool |
| T | Text tool |
| V | Select tool |

## Filter Guide

### Color Adjustments

- **Brightness** (-100 to 100): Lighten or darken the image
- **Contrast** (-100 to 100): Increase or decrease contrast
- **Saturation** (-100 to 100): Adjust color intensity
- **Hue** (-180 to 180): Rotate colors around the color wheel
- **Temperature** (-100 to 100): Make warmer (yellow) or cooler (blue)
- **Highlights** (-100 to 100): Adjust only bright areas
- **Shadows** (-100 to 100): Adjust only dark areas
- **Sharpness** (0 to 200): Laplacian-based filter optimized for documents and text

### Using Filters

1. Select an image with File > New
2. Adjust sliders in the Properties panel (right side)
3. Changes apply in real-time

## Project Format

imgedit saves projects as `.imgedit` files which contain:

- Original image path
- Base64-encoded thumbnail for quick preview
- Operations list (crop, brush strokes, text items, filters)
- Filter settings

This allows non-destructive editing - the original image is never modified.

## Architecture

```
imgedit/
├── src/
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.cpp    # Main window and UI coordination
│   ├── core/             # Document model, operations, undo/redo
│   │   ├── imagedocument.cpp
│   │   ├── operation.cpp
│   │   └── undostack.cpp
│   ├── filters/          # Image processing filters
│   │   └── filter.cpp    # All filters (brightness, contrast, sharpen, etc.)
│   ├── tools/            # Drawing and editing tools
│   │   ├── tool.cpp      # Base tool class and implementations
│   │   └── brush.cpp
│   ├── ui/               # User interface components
│   │   ├── canvasview.cpp    # Image display with zoom/pan
│   │   ├── toolbar.cpp       # Tool selection toolbar
│   │   └── propertiespanel.cpp  # Filter controls
│   ├── project/          # Project file save/load
│   └── utils/            # Font Awesome icon system
├── packaging/
│   ├── imgedit.desktop   # Desktop integration
│   └── install.sh        # Installation script
├── CMakeLists.txt        # Build configuration
└── README.md
```

## License

MIT License - See LICENSE file for details
