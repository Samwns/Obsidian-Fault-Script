# OFS Bare-metal/Freestanding Profile

O perfil bare-metal/freestanding permite compilar programas OFS sem dependência de libc, com entrada customizada e suporte a ambientes sem sistema operacional.

## Como ativar

1. Prepare o compilador OFS nativo:

   ```bash
   # Do diretório raiz do repositório
   cd ..
   bash ofscc/scripts/bootstrap-minimal.sh
   cd ofs
   ```

2. Compile seu programa normalmente:

   ```bash
   ../dist/ofscc check examples/baremetal_minimal.ofs
   ../dist/ofscc build examples/baremetal_minimal.ofs -o baremetal_minimal
   ./baremetal_minimal
   ```

## Exemplo mínimo

Veja [examples/baremetal_minimal.ofs](examples/baremetal_minimal.ofs) para um exemplo de programa sem dependência de libc.

## Limitações atuais
- Ainda depende do runtime mínimo da OFS.
- Não há suporte a linker script customizado ou inicialização de hardware.
- Entrada customizada (`_start`) ainda não está disponível.

## Próximos passos
- Suporte a linker script customizado.
- Permitir especificar ponto de entrada (`_start`).
- Exemplo de inicialização bare-metal real.
