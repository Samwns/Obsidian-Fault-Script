# Referencia OES

OES, Obsidian Effect Scripts, e a linguagem visual e de efeitos da OFS.

Voce escreve `.oes`, o comando `ofs oes` compila para CSS, e o navegador recebe um artefato padrao. O fonte de design continua sendo OES.

## Exemplo minimo

```oes
gem accent "#a978ff"
gem surface "#120d1f"

paint .button
  fill "$accent"
  ink "#13071f"
  curve "8px"
  transition "transform .2s ease"

veil "(max-width: 700px)"
  paint .grid
    grid-cols "1fr"

pulse "enter"
  rise
    opacity "0"
  rest
    opacity "1"
```

## Forma tipada e escopo explicito

A sintaxe indentada atual continua valida. A forma tipada permite declarar a
categoria da gema e agrupar contextos responsivos dentro do bloco de pintura.

```oes
gem accent: Color = "#a978ff"
gem spacing: Size = "16px"

paint .button
  fill(accent)
  curve("8px")
  transition("transform .2s ease")

  veil("(max-width: 700px)") {
    grid-cols("1fr")
    space(spacing)
  }
end
```

## Regras de sintaxe

- `gem` cria tokens de design.
- `gem nome: Tipo = "valor"` cria tokens tipados na forma nova.
- `paint` abre um bloco visual para um seletor.
- Declaracoes ficam indentadas dentro do bloco.
- Blocos com `{}` e fechamento `end` podem ser usados para escopo explicito.
- `$nome` usa um token como `var(--nome)`.
- `veil` cria regras responsivas.
- `guard` cria regras de suporte.
- `pulse` cria animacoes.

## Formas principais

| Forma | Finalidade |
|---|---|
| `gem nome "valor"` | Token de design |
| `paint seletor` | Bloco de regra visual |
| `fill` | Fundo |
| `ink` | Cor de texto |
| `curve` | Raio de borda |
| `space` | Espacamento/gap |
| `edge` | Borda |
| `flow` | Display |
| `grid-cols` | Colunas de grid |
| `grid-rows` | Linhas de grid |
| `type-size` | Tamanho de fonte |
| `veil "query"` | Media query |
| `guard "query"` | Feature query |
| `strata "nome"` | Cascade layer |
| `pulse "nome"` | Keyframes |
| `rise`, `rest`, `at50` | Frames de animacao |
| `raw "css"` | CSS bruto explicito |

## Compatibilidade

OES aceita nomes de propriedades CSS quando nao existe alias proprio. As formas antigas `token`, seletores planos, `media`, `supports`, `layer`, `motion`, `from` e `to` continuam aceitas para migracao. O modo Studio deve exportar OES revisavel e nao gravar inline styles no codigo fonte original.

## Compilacao

```bash
ofs oes theme.oes -o theme.css
```
