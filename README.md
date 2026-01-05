# Notes - An Obsidian-like Notebook

A beautiful, lightweight note-taking application written in pure C with a modern dark theme inspired by [Obsidian](https://obsidian.md/).

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey.svg)
![Language](https://img.shields.io/badge/language-C-orange.svg)

## Features

- **Dark Theme** — Obsidian-inspired color palette
- **Markdown Styling** — Headers and bullet points rendered with colors
- **Full Unicode** — Turkish, Emoji, and international characters
- **Native Shortcuts** — Cmd on macOS, Ctrl on Linux
- **Auto-save** — Notes saved as `.md` files
- **Search** — Find notes quickly

## Preview

```
+---------------------------------------------------------------------+
|  Notes                                                    _ [] X    |
+---------------+-----------------------------------------------------+
| NOTES         |  # Welcome Note                                     |
| ------------- |                                                     |
| > Welcome   * |  This is your first note!                           |
| > Ideas       |                                                     |
| > Tasks       |  ## Features                                        |
|               |  - Create notes                                     |
| [+ New Note]  |  - Edit content                                     |
+---------------+-----------------------------------------------------+
```

## Getting Started

### Prerequisites

- **raylib** — Simple game library for graphics
- **GCC** — C compiler

#### Install raylib

**macOS:**
```bash
brew install raylib
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt install libraylib-dev
```

**Linux (Arch):**
```bash
sudo pacman -S raylib
```

### Build

```bash
git clone https://github.com/yourusername/notes-app.git
cd notes-app
make
```

### Run

```bash
./notes
```

## Keyboard Shortcuts

| macOS | Linux | Action |
|-------|-------|--------|
| Cmd+N | Ctrl+N | New note |
| Cmd+S | Ctrl+S | Save note |
| Cmd+F | Ctrl+F | Search |
| — | — | Right-click to delete |

## Project Structure

```
notes-app/
├── obsidian_notebook.c   # Main source code
├── Makefile              # Build configuration
├── README.md             # This file
├── LICENSE               # MIT License
└── vault/                # Notes storage (auto-created)
    ├── Welcome.md
    └── ...
```

## Build from Source

### Manual Compilation

**macOS:**
```bash
gcc -o notes obsidian_notebook.c \
    -I/opt/homebrew/include -L/opt/homebrew/lib \
    -lraylib -framework OpenGL -framework Cocoa \
    -framework IOKit -framework CoreVideo
```

**Linux:**
```bash
gcc -o notes obsidian_notebook.c \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by [Obsidian](https://obsidian.md/)
- Built with [raylib](https://www.raylib.com/)
- Color palette based on popular dark themes

---

Made with C
