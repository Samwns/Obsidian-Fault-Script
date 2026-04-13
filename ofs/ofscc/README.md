# OFS Self-Hosted Compiler (ofscc)

Compilador completo escrito em OFS e usado como caminho principal do projeto.

Estado atual: o driver `ofscc` usa bridge por variaveis de ambiente (`OFSCC_INPUT`, `OFSCC_OUTPUT`, `OFSCC_MODE`, `OFSCC_OPT`) e ainda gera C para `gcc/clang`.

## Arquitetura

- chars.ofs
- tokens.ofs
- lexer.ofs
- nodes.ofs
- ast.ofs
- symbols.ofs
- parser.ofs
- typeck.ofs
- codegen.ofs
- fileio.ofs
- ofscc.ofs

## Pipeline

input.ofs -> lexer -> parser -> typeck -> codegen C -> gcc/clang -> binario

## Build

```bash
ofs build ofs/ofscc/ofscc.ofs -o ofscc
```

## Uso atual do driver

```bash
OFSCC_INPUT=ofs/examples/hello.ofs OFSCC_OUTPUT=hello ./ofscc
./hello
```

## Bootstrap e determinismo

```bash
bash ofs/ofscc/test_bootstrap.sh [bootstrap_compiler]
```

Padrao de bootstrap_compiler: ofs

O bootstrap atual ainda exige um compilador OFS anterior e um compilador C externo para fechar a cadeia.

## Testes

```bash
ofs build ofs/ofscc/test_lexer_units.ofs -o test_lex
./test_lex

ofs build ofs/ofscc/test_lexer_parser.ofs -o test_parse
./test_parse
```

## Distribuicao

Instaladores sao gerados pela propria linguagem em:

- packaging/installer_generator.ofs
- saida em packaging/generated/
