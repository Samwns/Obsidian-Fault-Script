# Referência OAL (v1)

OAL (Obsidian Android Language, `.oal`) descreve o que é **próprio do Android** num app OFS: o manifesto, as telas de toque, os estilos, as ações e o ciclo de vida. A lógica continua em `.ofs`. É irmã da ODL (documento → HTML), da OES (estilo → CSS) e da OLL (UI de desktop), e também faz o papel do CSS/OES no Android (bloco `style`).

```
app.oal ──oalc──► AndroidManifest.xml
                └► app_oal.ofs ──ofscc (aarch64)──► libapp.so ──► APK
```

- Compilador: `ofs/tools/oalc.ofs` (`OALC_INPUT`, `OALC_OUT_DIR`, `OALC_TARGET=android|desktop`).
- Runtime: `ofs/stdlib/oal_runtime.ofs` (núcleo neutro) + `oal_android.ofs` (backend). O usuário não importa a OAL: o arquivo gerado já liga o runtime e o `android.ofs`.
- Build: `ofs build --target android app.oal [-o app.apk] [--install] [--serial ID]` (ver [ANDROID.md](ANDROID.md)).

---

## Exemplo

```oal
// lógica em OFS: as duas formas abaixo têm o mesmo efeito
use "biblioteca.ofs"
// attach {F:biblioteca.ofs}

app "M.O.O.N" "com.fixedseat.moon"
  version "0.1.0"
  orientation any
  permission internet

style button.primario
  fill #8b6cff
  ink #101020
  curve 24
  pressed fill #5f45c8
  pressed ink #ffffff

screen biblioteca
  bar "M.O.O.N"
    button.icon "plugins" -> abrir_plugins
  title "Biblioteca"
  text biblioteca.resumo()
  button.primario "Importar jogo" icon=import -> importar
  list jogos from biblioteca.jogos
    item title=item.nome sub=item.motor -> abrir_jogo(item)
    empty "Nenhum jogo ainda."

screen plugins
  bar "Plugins"
    button.icon "back" -> @back
  list from biblioteca.plugins
    item title=item.nome sub=item.estado
    empty "Nenhum plugin registrado."

on back -> voltar
on pause -> quick_save
```

```ofs
// biblioteca.ofs
namespace biblioteca {
    vein jogos_count() -> stone { return 0 }
    vein jogos_nome(i: stone) -> obsidian { return "" }
    vein jogos_motor(i: stone) -> obsidian { return "" }
    vein resumo() -> obsidian { return "0 jogos instalados" }
}
vein abrir_plugins() -> void { oal.go(1) }
vein voltar() -> void { oal.back() }
```

---

## Regras gerais

- Uma declaração por linha; a hierarquia é dada pela **indentação** (2 espaços ou tab).
- Comentários: `// ...` ou `# ...` em linha própria.
- Textos entre aspas `"..."` (com `\"`, `\n`). Fora de aspas, um valor é uma **expressão OFS** (`biblioteca.resumo()`).
- `chave=valor` sem espaços em volta do `=` (`title=item.nome`, `corner=bottom_left`).
- Palavras em português também valem: `tela`, `barra`, `botao`, `texto`, `titulo`, `lista`, `grade`, `cartao`, `linha`, `vazio`, `retrato`, `paisagem`, `estilo`, `usa`, `ao`.
- Erros saem como `oalc: linha N: ...` e nada é gerado.

## Topo do arquivo

| Declaração | Efeito |
|---|---|
| `use "arquivo.ofs"` | inclui a lógica (caminho relativo ao `.oal`) |
| `attach {F:arquivo.ofs}` | igual ao `use`, com a sintaxe do `attach` do OFS |
| `attach {modulo}` | forma curta do OFS: `modulo.ofs` ao lado do `.oal`, senão da stdlib (hífen vira `_`) |
| `app "Nome" "pacote"` + propriedades | gera o `AndroidManifest.xml` |
| `style alvo` + propriedades | estilo (ver abaixo) |
| `screen nome` + widgets | uma tela; a primeira é a inicial |
| `on evento -> ação` | ciclo de vida |

### `app`

| Propriedade | Exemplo | Manifesto |
|---|---|---|
| `version "a.b.c"` | `version "0.1.0"` | `versionName`; `versionCode = a*10000 + b*100 + c` |
| `code N` | `code 7` | `versionCode` explícito |
| `icon "arquivo.png"` | | `@mipmap/ic_launcher` (só PNG; a CLI copia para `res/mipmap/`) |
| `orientation any\|portrait\|landscape` | | `screenOrientation` (`sensorLandscape` para paisagem) |
| `permission a b ...` | `permission internet vibrate` | `android.permission.INTERNET`… (`storage` = leitura + escrita; nome com ponto passa como está) |
| `fullscreen` | | tema `NoTitleBar.Fullscreen` |
| `assets "pasta"` | `assets "../android/assets"` | a pasta vai para `assets/` do APK (ler com `android.asset_read`) |

O nome da `.so` (`android.app.lib_name`) é o último segmento do pacote. A activity sempre declara `configChanges` de orientação/tamanho: girar **não** recria o app, só gera `on rotate`.

## Widgets (`screen`)

| Widget | Forma | Gera |
|---|---|---|
| `bar "Título"` | filhos: `button` | barra fixa no topo |
| `title "texto"` | | título |
| `text "texto"` / `text expr()` | | texto secundário |
| `button "rótulo" -> ação` | `icon=nome`, `.classe` | botão |
| `button.icon "nome" -> ação` | | botão só com ícone |
| `card "título" "sub" -> ação` | `title=`, `sub=` | cartão |
| `list [id] [from fonte]` | filhos: `item`, `empty` | lista |
| `grid [id] [from fonte] [columns=2] [landscape=4]` | filhos: `card`, `empty` | grade com colunas por orientação |
| `row` | filhos | widgets lado a lado |
| `portrait` / `landscape` | filhos | só aparecem naquela orientação |
| `fab "rótulo" -> ação` / `fab.icon "nome" -> ação` | `corner=` | botão flutuante (camada por cima, não rola) |
| `dpad -> ação(item)` | `corner=` | D-pad virtual (camada flutuante) |
| `diamond -> ação(item)` | `corner=`, `look=` | 4 botões de ação em losango (camada flutuante); `item` = botão tocado (`oal.SLOT_TOP/RIGHT/BOTTOM/LEFT`); `look` = `oal.PAD_KEYS` (teclas), `PAD_XBOX` (A B X Y), `PAD_PLAY` (✕ ○ □ △ em vetor, sem logotipos), `PAD_NINTENDO` (A B X Y espelhado) |
| `image expr` | `height=` (dp; 0 = proporcional) | imagem PNG (caminho) ajustada à largura |
| `canvas [full] draw=f touch=g` | `height=` | área desenhada pelo app (ver abaixo) |
| `tabs` | filhos: `tab "rótulo" icon= selected=expr -> ação` | abas / segmentos lado a lado |
| `slider "rótulo" value=expr min= max= -> ação(item)` | `icon=` | controle deslizante; `item` = novo valor |
| `colorpicker value=expr -> ação(item)` | | seletor de cor (toque): quadrado saturação/valor, barra de matiz, prévia e hex; `value`/`item` = `0xRRGGBB`; a ação roda enquanto arrasta |
| `toggle "rótulo" value=expr -> ação(item)` | `icon=` | interruptor; `item` = 1 (ligar) ou 0 |
| `progress "rótulo" value=expr` | | barra de progresso 0–100 |
| `field "dica" value=expr -> ação` | `icon=` | campo de texto (toque chama a ação) |
| `keyboard -> ação(item)` | | teclado na tela; `item` = código Unicode, 8 = apagar, 10 = confirmar. Camada hex (`oal.kb_layer = 2`): teclas 0-9, A-F, apagar, `#` (abc) e ↵. |

**Entrada de texto do sistema (`oal.edit_open`).** `oal.edit_open(título, texto, multilinha) -> bool` abre um Dialog Android com um `EditText` nativo (stdlib `android_input.ofs`, só JNI): teclado padrão do aparelho (Gboard), cursor, rolagem do texto, colar e autocompletar. Botões OK/Cancelar (`oal.edit_ok_label`/`edit_cancel_label`). O resultado chega no `on frame`: `oal.edit_state` 2 = confirmado, 3 = cancelado, com `oal.edit_text`; chame `oal.edit_done()` ao tratar. Devolve `false` fora do Android: use `keyboard`, que continua disponível como alternativa (ex.: gamepad).

**Seletor de cor (`colorpicker`).** Widget reutilizável de toque: arraste no quadrado (saturação × valor) e na barra de matiz; a prévia e o código `#RRGGBB` aparecem embaixo. `value=` é a cor atual (`0xRRGGBB`, inteiro) e `-> ação(item)` recebe a nova cor a cada movimento. O hex digitado fica por conta do app (um `field` + `keyboard` que chamam a mesma ação). Altura fixa (~280 dp). Tema: `oal.set_palette_full(fundo, barra, superfície, superfície_realçada, texto, secundário, destaque, botão, botão_pressionado, borda, selo)`; o texto sobre destaque/botão é escolhido por contraste (`oal.contrast_ink`). O botão da barra com ícone `back` é posicionado no **lado esquerdo**, antes do título.
| `if expr` / `else` | filhos | blocos condicionais (expressão OFS) |

`corner`: `bottom_right` (padrão do `fab`), `bottom_left` (padrão do `dpad`), `top_right`, `top_left`, `bottom_row` (fileira centrada embaixo), `top_row`, `right_column`, `left_column` (colunas centradas) ou uma **expressão OFS** (`corner=app.lado_dpad()`). Os cantos de baixo ficam acima da `bottom_row`.

Rótulos, títulos, textos de `empty` e nomes de ícone podem ser expressões: `button tr("jogar") icon=play`, `fab.icon player.icone_auto()`, `bar app.nome()`. Classe e ícone juntos: `fab.ctrl.icon "save"`.

`text` quebra em várias linhas na largura da tela (respeita `\n`); o aviso `empty` também.

### Navegação: barra inferior, menu suspenso, bottom sheet e segurar

**PT**
| Widget / atributo | Efeito |
| --- | --- |
| `navbar` com filhos `tab "rótulo" icon=nome selected=expr -> ação` | barra de navegação inferior fixa (ícone + rótulo; a aba `selected` fica destacada na cor de destaque do tema). O conteúdo rola acima dela e os `fab` sobem junto. |
| `popup menu -> fechar` / `popup sheet title=expr -> fechar`, filhos `item title=... icon=... [show=cond] -> ação` | `menu`: menu suspenso sob a barra, à direita (⋮); `sheet`: bottom sheet com título. Cobre a tela com fundo escuro; tocar fora chama a ação de fechar. Mostre-o com `if app.menu_aberto()`. Classe: `item.perigo`. |
| `card ... long=ação(item)` / `item ... long=ação(item)` | segurar (~0,45 s) o card/item chama a ação (o toque normal continua em `-> ação`). Mover o dedo cancela. |
| `bar "" logo=nome` | ícone (de `icons.ofs`) no lugar do texto do título da barra. |

Runtime: `oal.nav_begin/nav_tab/nav_end`, `oal.popup_begin(sheet, titulo, fechar)/popup_item/popup_end`, `oal.use_long(ação)`, `oal.bar_logo(nome)`. A pilha de telas e o botão voltar são do app (`on back`); `oal.reset_to(tela)` troca de aba limpando a pilha. Pilhas de `fab` no topo (`top_left`/`top_right`) encolhem para não cobrir o D-pad/losango do mesmo lado.

**EN**
- `navbar` with `tab "label" icon=name selected=expr -> action` children: fixed bottom navigation bar (icon + label; the `selected` tab uses the theme accent). Content scrolls above it and `fab`s move up with it.
- `popup menu -> close` / `popup sheet title=expr -> close`, children `item title=... icon=... [show=cond] -> action`: `menu` is a drop-down under the bar on the right (the ⋮ menu); `sheet` is a bottom sheet with a title. A dimmed scrim covers the screen; tapping outside calls the close action. Show it with `if app.menu_open()`. Class: `item.perigo`.
- `card ... long=action(item)` / `item ... long=action(item)`: long press (~0.45 s) calls the action (a normal tap still uses `-> action`). Moving the finger cancels it.
- `bar "" logo=name`: an `icons.ofs` icon in place of the bar title text.
- Runtime: `oal.nav_begin/nav_tab/nav_end`, `oal.popup_begin(sheet, title, close)/popup_item/popup_end`, `oal.use_long(action)`, `oal.bar_logo(name)`. Screen stack and back button belong to the app (`on back`); `oal.reset_to(screen)` switches tabs clearing the stack. Top `fab` stacks (`top_left`/`top_right`) shrink so they do not cover the D-pad/diamond on the same side.

**ES**
- `navbar` con hijos `tab "etiqueta" icon=nombre selected=expr -> acción`: barra de navegación inferior fija (icono + etiqueta; la pestaña `selected` usa el color de acento del tema). El contenido se desplaza encima y los `fab` suben con ella.
- `popup menu -> cerrar` / `popup sheet title=expr -> cerrar`, hijos `item title=... icon=... [show=cond] -> acción`: `menu` es un menú desplegable bajo la barra, a la derecha (el menú ⋮); `sheet` es un bottom sheet con título. Un fondo oscuro cubre la pantalla; tocar fuera llama a la acción de cerrar. Se muestra con `if app.menu_abierto()`. Clase: `item.perigo`.
- `card ... long=acción(item)` / `item ... long=acción(item)`: mantener pulsado (~0,45 s) llama a la acción (el toque normal sigue en `-> acción`). Mover el dedo lo cancela.
- `bar "" logo=nombre`: un icono de `icons.ofs` en lugar del texto del título de la barra.
- Runtime: `oal.nav_begin/nav_tab/nav_end`, `oal.popup_begin(sheet, título, cerrar)/popup_item/popup_end`, `oal.use_long(acción)`, `oal.bar_logo(nombre)`. La pila de pantallas y el botón atrás son de la app (`on back`); `oal.reset_to(pantalla)` cambia de pestaña vaciando la pila. Las pilas de `fab` superiores (`top_left`/`top_right`) se encogen para no tapar el D-pad/rombo del mismo lado.

### Listas e grades com dados (`from`)

`list jogos from biblioteca.jogos` chama funções do usuário pelo nome:

- `biblioteca.jogos_count() -> stone`;
- para cada atributo `x=item.campo` do modelo, `biblioteca.jogos_campo(i) -> obsidian`;
- sem modelo, `biblioteca.jogos_item(i)`.

O modelo é o filho `item` (lista) ou `card` (grade). `empty "texto"` aparece quando a contagem é 0. Na ação, `item` é o índice (`-> abrir(item)`). Sem `from`, os filhos são itens fixos: `item "título" "sub" -> ação`.

Atributos extras de `item`/`card` (literal ou `item.campo`):

| Atributo | Efeito |
|---|---|
| `image=item.capa` | capa do card (PNG, cortada para preencher) ou miniatura à esquerda do item |
| `icon=item.icone` | ícone (nome do `icons.ofs`); no card sem imagem vira a "capa" |
| `badge=item.motor` / `badge_icon=item.icone` | selo (ícone + texto curto) no canto do card ou à direita do item |

As imagens ficam num cache (até 40, reduzidas a 480 px); `oal.image_forget(caminho)` esquece uma imagem regravada.

Depois de mudar os dados, chame `oal.refresh()` para reconstruir a tela.

### Ícones

`button.icon "nome"`, `fab.icon "nome"` e `icon=nome` usam os ícones vetoriais do stdlib `icons.ofs`: `back`, `forward`, `reload`, `home`, `plus`, `close`, `lock`, `info`, `star`, `menu` (três pontos), `hamburger`, `search`, `globe`, `download`/`import`, `history`, `settings`/`config`, `profile`, `stop`, `file`, `bookmark`, `shield`, `plugin`/`plugins`, `save`, `folder`/`load`, `play`, `gamepad`.

## Ações (`->`)

| Ação | Efeito |
|---|---|
| `funcao` | chama `funcao()` do `.ofs` |
| `funcao(args)` / `ns.funcao(item)` | chama com argumentos (`item` = índice do item) |
| `@tela` | abre a tela (empilha) |
| `@back` | volta uma tela; na primeira, fecha o app |
| `@quit` | fecha o app |
| `@portrait` / `@landscape` / `@auto` | força a orientação em tempo de execução |

## Ciclo de vida (`on`)

`on start|back|pause|resume|rotate|low_memory|frame|key -> ação`. `key` (tecla/botão): o runtime só repassa teclas de controle (D-pad, A/B/X/Y, START/SELECT, Enter, Espaço, Esc, M, H); o app lê `oal.key_code` (códigos Android) e as repetições de tecla segurada são filtradas. O sistema trata as demais (volume etc.). Teclas de **controle físico** (fonte gamepad) ou o analógico marcam `oal.pad_connected = true` (zerado ao retomar o app), para esconder o gamepad virtual. `start` roda uma vez, depois dos estilos. `frame` roda a cada quadro enquanto `oal.set_animating(true)` (trabalho em pedaços sem travar a tela, animações, relógio do player). Sem `on back`, o voltar desempilha a tela ou fecha o app. Com `on back`, o app decide (chame `oal.back()` ou `oal.quit()`).

## Estilos (`style`)

```oal
style button.primario      // classe: usada como button.primario / fab.primario / dpad.primario
  fill #8b6cff             // fundo
  ink #101020              // texto e ícone
  curve 24                 // raio em dp
  opacity 60               // 0-100
  pressed fill #5f45c8     // estado pressionado
  pressed ink #ffffff
```

Alvos sem classe mudam o padrão do tipo: `style button`, `style card`, `style list`, `style bar` (cor da barra), `style screen` (`fill` = fundo do app, `ink` = texto). `style nome` sem tipo cria a classe `nome`. Cores: `#rgb`, `#rrggbb` ou decimal.

## API do runtime (para o `.ofs`)

| Chamada | Efeito |
|---|---|
| `oal.refresh()` / `oal.redraw()` | reconstrói / redesenha |
| `oal.go(n)`, `oal.back()`, `oal.quit()` | navegação (índice da tela na ordem do arquivo) |
| `oal.set_orientation(oal.ORIENT_PORTRAIT\|ORIENT_LANDSCAPE\|ORIENT_AUTO)` | força a orientação (JNI `setRequestedOrientation`) |
| `oal.pad_label(estilo, slot)`, `oal.pad_shape(estilo, slot)` | texto (A/B/Enter…) ou ícone (`icons.BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`) de cada botão do `diamond` |
| `oal.set_overlay_opacity(0..100)` | opacidade da camada flutuante (`fab`, `dpad`) |
| `oal.dpad_dir`, `oal.dpad_x`, `oal.dpad_y` | direção (`DIR_NONE/UP/RIGHT/DOWN/LEFT`) e vetor (-100..100) do D-pad |
| `oal.width`, `oal.height`, `oal.is_portrait()`, `oal.dp(v)` | tamanho em px e conversão dp → px |
| `oal.log(texto)` | logcat (tag `oal`) |
| `oal.toast(texto)` | mensagem curta por cima de tudo (some sozinha) |
| `oal.set_animating(bool)`, `oal.now_ms` | quadros contínuos (`on frame`) e relógio em ms |
| `oal.reset_to(n)`, `oal.replace(n)`, `oal.stack_depth()` | navegação sem empilhar / esvaziando a pilha |
| `oal.set_text_scale(%)`, `oal.set_overlay_scale(%)` | tamanho do texto; tamanho de `fab`/`dpad` |
| `oal.set_palette(bg, barra, superfície, superfície_alta, texto, apagado, destaque, texto_destaque)` | tema inteiro em tempo de execução |
| `oal.image_get(caminho)`, `oal.image_forget(caminho)` | cache de PNG decodificado |
| `oal.cv()`, `oal.font()`, `oal.font_bold()`, `oal._fs(dp)` | para desenhar dentro de um `canvas` |
| `oal.kb_shift`, `oal.kb_layer` | estado do teclado na tela |

### `canvas`

```oal
screen player
  canvas full draw=player.paint touch=player.touch
  fab.icon "menu" corner=bottom_row -> abrir_menu
```

O `.ofs` define `paint(x, y, w, h)` (desenha em `oal.cv()` na área dada) e `touch(kind, dedo, x, y)` (`kind` 0 desce, 1 move, 2 sobe; coordenadas relativas ao canvas). `full` ocupa toda a área abaixo da barra (não rola; `fab`/`dpad` ficam por cima). Chame `oal.redraw()` quando o desenho mudar. O oalc gera `oal_app_paint`/`oal_app_touch`.

O D-pad chama a ação a cada mudança de direção, com `item` = direção, e com `DIR_NONE` ao soltar.

## Multitoque

Cada dedo é seguido separado (`android.touch_id`): dá para segurar o D-pad e tocar num `fab` ao mesmo tempo. Só o primeiro dedo rola o conteúdo. Botões disparam ao soltar; slider e D-pad acompanham o arrasto; o `canvas` recebe todos os dedos.

## Desenho

Tudo é desenhado em software num buffer RGBA (`canvas` + `raster` + `ttf` + `icons`) e copiado para o `ANativeWindow`. A fonte é a Roboto do sistema. O layout usa o núcleo neutro do OLL (`oll.new_node`/`add_child`/`layout_node`); as medidas são em dp (densidade lida do `AConfiguration`). A área das barras do sistema (`onContentRectChanged`) é respeitada em retrato e paisagem. O conteúdo rola com arrasto.

## Desktop

`ofs run app.oal` abre o app numa janela X11 do tamanho de um celular (`oal_x11.ofs`). É **experimental e não suportado**: serve só para conferir layout; o alvo da OAL é o Android.

## Limites

- Texto: campo de texto do sistema via `oal.edit_open` (`keyboard` + `field` como alternativa); sem fontes CJK (a Roboto do sistema não tem japonês).
- Gestos: só toque, arrastar (rolagem) e segurar (`long=`); sem `swipe`/`pinch`. Sem `intent` e `storage` declarativos (use `android.start_intent` e caminhos do `android.ofs`).
- Camadas: conteúdo (rola), barra, flutuante e `canvas full`; um `canvas full` por tela.
- Render em software: telas com muitas imagens grandes ficam mais lentas na primeira vez (decodificação PNG em OFS).
