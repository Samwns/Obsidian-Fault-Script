# Golden files — Reporter OFS

Esta árvore ancora as saídas do reporter (`Plain`, `Fancy`, `Json`, locale) em cenários-chave. Como os números de tempo (`ms`) e alguns tamanhos podem variar entre máquinas, os testes aqui são **baseados em padrões** (`grep -E`) e não em `diff` byte-a-byte. Arquivos `.txt` documentam o formato esperado visualmente; quem garante a não-regressão é o `run.sh`.

## Como rodar

```bash
bash tests/golden/run.sh
```

Precisa de `jq` (para validar NDJSON). Use `OFSCC_BIN=/caminho/para/ofscc` se o binário não estiver em `~/.local/bin/ofscc` nem em `/tmp/ofs-logs/ofscc-f5`.

## Convenção

- `plain/hello.txt`, `plain/twin_ok.txt`, `plain/stub_twin.txt`, `plain/typecheck_fail.txt`: formato Plain de referência.
- `fancy/hello_ansi.txt`: formato Fancy de referência (com códigos ANSI escritos como `<ESC>`).
- `json/hello.ndjson`, `json/twin_ok.ndjson`: exemplos de linhas NDJSON válidas.
- `locale/pt_check.txt`, `locale/es_check.txt`, `locale/en_fallback.txt`: saída Plain localizada.

## Atualizando

Se a saída Plain/Fancy mudar intencionalmente, regenere os `.txt` com:

```bash
OFS_UPDATE_GOLDEN=1 bash tests/golden/run.sh
```

Isso sobrescreve os `.txt` com a saída atual — faça num commit separado para revisão.
