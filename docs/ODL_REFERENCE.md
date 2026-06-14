# Referencia ODL

ODL, Obsidian Document Language, e a linguagem estrutural de documentos da OFS.

Voce escreve `.odl`, o comando `ofs odl` compila para HTML, e o navegador recebe um artefato padrao. O fonte continua sendo ODL; HTML e apenas a saida de hospedagem.

## Exemplo minimo

```odl
page "Meu site" "pt"
skin "site.css"
flow
  deck.topbar
    title1 "Meu site"
    link.button "Docs" "#docs"
  stage#content
    band.docs
      mark "## Conteudo\nTexto em **Markdown**."
      raw "<small>HTML bruto quando precisar.</small>"
  spark "app.js"
```

## Forma tipada e escopo explicito

A sintaxe indentada atual continua valida. A forma com chaves, chamadas e
assinaturas tipadas e uma evolucao compativel para componentes com contrato
mais forte.

```odl
page "OFS Web" "pt"
  skin("theme.oes")

  flow {
    deck.topbar {
      title1("OFS Web")
      link.button("Docs", "#docs")
    }

    stage#content {
      band.docs {
        use Notice(title: "HMR Ativo", status: "success")
      }
    }
  }
end

component Notice(title: String, status: String)
  tile.notice(data-status: status) {
    title2(title)
    text("Componente injetado com tipagem forte e escopo fechado.")
  }
end
```

## Regras de sintaxe

- A indentacao abre e fecha elementos.
- Blocos com `{}` e fechamento `end` podem ser usados na forma tipada.
- Argumentos tipados usam `nome: Tipo` em assinaturas de componentes.
- O nome pode receber classe com `.classe`.
- O nome pode receber id com `#id`.
- Texto entre aspas e escapado por padrao.
- `raw` existe para interoperabilidade explicita com HTML.
- `mark` aceita Markdown nativo.

## Formas principais

| Forma | Finalidade |
|---|---|
| `page "titulo" "idioma"` | Metadados do documento |
| `skin "arquivo.css"` | Link de stylesheet |
| `flow` | Inicio do corpo |
| `deck` | Navegacao, gerada como `nav` |
| `stage` | Area principal, gerada como `main` |
| `band` | Secao semantica, gerada como `section` |
| `tile` | Card/artigo, gerado como `article` |
| `title1`, `title2`, `title3` | Titulos |
| `text` | Paragrafo |
| `link` | Ancora |
| `asset` | Imagem |
| `mark` | Markdown |
| `raw` | HTML bruto |
| `wire` | Script classico |
| `spark` | Modulo JavaScript |
| `pulse` | JavaScript inline |
| `server` | Bloco compativel com template PHP |

## Compatibilidade

As formas antigas `document`, `style`, `body`, `markdown`, `html`, `script`, `module`, `js` e `php` continuam aceitas para migracao gradual. Se a forma tipada ficar menos rigorosa que o type checker OFS atual, a tipagem atual deve ser mantida.

Projetos novos devem preferir o vocabulario ODL.

## Compilacao

```bash
ofs odl page.odl -o index.html
```
