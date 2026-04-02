# OFS Jornada Iniciante

Guia rapido para quem esta aprendendo programacao e quer evoluir com OFS sem se perder.

## Como usar este guia

1. Escolha um modulo.
2. Copie o exemplo.
3. Rode no terminal com `ofs arquivo.ofs`.
4. Complete o mini desafio.

## Modulo 1: Fundamentos

Objetivo:
- Entender `core`, variaveis e tipos basicos.

Exemplo:

```ofs
core main() {
    forge nome: obsidian = "Ana"
    forge idade: stone = 19
    forge peso: crystal = 55.7
    echo(nome)
    echo(idade)
    echo(peso)
}
```

Mini desafio:
- Crie variaveis para cidade e ano atual e imprima tudo.

## Modulo 2: Decisao e repeticao

Objetivo:
- Aprender `if`, `else`, `while` e `cycle`.

Exemplo:

```ofs
core main() {
    forge energia: stone = 3

    while (energia > 0) {
        echo(energia)
        energia -= 1
    }

    if (energia == 0) {
        echo("Recarregar")
    } else {
        echo("Continuar")
    }
}
```

Mini desafio:
- Conte de 10 ate 1 e mostre "decolar" no final.

## Modulo 3: Funcoes

Objetivo:
- Criar funcoes reutilizaveis com `vein`.

Exemplo:

```ofs
vein dobro(n: stone) -> stone {
    return n * 2
}

core main() {
    forge x: stone = 21
    echo(dobro(x))
}
```

Mini desafio:
- Crie uma funcao `media(a, b)` que retorna a media de dois valores.

## Modulo 4: Structs e organizacao

Objetivo:
- Modelar dados com `monolith`.

Exemplo:

```ofs
monolith Player {
    nome: obsidian
    hp: stone
}

core main() {
    forge p: Player
    p.nome = "Rex"
    p.hp = 100
    echo(p.nome)
    echo(p.hp)
}
```

Mini desafio:
- Adicione um campo `nivel` e mostre no terminal.

## Modulo 5: Nivel avancado

Objetivo:
- Conhecer recursos de baixo nivel quando ja estiver confortavel.

Topicos:
- `fracture` e `shard` (ponteiros seguros)
- `extern` (integracao com C)
- `attach` (reuso de modulos)

Sugestao:
- Use como referencia o arquivo `ofs/examples/showcase.ofs`.

## Proximos passos

1. Ler [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md)
2. Explorar [docs/LANGUAGE_REFERENCE.md](docs/LANGUAGE_REFERENCE.md)
3. Rodar exemplos em `ofs/examples/`

Aprender programacao fica mais facil quando voce faz pequenos ciclos: testar, errar, ajustar e testar de novo.
