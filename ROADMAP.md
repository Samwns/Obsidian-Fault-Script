# OFS Roadmap - Verificacao Final

Versao alvo: v2.0.0
Status: em verificacao
Atualizado em: 2026-04-13

## Gate de conclusao

O roadmap esta completo quando:
- [ ] bootstrap deterministico validado em Linux/Windows/macOS
- [ ] `ofscc` aceita CLI compativel ou wrappers oficiais estaveis
- [ ] pipeline de release nao depende de baixar o bootstrap da latest release
- [ ] instaladores sao gerados via OFS no fluxo validado de release
- [x] codigo C++ legado e removido do repositorio

Gate automatico:
- [x] .github/workflows/roadmap-gate.yml

## Fase A - Auto-hosting proof
- [x] compilador em OFS implementado
- [x] pipeline lexer/parser/typeck/codegen funcional
- [x] bootstrap inicial documentado

## Fase B - Refinement
- [x] bridge de configuracao por ambiente no ofscc
- [x] suporte base de impl/namespace/strata
- [x] suporte base de tremor/catch
- [x] flags de otimizacao de compilacao
- [~] CI self-hosted alinhada com o bridge atual do ofscc
- [~] documentacao alinhada com a interface atual do compilador

## Fase C - Backend evolution
- [x] base de evolucao de backend definida no compilador OFS
- [x] caminho de release desacoplado de C++
- [ ] caminho de release desacoplado de bootstrap externo da latest release
- [x] arquitetura preparada para extensao futura de backend
- [ ] backend sem gcc/clang externo

## Fase D - Remocao de C++
- [x] removido ofs/src
- [x] removido ofs/tests em C++
- [x] removido ofs/CMakeLists.txt
- [~] workflows de build/release migrados para self-hosted e ainda em verificacao

## Fase E - Distribution and tooling
- [x] geracao de instaladores via packaging/installer_generator.ofs
- [x] assets Linux gerados (install.sh/control)
- [x] assets Windows gerados (NSIS script)
- [x] assets macOS gerados (formula/script)
- [ ] release self-hosted publicada e verificada com artefatos gerados por OFS

## Entregaveis finais
- [~] compilador self-hosted como caminho principal
- [x] instaladores gerados pela propria linguagem
- [x] sem dependencia de build C++ no repositorio
- [ ] sem dependencia externa de bootstrap e de compilador C
- [ ] roadmap fechado em 100%
