# OFS Bootstrap Guide (Self-Hosted)

OFS usa bootstrap self-hosted. O repositorio nao depende mais de codigo C++.

## Fluxo

1. obter um compilador OFS bootstrap (release anterior)
2. compilar ofs/ofscc/ofscc.ofs para gerar ofscc_v1
3. compilar ofscc.ofs novamente com ofscc_v1 para gerar ofscc_v2
4. compilar ofscc.ofs com ofscc_v2 para gerar ofscc_v3
5. validar determinismo (ofscc_v2 == ofscc_v3)

## Comandos

```bash
ofs build ofs/ofscc/ofscc.ofs -o ofscc_v1
./ofscc_v1 ofs/ofscc/ofscc.ofs -o ofscc_v2
./ofscc_v2 ofs/ofscc/ofscc.ofs -o ofscc_v3
cmp -s ofscc_v2 ofscc_v3 && echo "BOOTSTRAP OK"
```

## Script automatizado

```bash
bash ofs/ofscc/test_bootstrap.sh
```

## Resultado esperado

- binarios deterministas
- compilacao self-hosted estavel
- instaladores gerados por OFS via packaging/installer_generator.ofs
