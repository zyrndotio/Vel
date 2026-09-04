# Editor and GitHub Support

Vel source files use the `.vel` extension. A publish-ready VS Code language extension is delivered separately from the compiler repository as `vel-language-vscode-0.3.0.zip`.

## VS Code extension

The extension currently provides:

- Vel language registration for `.vel` files.
- Syntax highlighting for comments, strings, numbers, keywords, built-in functions, types, booleans, operators, declarations, and calls.
- Comment toggling, bracket matching, auto-closing pairs, and indentation behavior.
- Snippets for functions, `main`, mutable variables, and structs.

Extract the standalone package, then package and install a VSIX:

```bash
unzip vel-language-vscode-0.3.0.zip -d vel-language-vscode
cd vel-language-vscode
npx @vscode/vsce package
code --install-extension vel-language-0.3.0.vsix
```

The extension is intentionally dependency-free at runtime. Publishing it to the Visual Studio Marketplace requires a verified publisher account and a Marketplace personal access token; those credentials must be supplied by the project maintainer and should never be committed to this repository.

## GitHub support

`.gitattributes` marks `*.vel` as Vel source for GitHub Linguist and excludes local build products from language statistics. GitHub will recognize the file extension for repository statistics. Rich GitHub-native grammar highlighting depends on Vel being accepted into the upstream [Linguist language definitions](https://github.com/github-linguist/linguist); the repository metadata is the correct foundation for that contribution.

## Editor roadmap

The next editor milestones are:

1. A standalone Language Server Protocol implementation for diagnostics, go-to-definition, hover, and completion.
2. VS Code tasks for `vel check`, `vel build`, and `vel run`, with problem matchers for line and column diagnostics.
3. Formatting and semantic token support.
4. Adapters for Zed, Sublime Text, Neovim, and Emacs using the grammar and language server.
5. Debug Adapter Protocol support once native debug metadata and a stable runtime ABI are available.

Syntax highlighting is useful immediately, but a language server should only be shipped after diagnostics and project/module semantics are stable enough to avoid misleading editor behavior.
