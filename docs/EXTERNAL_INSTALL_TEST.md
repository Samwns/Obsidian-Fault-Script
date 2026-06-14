# Teste de instalacao externa

Data do teste: 2026-06-13

Comando usado como usuario externo:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script /tmp/ofs-external-test
cd /tmp/ofs-external-test
bash ofscc/scripts/bootstrap.sh
```

## Resultado encontrado

O clone publico funcionou, mas o bootstrap falhou antes desta correcao porque chamava:

```bash
dist/ofscc build ofs/ofscc/ofscc.ofs -o ofscc_fresh -O3
```

O binario `dist/ofscc` publicado nao usa subcomandos; ele aceita flags diretas ou variaveis de ambiente:

```bash
OFSCC_INPUT=arquivo.ofs OFSCC_MODE=ir OFSCC_C_OUT=arquivo.ll dist/ofscc
```

## Correcao aplicada

`ofscc/scripts/bootstrap.sh` agora e o ponto de entrada oficial. Ele usa o fluxo self-hosted rapido que compila programas OFS gerando LLVM IR com `dist/ofscc` e linkando com `clang` + `dist/libofs_runtime.a`, seguindo a mesma estrategia do wrapper `ofs`.

O nome antigo `bootstrap-minimal.sh` continua disponivel por compatibilidade, mas a documentacao nova aponta para `bootstrap.sh`.

Se o ambiente encerrar a recompilacao direta do compilador OFS, o bootstrap usa `dist/ofscc.ll` como fallback versionado. Esse e o mesmo artefato-base usado pelas releases oficiais.

## Limites que continuam reais

- O bootstrap externo depende de `clang`, `ar` e `curl`.
- Windows precisa do script PowerShell e LLVM no PATH.
- A biblioteca `window` usa backend Win32/GDI no Windows e X11 dinamico no Linux; sem display, entra em modo headless para CI.
