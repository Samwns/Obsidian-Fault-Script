# OFS Bare-Metal/Freestanding Profile (experimental)

Este exemplo mostra como compilar e rodar um programa OFS sem dependência de libc, usando o compilador OFS nativo.

## Como usar

1. Prepare o compilador OFS nativo (do diretório raiz):

   ```bash
   bash ofscc/scripts/bootstrap-minimal.sh
   ```

2. Compile um programa OFS normalmente. O backend irá gerar código sem dependências de libc (exceto o mínimo do runtime OFS).

   ```bash
   dist/ofscc build ofs/examples/baremetal_minimal.ofs -o baremetal_minimal
   ./baremetal_minimal
   ```

3. Para targets realmente bare-metal, será necessário fornecer um linker script e código de startup específico para a plataforma.

## Limitações
- O runtime ainda pode depender de funções mínimas do sistema/hardware.
- Entrada customizada (ex: `_start` ao invés de `main`) ainda não está implementada.
- Não há suporte a inicialização de hardware, interrupções, etc.

## Próximos passos
- Adicionar suporte a linker script customizado.
- Permitir especificar ponto de entrada (`_start`) via OFS.
- Exemplo de programa "hello world" bare-metal (em assembly ou C mínimo).
