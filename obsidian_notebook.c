/**
 * @file obsidian_notebook.c
 * @brief A beautiful, Obsidian-inspired notebook application
 * @author Your Name
 * @version 1.0.0
 * @date 2026
 *
 * This is a lightweight, cross-platform note-taking application written in pure
 * C using raylib for graphics. It features a modern dark theme inspired by
 * Obsidian, full Unicode support (including Turkish characters), and
 * platform-native shortcuts.
 *
 * Features:
 *   - Dark theme with purple accents
 *   - Sidebar with note list
 *   - Markdown-style heading rendering
 *   - Full UTF-8 support (Turkish, Emoji, etc.)
 *   - macOS Cmd / Linux Ctrl shortcuts
 *   - Notes saved as .md files
 *
 * Dependencies:
 *   - raylib (https://www.raylib.com/)
 *
 * Build:
 *   macOS:  make
 *   Linux:  make
 *
 * License: MIT
 */

#include "raylib.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* ============================================================================
 * Platform Detection
 * ============================================================================
 * We detect the platform at compile time to use the appropriate modifier key:
 * - macOS: Command (⌘) key
 * - Linux: Control key
 */
#ifdef __APPLE__
#define IS_MACOS 1
#else
#define IS_MACOS 0
#endif

/* ============================================================================
 * Application Configuration
 * ============================================================================
 * Adjust these values to customize the application behavior and appearance.
 */
#define WINDOW_WIDTH 1200        /* Initial window width in pixels */
#define WINDOW_HEIGHT 800        /* Initial window height in pixels */
#define SIDEBAR_WIDTH 280        /* Width of the left sidebar */
#define HEADER_HEIGHT 50         /* Height of the top header bar */
#define MAX_NOTES 100            /* Maximum number of notes */
#define MAX_TITLE_LENGTH 128     /* Maximum characters in note title */
#define MAX_CONTENT_LENGTH 32768 /* Maximum characters in note content */
#define VAULT_FOLDER "vault"     /* Folder where notes are stored */

/* ============================================================================
 * Color Palette
 * ============================================================================
 * Obsidian-inspired dark theme colors. Each color includes an RGB value
 * and its hex equivalent for reference.
 */

/* Background colors */
#define BG_DARK (Color){30, 30, 30, 255}      /* Main background    #1e1e1e */
#define BG_SIDEBAR (Color){37, 37, 37, 255}   /* Sidebar background #252525 */
#define BG_HOVER (Color){45, 45, 45, 255}     /* Hover state        #2d2d2d */
#define BG_SELECTED (Color){72, 61, 139, 255} /* Selected item      Purple  */
#define BG_EDITOR (Color){35, 35, 35, 255}    /* Editor area        #232323 */
#define BG_HEADER (Color){28, 28, 28, 255}    /* Header bar         #1c1c1c */

/* Text colors */
#define TEXT_PRIMARY                                                           \
  (Color){220, 220, 220, 255} /* Primary text       #dcdcdc */
#define TEXT_SECONDARY                                                         \
  (Color){150, 150, 150, 255}                  /* Secondary text     #969696 */
#define TEXT_MUTED (Color){100, 100, 100, 255} /* Muted/hint text    #646464   \
                                                */

/* Accent colors */
#define ACCENT_PURPLE                                                          \
  (Color){138, 79, 255, 255}                   /* Primary accent     Purple  */
#define ACCENT_BLUE (Color){66, 165, 245, 255} /* Secondary accent   Blue */
#define BORDER_COLOR (Color){50, 50, 50, 255}  /* Border/divider     #323232 */

/* ============================================================================
 * Data Structures
 * ============================================================================
 */

/**
 * @brief Represents a single note
 */
typedef struct {
  char title[MAX_TITLE_LENGTH];     /* Note title (also used as filename) */
  char content[MAX_CONTENT_LENGTH]; /* Note content in plain text */
  char filepath[256];               /* Full path to the .md file */
  bool modified;                    /* True if note has unsaved changes */
} Note;

/**
 * @brief Application state container
 */
typedef struct {
  Note notes[MAX_NOTES]; /* Array of all notes */
  int count;             /* Number of notes currently loaded */
  int selected;          /* Index of currently selected note (-1 if none) */
  bool editingTitle;     /* True if user is editing note title */
  int cursorPos;         /* Cursor position in editor */
  int scrollOffset;      /* Scroll offset for sidebar */
  char searchQuery[128]; /* Current search query */
  bool showSearch;       /* True if search bar is visible */
} Notebook;

/* ============================================================================
 * Global State
 * ============================================================================
 */

static Notebook notebook = {0}; /* Main application state */
static Font mainFont;           /* Regular text font */
static Font boldFont;           /* Bold text font */

/* ============================================================================
 * UTF-8 Encoding Utilities
 * ============================================================================
 * These functions handle UTF-8 encoding for international character support,
 * including Turkish characters (ş, ğ, ü, ö, ç, ı, İ, Ş, Ğ, Ü, Ö, Ç).
 */

/**
 * @brief Encode a Unicode codepoint to UTF-8
 * @param codepoint The Unicode codepoint to encode
 * @param out Output buffer (must be at least 4 bytes)
 * @return Number of bytes written (1-4)
 */
static int encode_utf8(unsigned int codepoint, char *out) {
  if (codepoint <= 0x7F) {
    /* ASCII: single byte */
    out[0] = (char)codepoint;
    return 1;
  } else if (codepoint <= 0x7FF) {
    /* 2-byte sequence */
    out[0] = (char)(0xC0 | (codepoint >> 6));
    out[1] = (char)(0x80 | (codepoint & 0x3F));
    return 2;
  } else if (codepoint <= 0xFFFF) {
    /* 3-byte sequence (most common for Turkish) */
    out[0] = (char)(0xE0 | (codepoint >> 12));
    out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    out[2] = (char)(0x80 | (codepoint & 0x3F));
    return 3;
  } else if (codepoint <= 0x10FFFF) {
    /* 4-byte sequence (emoji, etc.) */
    out[0] = (char)(0xF0 | (codepoint >> 18));
    out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    out[3] = (char)(0x80 | (codepoint & 0x3F));
    return 4;
  }
  return 0;
}

/**
 * @brief Check if the platform modifier key is pressed
 * @return True if Cmd (macOS) or Ctrl (Linux) is held down
 */
static bool is_modifier_down(void) {
#if IS_MACOS
  return IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
#else
  return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
#endif
}

/**
 * @brief Get the byte length of the last UTF-8 character in a string
 * @param str The string to analyze
 * @param len Length of the string
 * @return Number of bytes in the last character (1-4)
 */
static int get_last_utf8_char_bytes(const char *str, int len) {
  if (len == 0)
    return 0;

  /* Walk backwards to find the start of the last UTF-8 character */
  int i = len - 1;
  while (i > 0 && (str[i] & 0xC0) == 0x80) {
    i--;
  }
  return len - i;
}

/* ============================================================================
 * File System Operations
 * ============================================================================
 */

/**
 * @brief Ensure the vault folder exists, create if needed
 */
static void ensure_vault_exists(void) {
  struct stat st = {0};
  if (stat(VAULT_FOLDER, &st) == -1) {
    mkdir(VAULT_FOLDER, 0700);
  }
}

/**
 * @brief Load all notes from the vault folder
 */
static void load_notes(void) {
  notebook.count = 0;
  DIR *dir = opendir(VAULT_FOLDER);
  if (dir == NULL)
    return;

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL && notebook.count < MAX_NOTES) {
    if (entry->d_type == DT_REG) {
      const char *ext = strrchr(entry->d_name, '.');
      if (ext && strcmp(ext, ".md") == 0) {
        Note *note = &notebook.notes[notebook.count];

        /* Extract title from filename (remove .md extension) */
        size_t name_len = strlen(entry->d_name) - 3;
        strncpy(note->title, entry->d_name, name_len);
        note->title[name_len] = '\0';

        /* Build full file path */
        snprintf(note->filepath, sizeof(note->filepath), "%s/%s", VAULT_FOLDER,
                 entry->d_name);

        /* Load file content */
        FILE *file = fopen(note->filepath, "r");
        if (file) {
          size_t bytes_read =
              fread(note->content, 1, MAX_CONTENT_LENGTH - 1, file);
          note->content[bytes_read] = '\0';
          fclose(file);
        }

        note->modified = false;
        notebook.count++;
      }
    }
  }
  closedir(dir);

  /* Create a welcome note if the vault is empty */
  if (notebook.count == 0) {
    Note *note = &notebook.notes[0];
    strcpy(note->title, "Welcome");
    snprintf(note->filepath, sizeof(note->filepath), "%s/Welcome.md",
             VAULT_FOLDER);

#if IS_MACOS
    strcpy(note->content,
           "# Welcome to Notes! 📝\n\n"
           "This is your personal notebook, inspired by Obsidian.\n\n"
           "## Features\n\n"
           "- **Create** new notes with the + button\n"
           "- **Edit** notes in the editor panel\n"
           "- **Delete** notes with right-click\n"
           "- **Search** notes with ⌘F\n\n"
           "## Keyboard Shortcuts\n\n"
           "- `⌘N` - New note\n"
           "- `⌘S` - Save note\n"
           "- `⌘F` - Search\n\n"
           "Supports Turkish keyboard: ş, ğ, ü, ö, ç, ı\n\n"
           "Start writing your notes!\n");
#else
    strcpy(note->content,
           "# Welcome to Notes! 📝\n\n"
           "This is your personal notebook, inspired by Obsidian.\n\n"
           "## Features\n\n"
           "- **Create** new notes with the + button\n"
           "- **Edit** notes in the editor panel\n"
           "- **Delete** notes with right-click\n"
           "- **Search** notes with Ctrl+F\n\n"
           "## Keyboard Shortcuts\n\n"
           "- `Ctrl+N` - New note\n"
           "- `Ctrl+S` - Save note\n"
           "- `Ctrl+F` - Search\n\n"
           "Supports Turkish keyboard: ş, ğ, ü, ö, ç, ı\n\n"
           "Start writing your notes!\n");
#endif
    note->modified = true;
    notebook.count = 1;
    notebook.selected = 0;
  }
}

/**
 * @brief Save a single note to disk
 * @param note Pointer to the note to save
 */
static void save_note(Note *note) {
  if (!note->modified)
    return;

  /* Update filepath in case title changed */
  snprintf(note->filepath, sizeof(note->filepath), "%s/%s.md", VAULT_FOLDER,
           note->title);

  FILE *file = fopen(note->filepath, "w");
  if (file) {
    fprintf(file, "%s", note->content);
    fclose(file);
    note->modified = false;
  }
}

/**
 * @brief Save all notes to disk
 */
static void save_all_notes(void) {
  for (int i = 0; i < notebook.count; i++) {
    save_note(&notebook.notes[i]);
  }
}

/**
 * @brief Create a new empty note
 */
static void create_new_note(void) {
  if (notebook.count >= MAX_NOTES)
    return;

  Note *note = &notebook.notes[notebook.count];

  /* Generate unique title */
  int note_num = notebook.count + 1;
  snprintf(note->title, MAX_TITLE_LENGTH, "Untitled %d", note_num);
  snprintf(note->filepath, sizeof(note->filepath), "%s/%s.md", VAULT_FOLDER,
           note->title);

  note->content[0] = '\0';
  note->modified = true;

  notebook.selected = notebook.count;
  notebook.count++;
  notebook.cursorPos = 0;
}

/**
 * @brief Delete a note by index
 * @param index Index of the note to delete
 */
static void delete_note(int index) {
  if (index < 0 || index >= notebook.count)
    return;

  /* Delete the file from disk */
  remove(notebook.notes[index].filepath);

  /* Shift remaining notes to fill the gap */
  for (int i = index; i < notebook.count - 1; i++) {
    notebook.notes[i] = notebook.notes[i + 1];
  }
  notebook.count--;

  /* Adjust selection */
  if (notebook.selected >= notebook.count) {
    notebook.selected = notebook.count - 1;
  }
  if (notebook.selected < 0) {
    notebook.selected = 0;
  }
}

/* ============================================================================
 * Drawing Functions
 * ============================================================================
 */

/**
 * @brief Draw the header bar
 */
static void draw_header(void) {
  /* Background */
  DrawRectangle(0, 0, WINDOW_WIDTH, HEADER_HEIGHT, BG_HEADER);
  DrawRectangle(0, HEADER_HEIGHT - 1, WINDOW_WIDTH, 1, BORDER_COLOR);

  /* App title */
  DrawTextEx(mainFont, "📓 Notes", (Vector2){20, 14}, 22, 1, TEXT_PRIMARY);

  /* Current note title */
  if (notebook.count > 0 && notebook.selected >= 0) {
    Note *note = &notebook.notes[notebook.selected];
    char title_display[150];
    snprintf(title_display, sizeof(title_display), " / %s%s", note->title,
             note->modified ? " •" : "");
    DrawTextEx(mainFont, title_display, (Vector2){130, 14}, 22, 1,
               TEXT_SECONDARY);
  }

  /* Search box (when visible) */
  if (notebook.showSearch) {
    DrawRectangleRounded((Rectangle){WINDOW_WIDTH - 250, 10, 230, 30}, 0.3f, 8,
                         BG_SIDEBAR);
    DrawTextEx(mainFont, "🔍", (Vector2){WINDOW_WIDTH - 240, 14}, 18, 1,
               TEXT_SECONDARY);
    DrawTextEx(mainFont, notebook.searchQuery,
               (Vector2){WINDOW_WIDTH - 215, 14}, 18, 1, TEXT_PRIMARY);
  }
}

/**
 * @brief Draw the sidebar with note list
 */
static void draw_sidebar(void) {
  /* Background */
  DrawRectangle(0, HEADER_HEIGHT, SIDEBAR_WIDTH, WINDOW_HEIGHT - HEADER_HEIGHT,
                BG_SIDEBAR);
  DrawRectangle(SIDEBAR_WIDTH - 1, HEADER_HEIGHT, 1, WINDOW_HEIGHT,
                BORDER_COLOR);

  /* Section header */
  DrawTextEx(mainFont, "NOTES", (Vector2){20, HEADER_HEIGHT + 15}, 12, 1,
             TEXT_MUTED);

  /* New note button */
  Rectangle new_btn = {15, HEADER_HEIGHT + 40, SIDEBAR_WIDTH - 30, 35};
  bool hover_new = CheckCollisionPointRec(GetMousePosition(), new_btn);
  DrawRectangleRounded(new_btn, 0.2f, 8, hover_new ? ACCENT_PURPLE : BG_HOVER);
  DrawTextEx(mainFont, "+ New Note", (Vector2){new_btn.x + 70, new_btn.y + 8},
             16, 1, TEXT_PRIMARY);

  if (hover_new && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    create_new_note();
  }

  /* Note list */
  int start_y = HEADER_HEIGHT + 90;
  int item_height = 40;

  for (int i = 0; i < notebook.count; i++) {
    int y = start_y + i * item_height - notebook.scrollOffset;

    /* Skip items outside visible area */
    if (y < HEADER_HEIGHT + 85 || y > WINDOW_HEIGHT - item_height)
      continue;

    Rectangle item_rect = {10, y, SIDEBAR_WIDTH - 20, item_height - 5};
    bool hover = CheckCollisionPointRec(GetMousePosition(), item_rect);
    bool selected = (i == notebook.selected);

    /* Draw background */
    if (selected) {
      DrawRectangleRounded(item_rect, 0.2f, 8, BG_SELECTED);
    } else if (hover) {
      DrawRectangleRounded(item_rect, 0.2f, 8, BG_HOVER);
    }

    /* Draw note title with icon */
    Note *note = &notebook.notes[i];
    char display[150];
    snprintf(display, sizeof(display), "📄 %s%s", note->title,
             note->modified ? " •" : "");
    DrawTextEx(mainFont, display, (Vector2){item_rect.x + 10, item_rect.y + 10},
               15, 1, selected ? TEXT_PRIMARY : TEXT_SECONDARY);

    /* Handle clicks */
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      notebook.selected = i;
      notebook.cursorPos = strlen(note->content);
    }

    /* Right-click to delete */
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      delete_note(i);
    }
  }
}

/**
 * @brief Draw the main editor area
 */
static void draw_editor(void) {
  int editor_x = SIDEBAR_WIDTH;
  int editor_y = HEADER_HEIGHT;
  int editor_width = WINDOW_WIDTH - SIDEBAR_WIDTH;
  int editor_height = WINDOW_HEIGHT - HEADER_HEIGHT;

  /* Background */
  DrawRectangle(editor_x, editor_y, editor_width, editor_height, BG_EDITOR);

  /* Empty state */
  if (notebook.count == 0 || notebook.selected < 0) {
    const char *empty_msg = "Create a new note to get started";
    Vector2 text_size = MeasureTextEx(mainFont, empty_msg, 20, 1);
    DrawTextEx(mainFont, empty_msg,
               (Vector2){editor_x + (editor_width - text_size.x) / 2,
                         editor_y + editor_height / 2 - 10},
               20, 1, TEXT_MUTED);
    return;
  }

  Note *note = &notebook.notes[notebook.selected];

  /* Layout */
  int padding = 40;
  int content_x = editor_x + padding;
  int content_y = editor_y + padding;
  int content_width = editor_width - padding * 2;

  /* Draw title */
  DrawTextEx(boldFont, note->title, (Vector2){content_x, content_y}, 32, 1,
             TEXT_PRIMARY);

  /* Separator line */
  DrawRectangle(content_x, content_y + 45, content_width, 1, BORDER_COLOR);

  /* Draw content with word wrap and markdown styling */
  int text_y = content_y + 60;
  int line_height = 24;
  int max_width = content_width - 20;

  char *content = note->content;
  char line[256];
  int char_index = 0;

  while (content[char_index] != '\0' && text_y < WINDOW_HEIGHT - 30) {
    /* Find line boundaries */
    int line_len = 0;
    int last_space = -1;

    while (content[char_index + line_len] != '\0' &&
           content[char_index + line_len] != '\n') {
      if (content[char_index + line_len] == ' ') {
        last_space = line_len;
      }

      /* Check if line exceeds width */
      strncpy(line, content + char_index, line_len + 1);
      line[line_len + 1] = '\0';
      Vector2 size = MeasureTextEx(mainFont, line, 18, 1);

      if (size.x > max_width && last_space > 0) {
        line_len = last_space;
        break;
      }
      line_len++;
    }

    /* Extract the line */
    strncpy(line, content + char_index, line_len);
    line[line_len] = '\0';

    /* Apply markdown styling */
    Color line_color = TEXT_PRIMARY;
    int font_size = 18;

    if (line[0] == '#' && line[1] == ' ') {
      /* H1 heading */
      line_color = ACCENT_PURPLE;
      font_size = 24;
      DrawTextEx(boldFont, line + 2, (Vector2){content_x, text_y}, font_size, 1,
                 line_color);
    } else if (line[0] == '#' && line[1] == '#' && line[2] == ' ') {
      /* H2 heading */
      line_color = ACCENT_BLUE;
      font_size = 20;
      DrawTextEx(boldFont, line + 3, (Vector2){content_x, text_y}, font_size, 1,
                 line_color);
    } else if (line[0] == '-' && line[1] == ' ') {
      /* Bullet point */
      DrawTextEx(mainFont, "•", (Vector2){content_x, text_y}, font_size, 1,
                 ACCENT_PURPLE);
      DrawTextEx(mainFont, line + 2, (Vector2){content_x + 15, text_y},
                 font_size, 1, line_color);
    } else {
      /* Normal text */
      DrawTextEx(mainFont, line, (Vector2){content_x, text_y}, font_size, 1,
                 line_color);
    }

    /* Move to next line */
    char_index += line_len;
    if (content[char_index] == '\n')
      char_index++;
    else if (content[char_index] == ' ')
      char_index++;

    text_y += line_height;
  }

  /* Blinking cursor */
  if ((int)(GetTime() * 2) % 2 == 0) {
    DrawRectangle(content_x, text_y, 2, line_height, ACCENT_PURPLE);
  }
}

/**
 * @brief Draw the status bar at the bottom
 */
static void draw_status_bar(void) {
  int bar_height = 25;
  int bar_y = WINDOW_HEIGHT - bar_height;

  DrawRectangle(0, bar_y, WINDOW_WIDTH, bar_height, BG_HEADER);
  DrawRectangle(0, bar_y, WINDOW_WIDTH, 1, BORDER_COLOR);

  /* Statistics */
  char status[128];
  if (notebook.count > 0 && notebook.selected >= 0) {
    Note *note = &notebook.notes[notebook.selected];
    int char_count = strlen(note->content);
    int word_count = 0;
    bool in_word = false;

    for (int i = 0; note->content[i]; i++) {
      if (note->content[i] == ' ' || note->content[i] == '\n') {
        in_word = false;
      } else if (!in_word) {
        in_word = true;
        word_count++;
      }
    }
    snprintf(status, sizeof(status), "%d notes | %d words | %d characters",
             notebook.count, word_count, char_count);
  } else {
    snprintf(status, sizeof(status), "%d notes", notebook.count);
  }

  DrawTextEx(mainFont, status, (Vector2){15, bar_y + 5}, 14, 1, TEXT_MUTED);

  /* Keyboard shortcuts hint */
#if IS_MACOS
  const char *shortcuts = "⌘N: New | ⌘S: Save | Right-click: Delete";
#else
  const char *shortcuts = "Ctrl+N: New | Ctrl+S: Save | Right-click: Delete";
#endif
  Vector2 shortcut_size = MeasureTextEx(mainFont, shortcuts, 14, 1);
  DrawTextEx(mainFont, shortcuts,
             (Vector2){WINDOW_WIDTH - shortcut_size.x - 15, bar_y + 5}, 14, 1,
             TEXT_MUTED);
}

/* ============================================================================
 * Input Handling
 * ============================================================================
 */

/**
 * @brief Process all user input
 */
static void handle_input(void) {
  /* Keyboard shortcuts */
  if (is_modifier_down()) {
    if (IsKeyPressed(KEY_N)) {
      create_new_note();
    }
    if (IsKeyPressed(KEY_S)) {
      if (notebook.selected >= 0) {
        save_note(&notebook.notes[notebook.selected]);
      }
    }
    if (IsKeyPressed(KEY_F)) {
      notebook.showSearch = !notebook.showSearch;
      if (!notebook.showSearch) {
        notebook.searchQuery[0] = '\0';
      }
    }
  }

  /* Text input (supports Unicode / Turkish) */
  if (notebook.count > 0 && notebook.selected >= 0) {
    Note *note = &notebook.notes[notebook.selected];

    /* Process Unicode character input */
    int codepoint = GetCharPressed();
    while (codepoint > 0) {
      if (codepoint >= 32) { /* Printable characters only */
        int len = strlen(note->content);
        char utf8[5] = {0};
        int utf8_len = encode_utf8(codepoint, utf8);

        if (len + utf8_len < MAX_CONTENT_LENGTH - 1) {
          strcat(note->content, utf8);
          note->modified = true;
        }
      }
      codepoint = GetCharPressed();
    }

    /* Backspace (handles multi-byte UTF-8) */
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
      int len = strlen(note->content);
      if (len > 0) {
        int char_bytes = get_last_utf8_char_bytes(note->content, len);
        note->content[len - char_bytes] = '\0';
        note->modified = true;
      }
    }

    /* Enter key */
    if (IsKeyPressed(KEY_ENTER)) {
      int len = strlen(note->content);
      if (len < MAX_CONTENT_LENGTH - 1) {
        note->content[len] = '\n';
        note->content[len + 1] = '\0';
        note->modified = true;
      }
    }

    /* Tab key (insert 4 spaces) */
    if (IsKeyPressed(KEY_TAB)) {
      int len = strlen(note->content);
      if (len < MAX_CONTENT_LENGTH - 4) {
        strcat(note->content, "    ");
        note->modified = true;
      }
    }
  }

  /* Sidebar scrolling */
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouse = GetMousePosition();
    if (mouse.x < SIDEBAR_WIDTH) {
      notebook.scrollOffset -= (int)(wheel * 30);
      if (notebook.scrollOffset < 0) {
        notebook.scrollOffset = 0;
      }
      int max_scroll =
          notebook.count * 40 - (WINDOW_HEIGHT - HEADER_HEIGHT - 100);
      if (max_scroll < 0)
        max_scroll = 0;
      if (notebook.scrollOffset > max_scroll) {
        notebook.scrollOffset = max_scroll;
      }
    }
  }
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================
 */

int main(void) {
  /* Configure window */
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Notes - Obsidian-like Notebook");
  SetTargetFPS(60);

  /* Load fonts */
  mainFont = GetFontDefault();
  boldFont = GetFontDefault();

  /* Initialize file system */
  ensure_vault_exists();
  load_notes();

  if (notebook.count > 0) {
    notebook.selected = 0;
  }

  /* Main loop */
  while (!WindowShouldClose()) {
    handle_input();

    BeginDrawing();
    ClearBackground(BG_DARK);

    draw_sidebar();
    draw_editor();
    draw_header();
    draw_status_bar();

    EndDrawing();
  }

  /* Save all notes before exit */
  save_all_notes();

  CloseWindow();
  return 0;
}
