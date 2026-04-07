# OFS Bare-metal/Freestanding Profile

O perfil bare-metal/freestanding permite compilar programas OFS sem dependência de libc, com entrada customizada e suporte a ambientes sem sistema operacional.

## Como ativar

1. Gere o build do compilador OFS com o perfil bare-metal:

   ```sh
   cd ofs
   cmake -B build -DOFS_FREESTANDING=ON
   cmake --build build
   ```

2. Compile seu programa normalmente:

   ```sh
   ./build/ofs check examples/baremetal_minimal.ofs
   ./build/ofs run examples/baremetal_minimal.ofs
   ```

## Exemplo mínimo

Veja `ofs/examples/baremetal_minimal.ofs` para um exemplo de programa sem dependência de libc.

## Limitações atuais
- Ainda depende do runtime mínimo da OFS.
- Não há suporte a linker script customizado ou inicialização de hardware.
- Entrada customizada (`_start`) ainda não está disponível.

## Próximos passos
- Suporte a linker script customizado.
- Permitir especificar ponto de entrada (`_start`).
- Exemplo de inicialização bare-metal real.
