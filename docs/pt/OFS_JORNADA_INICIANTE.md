# OFS — Jornada do Iniciante

Guia prático para aprender Obsidian Fault Script (OFS) por etapas graduais, do básico até a criação de componentes e módulos.

---

## Como Utilizar este Guia

1. Estude um módulo de cada vez.
2. Execute o código no terminal com `ofs run arquivo.ofs` (ou compile com `ofs build arquivo.ofs -o programa`).
3. Modifique os valores para testar o comportamento.
4. Avance apenas após compreender os resultados impressos no terminal.

---

## Módulo 1: Primeiros Valores e Variáveis

**Objetivo**: Compreender a função `core main()`, a declaração de variáveis com `forge`, a saída no terminal com `echo()` e os tipos primitivos fundamentais.

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

**Desafio**: Adicione variáveis para armazenar uma cidade (`obsidian`) e uma taxa (`crystal`).

---

## Módulo 2: Decisão e Laços de Repetição

**Objetivo**: Utilizar condicionais (`if` / `else`) e laços de repetição (`while`).

```ofs
core main() {
    forge energia: stone = 3

    while (energia > 0) {
        echo(energia)
        energia = energia - 1
    }

    if (energia == 0) {
        echo("Recarga necessária")
    }
}
```

**Desafio**: Crie um laço que realize uma contagem regressiva de 5 a 1 e exiba "Decolagem".

---

## Módulo 3: Funções (`vein`)

**Objetivo**: Estruturar e reutilizar código com funções declaradas pela palavra-chave `vein`.

```ofs
vein dobro(n: stone) -> stone {
    return n * 2
}

vein somar(a: stone, b: stone) -> stone {
    return a + b
}

core main() {
    echo(dobro(21))
    echo(somar(10, 32))
}
```

**Desafio**: Escreva uma função `media(a: stone, b: stone) -> stone` que retorne a média aritmética inteira de dois números.

---

## Módulo 4: Tipos Inteiros de Largura Fixa

**Objetivo**: Manipulação de bytes e inteiros com sinal e sem sinal (`u8`, `u16`, `u32`, `i8`, `i32`).

```ofs
core main() {
    forge canal_r: u8 = 255
    forge canal_g: u8 = 128
    forge canal_b: u8 = 0

    forge pixel: u32 = (canal_r as u32 << 16) | (canal_g as u32 << 8) | (canal_b as u32)
    echo(pixel)
}
```

**Desafio**: Isole o canal verde (`canal_g`) de volta a partir de `pixel` utilizando deslocamento de bits e máscara (`& 0xFF`).

---

## Módulo 5: Estruturas de Dados (`monolith`) e Métodos (`impl`)

**Objetivo**: Modelar tipos compostos e encapsular operações com `monolith` e `impl`.

```ofs
monolith Retangulo {
    largura: stone
    altura: stone
}

impl Retangulo {
    vein area(self) -> stone {
        return self.largura * self.altura
    }

    vein perimetro(self) -> stone {
        return (self.largura + self.altura) * 2
    }
}

core main() {
    forge r: Retangulo
    r.largura = 10
    r.altura = 20

    echo(r.area())
    echo(r.perimetro())
}
```

**Desafio**: Crie um método `eh_quadrado(self) -> bool` que retorne verdadeiro caso largura e altura sejam idênticas.

---

## Módulo 6: Espaços de Nomes (`namespace`)

**Objetivo**: Agrupar funções e constantes relacionadas para manter o escopo global organizado.

```ofs
namespace calculo {
    vein quadrado(x: stone) -> stone {
        return x * x
    }

    vein cubo(x: stone) -> stone {
        return x * x * x
    }
}

core main() {
    echo(calculo.quadrado(5))
    echo(calculo.cubo(3))
}
```

---

## Módulo 7: Módulos Externos com `attach`

**Objetivo**: Dividir código em múltiplos arquivos e importar funções.

Arquivo auxiliar `auxiliar.ofs`:
```ofs
vein saudacao(nome: obsidian) -> obsidian {
    return "Olá, " + nome + "!"
}
```

Arquivo principal `main.ofs`:
```ofs
attach {F:./auxiliar.ofs}

core main() {
    echo(saudacao("OFS"))
}
```

---

## Módulo 8: Funções como Valores e Lambdas

**Objetivo**: Passar funções como parâmetros para outras rotinas.

```ofs
vein aplicar(x: stone, operacao: vein(stone) -> stone) -> stone {
    return operacao(x)
}

core main() {
    forge triplo = vein(n: stone) -> stone {
        return n * 3
    }

    echo(aplicar(7, triplo))
}
```

---

## Módulo 9: Seleção Múltipla (`match`) e Constantes

**Objetivo**: Simplificar árvores condicionais através de correspondência de padrões e constantes.

```ofs
const SUCESSO: stone = 200
const NAO_ENCONTRADO: stone = 404

core main() {
    forge status = 200

    match status {
        case SUCESSO: {
            echo("Requisição atendida com êxito")
        }
        case NAO_ENCONTRADO: {
            echo("Recurso não encontrado")
        }
        default: {
            echo("Código não mapeado")
        }
    }
}
```

---

## Próximos Passos

1. Leia o [Guia de Início](../GETTING_STARTED.md)
2. Consulte a [Referência Completa da Linguagem](../LANGUAGE_REFERENCE.md)
3. Explore os exemplos práticos em `ofs/examples/`
