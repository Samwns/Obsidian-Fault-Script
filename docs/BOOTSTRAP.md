# Guia de Bootstrap e Auto-Hospedagem do OFS

O compilador OFS é completamente auto-hospedado (*self-hosted*) e determinístico. O processo de bootstrap compila o compilador utilizando a si próprio, sem depender de ferramentas C++ ou runtimes externos.

---

## Início Rápido

Para realizar o bootstrap do compilador nativo no repositório:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

Após o bootstrap, o binário do compilador estará disponível em `ofs/dist/ofs` (ou `ofs/dist/ofscc`).

Para compilar um programa:

```bash
ofs/dist/ofs build ofs/examples/hello.ofs -o hello
./hello
```

---

## O que é Bootstrap?

O processo de **bootstrap** ocorre quando um compilador é capaz de compilar o seu próprio código-fonte. Esse marco valida formalmente a expressividade sintática, a maturidade da biblioteca padrão e a completude do sistema de tipos da linguagem.

- **Lisp** (1962): Primeira linguagem auto-hospedada.
- **C** (1972): Bootstrapped e estabelecido como padrão de sistemas.
- **Rust** (2011): Compilador escrito em Rust.
- **Go** (2015): Compilador migrado inteiramente para Go.
- **OFS** (2024–2026): Compilador e runtime nativo auto-hospedados.

---

## Etapas do Processo

### Fase 1: Compilador Inicial
O diretório `ofs/dist/` contém o binário base do compilador (`ofscc` / `ofs`), pronto para inicializar o ciclo de construção.

### Fase 2: Recompilação
O script compila os arquivos do compilador (`ofs/ofscc/*.ofs`) utilizando o binário existente, gerando um novo executável compilador intermediário.

### Fase 3: Validação de Determinismo
O novo compilador compila novamente o código-fonte do compilador. A saída binária da passagem 2 é comparada byte a byte com a passagem 3 (`v2 === v3`). Caso os checksums coincidam perfeitamente, o determinismo da compilação está garantido.

---

## Execução Detalhada

### 1. Bootstrap Padrão

```bash
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

**Saída esperada**:
```text
[OK] ofs/dist/ofscc criado com sucesso
[OK] Checksum verificado
[OK] Determinismo validado
```

### 2. Bootstrap com Validação Completa de Determinismo

Para executar o ciclo triplo completo de validação:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --validate
```

O script compilará:
1. `ofs/dist/ofscc` (v1)
2. `ofs/dist/ofscc` (v2, gerado por v1)
3. `ofs/dist/ofscc` (v3, gerado por v2)
4. Verificação estrita: `cmp -s v2 v3`

---

## Resolução de Problemas Comuns

### 1. Permissão negada ao executar o script

```bash
chmod +x ofs/bootstrap/scripts/bootstrap-minimal.sh
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

### 2. Permissão de execução no binário gerado

```bash
chmod +x ofs/dist/ofs ofs/dist/ofscc
```

### 3. Divergência na verificação de determinismo

Se `v2 != v3`, execute com a opção `--debug` para registrar o log de diferenças:

```bash
bash ofs/bootstrap/scripts/bootstrap-minimal.sh --debug
```

---

## Arquitetura do Pipeline

```text
Código-fonte OFS (.ofs)
  │
  ▼
[Compilador OFS nativo: ofs/dist/ofscc]
  │ Análise léxica e sintática (lexer / parser)
  │ Verificação estática de tipos
  │ Geração de LLVM IR nativo (.ll)
  ▼
[Compilador estático llc]
  │ Emissão de código de máquina objeto (.o)
  ▼
[Linker nativo ld / ld.lld]
  │ Ligação com o runtime nativo (magma.o)
  ▼
Executável nativo final (ELF no Linux, Mach-O no macOS, PE32+ no Windows)
```

---

## Estrutura dos Componentes

- `ofs/dist/ofs` / `ofs/dist/ofscc`: Binário executável do compilador.
- `ofs/dist/magma.o`: Runtime nativo compilado em objeto de máquina.
- `ofs/ofscc/`: Código-fonte em OFS do compilador (lexer, parser, type checker, gerador LLVM).
- `ofs/stdlib/`: Módulos da biblioteca padrão da linguagem.
- `ofs/stdlib/runtime/`: Código-fonte do runtime nativo (Stack Magma).
