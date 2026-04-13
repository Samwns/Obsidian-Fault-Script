# OFS Jornada Iniciante

Guia direto para quem quer aprender OFS por etapas, sem pular do básico para o baixo nível cedo demais.

## Como usar

1. Copie um módulo por vez.
2. Rode com `ofs arquivo.ofs`.
3. Mude alguma coisa no exemplo.
4. Só avance quando entender o que saiu no terminal.

---

## Módulo 1: Primeiros valores

Objetivo:
- entender `core`, `forge`, `echo` e tipos básicos.

```ofs
core main() {
    forge nome: obsidian = "Ana"
    forge idade: stone = 19
    forge peso: crystal = 55.7
    forge ativo: bool = true

    echo(nome)
    echo(idade)
    echo(peso)
    echo(ativo)
}
```

Mini desafio:
- adicione cidade e profissão.

---

## Módulo 2: Decisão e repetição

Objetivo:
- aprender `if`, `while` e `cycle`.

```ofs
core main() {
    forge energia: stone = 3

    while (energia > 0) {
        echo(energia)
        energia -= 1
    }

    cycle (forge i = 0; i < 3; i++) {
        echo(i)
    }

    if (energia == 0) {
        echo("recarregar")
    }
}
```

Mini desafio:
- conte de 10 até 1 e depois mostre `decolar`.

---

## Módulo 3: Funções

Objetivo:
- criar lógica reutilizável com `vein`.

```ofs
vein dobro(n: stone) -> stone {
    return n * 2
}

core main() {
    echo(dobro(21))
}
```

Mini desafio:
- crie `media(a, b)`.

---

## Módulo 4: Tipos pequenos

Objetivo:
- entender por que `u8` e `u32` existem.

```ofs
core main() {
    forge r: u8 = 255
    forge g: u8 = 128
    forge b: u8 = 0

    forge pixel: u32 = (r as u32 << 16) | (g as u32 << 8) | (b as u32)
    echo(pixel)
}
```

Mini desafio:
- extraia o canal vermelho de volta com `as u8`.

---

## Módulo 5: `monolith` e `impl`

Objetivo:
- modelar dados e colocar comportamento no tipo.

```ofs
monolith Rect {
    w: stone
    h: stone
}

impl Rect {
    vein area(self) -> stone {
        return self.w * self.h
    }
}

core main() {
    forge r: Rect
    r.w = 10
    r.h = 20
    echo(r.area())
}
```

Mini desafio:
- adicione um método `perimeter`.

---

## Módulo 6: `namespace`

Objetivo:
- organizar funções sem poluir o escopo global.

```ofs
namespace mathx {
    vein square(x: stone) -> stone {
        return x * x
    }
}

core main() {
    echo(mathx.square(4))
}
```

Mini desafio:
- adicione `cube`.

---

## Módulo 7: `attach`

Objetivo:
- reaproveitar código OFS de outro arquivo.

Arquivo `minha_lib.ofs`:

```ofs
vein saudacao() -> obsidian {
    return "oi"
}
```

Arquivo principal:

```ofs
attach {F:minha_lib.ofs}

core main() {
    echo(saudacao())
}
```

Mini desafio:
- mova duas funções para outro arquivo e importe com `attach {F:...}`.

---

## Módulo 8: Função como valor

Objetivo:
- usar lambda e passar função como argumento.

```ofs
vein aplicar(x: stone, fn: vein(stone) -> stone) -> stone {
    return fn(x)
}

core main() {
    forge dobrar = vein(n: stone) -> stone {
        return n * 2
    }

    echo(aplicar(10, dobrar))
}
```

Mini desafio:
- troque a lambda por uma que eleva ao quadrado.

---

## Módulo 9: Recursos modernos

Objetivo:
- conhecer recursos úteis fora do fluxo básico.

```ofs
const limite: stone = 10
strata Status { Idle, Running, Failed }

core main() {
    match limite {
        case 10: { echo("dez") }
        default: { echo("outro") }
    }
}
```

Mini desafio:
- troque o `match` para testar dois valores.

---

## Módulo 10: Quando avançar para o baixo nível

Só entre nisso quando já estiver confortável com os módulos anteriores.

Tópicos:
- `rift vein`
- `bedrock`
- `fracture`
- `abyss`
- `window`

Sugestão:
- veja `ofs/examples/showcase.ofs`
- veja `ofs/examples/attach_file_demo.ofs`
- leia `docs/LANGUAGE_REFERENCE.md`

---

## Próximos passos

1. Ler [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md)
2. Ler [docs/LANGUAGE_REFERENCE.md](docs/LANGUAGE_REFERENCE.md)
3. Rodar exemplos em `ofs/examples/`
4. Testar pacotes em [packages/README.md](packages/README.md)
