# Teste de instalacao externa

Data do teste: 2026-06-13

Comando usado como usuario externo:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script /tmp/ofs-external-test
cd /tmp/ofs-external-test
bash ofscc/scripts/bootstrap-minimal.sh
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

`ofscc/scripts/bootstrap-minimal.sh` agora compila programas OFS gerando LLVM IR com `dist/ofscc` e linkando com `clang` + `dist/libofs_runtime.a`, seguindo a mesma estrategia do wrapper `ofs`.

## Limites que continuam reais

- O bootstrap externo depende de `clang`, `ar` e `curl`.
- Windows precisa do script PowerShell e LLVM no PATH.
- A biblioteca `window` compila, mas a janela real ainda depende de implementar backend nativo no runtime.
