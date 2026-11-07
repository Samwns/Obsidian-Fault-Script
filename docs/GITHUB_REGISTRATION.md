# Reconhecimento no GitHub

## Reconhecimento no repositório

Este repositório usa `.gitattributes` para que o GitHub trate arquivos `.ofs` como OFS:

```gitattributes
*.ofs linguist-language=OFS
*.ofs linguist-detectable=true
*.odl linguist-language=ODL
*.odl linguist-detectable=true
*.oes linguist-language=OES
*.oes linguist-detectable=true
```

Esse override vale imediatamente para repositórios que o incluem.

## Gramática de sintaxe

A gramática TextMate fica em:

- `src/grammar/syntaxes/ofs.tmLanguage.json`
- `src/vscode-extension/syntaxes/ofs.tmLanguage.json`

O escopo é `source.ofs` e a gramática é licenciada sob MIT.

## Registro global no GitHub Linguist

O GitHub Linguist exige que uma nova linguagem inclua:

1. Uma entrada em `languages.yml` sem `language_id` inventado à mão.
2. Uma gramática TextMate mantida separadamente, com licença aceitável.
3. Amostras reais e representativas, não arquivos "olá, mundo" de tutorial.
4. Um ID gerado pelo `script/update-ids` do Linguist.
5. Evidência, na busca de código do GitHub, de uso em projetos reais.

Em 14 de junho de 2026, o Linguist exige pelo menos 2.000 arquivos `.ofs`
indexados no último ano, excluindo forks, com distribuição razoável entre
repositórios e usuários públicos. Linguagens muito novas ou de hobby abaixo
desse limite não são aceitas.

Uma busca de código no GitHub em 14 de junho de 2026 retornou 364 arquivos
`.ofs` indexados no total e 133 excluindo `user:Samwns`. É um bom avanço, mas
ainda abaixo do critério de aceitação do Linguist.

Os metadados preparados, o plano de amostras, a URL da gramática e o critério de
aceitação estão em `docs/linguist-pr.md`.

## Situação atual

- Reconhecimento local de `.ofs`, `.odl` e `.oes` no repositório: concluído.
- Gramática TextMate: concluída.
- Realce de sintaxe no VS Code: concluído.
- Arquivos de contribuição ao Linguist: preparados.
- Repositório público da gramática: exige um token do GitHub com permissão para criar repositórios.
- Fork do Linguist: exige um token do GitHub com permissão para criar forks.
- Pull request global: aguarda o limite de uso público do Linguist.
