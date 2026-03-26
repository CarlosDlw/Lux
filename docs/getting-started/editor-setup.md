# Editor Setup

This page covers editor configuration for working with T (`.lx`) files. Editor tooling is still in early stages — this page documents what is available today and what is planned.

## Current Status

T does not yet have a dedicated editor extension or language server (LSP). The sections below describe workarounds and plans for future tooling.

## VS Code

### Syntax Highlighting (Planned)

A TextMate grammar for `.lx` files is planned. This will provide:

- Keyword highlighting (`if`, `else`, `for`, `while`, `ret`, `use`, `namespace`, etc.)
- Type highlighting (`int32`, `float64`, `bool`, `string`, `void`, etc.)
- String and character literal highlighting
- Comment highlighting (`//` line comments, `/* */` block comments)
- Operator and punctuation highlighting

Until then, you can set `.lx` files to use C/C++ syntax highlighting as a rough approximation:

1. Open a `.lx` file in VS Code.
2. Click the language indicator in the bottom-right corner (it may say "Plain Text").
3. Select "C" or "C++" from the list.

To make this permanent, add to your `settings.json`:

```json
{
    "files.associations": {
        "*.lx": "c"
    }
}
```

### Language Server (Planned)

A language server for Lux is planned for the future. It will provide:

- Diagnostics (error highlighting as you type)
- Go to definition
- Hover information (types, documentation)
- Auto-completion for keywords, types, stdlib functions
- Rename refactoring

### ANTLR4 Extension (For Compiler Development)

If you are working on the Lux compiler itself, the **ANTLR4** VS Code extension is useful for editing the grammar files (`.g4`):

1. Install the **ANTLR4** extension from the VS Code marketplace.
2. The project already includes `.vscode/settings.json` configured for grammar generation.
3. When you save a `.g4` file, the extension automatically regenerates the C++ parser files in `src/generated/`.

## Other Editors

### Vim / Neovim

You can set up basic filetype detection for `.lx` files:

```vim
" In ~/.config/nvim/filetype.vim or ~/.vim/filetype.vim
au BufRead,BufNewFile *.lx set filetype=c
```

This gives you C-like syntax highlighting as a temporary measure.

### Emacs

Add to your Emacs configuration:

```elisp
(add-to-list 'auto-mode-alist '("\\.lx\\'" . c-mode))
```

## Contributing

If you are interested in building editor tooling for T (syntax highlighting, LSP, tree-sitter grammar), contributions are welcome. The formal grammar is defined in the ANTLR4 files:

- `grammar/LuxLexer.g4` — Lexer rules (tokens, keywords, operators)
- `grammar/LuxParser.g4` — Parser rules (syntax structure)

These files are the authoritative source for T's syntax and can serve as a starting point for a TextMate grammar or tree-sitter parser.

## See Also

- [Installation](installation.md) — Build the compiler from source
- [Hello World](hello-world.md) — Write and run your first program
- [CLI Usage](cli-usage.md) — Compiler flags and options
