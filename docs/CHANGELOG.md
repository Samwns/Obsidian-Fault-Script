# Changelog

Todas as mudancas relevantes deste projeto sao registradas aqui.

## [1.0.3] - 2026-04-01
### Added
- Novas palavras-chave: `match`, `case`, `default`, `const`, `strata`, `tremor`, `catch`, `throw`.
- Novos operadores: `<<`, `>>`, `%=`.
- Suporte a operadores bitwise em expressoes binarias (`&`, `|`, `^`, `<<`, `>>`).
- Novos nos de AST para:
  - pattern matching (`MatchStmt`, `MatchArm`)
  - variavel imutavel (`ConstStmt`)
  - enum (`StrataDecl`)
  - tratamento de erro (`TremorStmt`, `ThrowStmt`)

### Changed
- Parser atualizado para reconhecer `strata`, `const`, `match/case/default`, `tremor/catch`, `throw`.
- Analise semantica com stubs e validacoes basicas para novos constructs.
- Codegen com stubs para novos constructs e implementacao de bitwise/shift em expressoes.
- Testes de lexer e parser ampliados para novos tokens e sintaxes.
- CI/CD de release ajustado para Linux/Windows/macOS com melhor deteccao de LLVM.

### Fixed
- Workflow Windows: evita falha por tentar instalar downgrade do LLVM quando versao mais nova ja existe.
- Workflow Linux: define `LLVM_DIR` explicitamente para evitar detectar LLVM antigo no runner.
- CMake: selecao de backend LLVM por arquitetura (x86_64 vs arm64).
- Codegen: compatibilidade com LLVM 15/17+ em `Reloc::Model` (`llvm::Optional` vs `std::optional`).

## [1.0.2] - 2026-04-01
### Added
- Pipeline de release automatica com binarios para Linux, Windows e macOS.

## [1.0.1] - 2026-04-01
### Changed
- README simplificado para instalacao e execucao rapida.

## [1.0.0] - 2026-04-01
### Added
- Primeira versao estavel da linguagem OFS (lexer, parser, semantica, codegen LLVM, runtime e exemplos).
