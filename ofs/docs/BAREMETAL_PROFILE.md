# OFS Bare-Metal/Freestanding Profile (experimental)

Este exemplo mostra como compilar e rodar um programa OFS sem dependência de libc, usando o novo perfil `-DOFS_FREESTANDING=ON` no CMake.

## Como usar

1. Gere o build com o perfil bare-metal ativado:

   ```sh
   cd ofs
   cmake -B build -DOFS_FREESTANDING=ON
   cmake --build build
   ```

2. Compile um programa OFS normalmente. O backend irá gerar código sem dependências de libc (exceto o mínimo do runtime OFS).

3. Para targets realmente bare-metal, será necessário fornecer um linker script e código de startup específico para a plataforma.

## Limitações
- O runtime ainda pode depender de funções mínimas do sistema/hardware.
- Entrada customizada (ex: `_start` ao invés de `main`) ainda não está implementada.
- Não há suporte a inicialização de hardware, interrupções, etc.

## Próximos passos
- Adicionar suporte a linker script customizado.
- Permitir especificar ponto de entrada (`_start`) via OFS.
- Exemplo de programa "hello world" bare-metal (em assembly ou C mínimo).
