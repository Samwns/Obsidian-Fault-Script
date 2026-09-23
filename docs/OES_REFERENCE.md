# Referência OES

OES (Obsidian Effect Scripts) é a linguagem de efeitos, estilos visuais e movimento do ecossistema OFS.

O desenvolvedor escreve arquivos `.oes`, o comando `ofs oes` compila as regras para CSS padrão, e o navegador consome a folha de estilos gerada. O código OES é a fonte de verdade do sistema de design.

---

## Exemplo Mínimo

```oes
gem primario "#a978ff"
gem superficie "#120d1f"

paint .botao
  fill "$primario"
  ink "#13071f"
  curve "8px"
  transition "transform .2s ease"

veil "(max-width: 700px)"
  paint .grade
    grid-cols "1fr"

pulse "surgir"
  rise
    opacity "0"
  rest
    opacity "1"
```

---

## Forma Tipada e Escopo Explícito

Além da sintaxe tradicional baseada em indentação, a forma tipada permite categorizar tokens e aninhar contextos responsivos dentro de regras de estilização:

```oes
gem destaque: Color = "#a978ff"
gem espacamento: Size = "16px"

paint .botao
  fill(destaque)
  curve("8px")
  transition("transform .2s ease")

  veil("(max-width: 700px)") {
    grid-cols("1fr")
    space(espacamento)
  }
end
```

---

## Regras Sintáticas

- A diretiva `gem` define tokens de design como variáveis CSS (`--nome: valor;`).
- `gem nome: Tipo = "valor"` declara tokens tipados na sintaxe explícita.
- A diretiva `paint` abre um bloco de estilização para um seletor específico.
- Propriedades visuais são declaradas de forma indentada dentro do bloco seletor.
- Blocos delimitados por chaves `{}` e fechamento `end` podem ser usados para escopo léxico explícito.
- A interpolação `$nome` referencia um token via `var(--nome)`.
- `veil` declara consultas de mídia (*media queries*).
- `guard` declara regras condicionais de suporte (*feature queries* `@supports`).
- `strata` define camadas de cascata (*cascade layers* `@layer`).
- `pulse` define sequências de animação (*keyframes*).

---

## Diretivas e Propriedades Principais

| Diretiva / Propriedade | Equivalente CSS | Finalidade |
|---|---|---|
| `gem nome "valor"` | `:root { --nome: valor; }` | Definição de token de design |
| `paint seletor` | `seletor { ... }` | Bloco de estilo para o seletor |
| `fill "valor"` | `background: valor;` | Cor ou imagem de fundo |
| `ink "valor"` | `color: valor;` | Cor tipográfica do texto |
| `curve "valor"` | `border-radius: valor;` | Arredondamento de bordas |
| `space "valor"` | `gap: valor;` | Espaçamento em layouts flex/grid |
| `edge "valor"` | `border: valor;` | Definição de bordas |
| `flow "valor"` | `display: valor;` | Modo de exibição e layout |
| `grid-cols "valor"` | `grid-template-columns: valor;` | Definição de colunas em grid |
| `grid-rows "valor"` | `grid-template-rows: valor;` | Definição de linhas em grid |
| `type-size "valor"` | `font-size: valor;` | Tamanho da tipografia |
| `veil "query"` | `@media query { ... }` | Regra de responsividade por tela |
| `guard "query"` | `@supports query { ... }` | Regra condicional de suporte a recursos |
| `strata "nome"` | `@layer nome { ... }` | Definição de camada de cascata |
| `pulse "nome"` | `@keyframes nome { ... }` | Linha do tempo de animação |
| `rise` / `rest` | `from { ... }` / `to { ... }` | Pontos inicial e final de animação |
| `at50` / `atNN` | `50% { ... }` | Etapa percentual em linha de tempo |
| `raw "css"` | Nenhum invólucro | Emissão de CSS bruto sem alterações |

---

## Compatibilidade

O analisador OES reconhece propriedades CSS nativas quando não há um atalho próprio configurado. Palavras-chave legadas como `token`, seletores planos, `media`, `supports`, `layer`, `motion`, `from` e `to` continuam sendo suportadas para migração gradual.

---

## Compilação

Para compilar um arquivo OES para folha de estilos CSS:

```bash
ofs oes tema.oes -o public/tema.css
```
