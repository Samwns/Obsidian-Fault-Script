# Referencia OAL (v1)

OAL (Obsidian Android Language, `.oal`) describe lo que es **propio de Android** en una app OFS: el manifiesto, las pantallas táctiles, los estilos, las acciones y el ciclo de vida. La lógica sigue en `.ofs`. Es hermana de ODL (documento → HTML), OES (estilo → CSS) y OLL (UI de escritorio), y también cumple el papel del CSS/OES en Android (bloque `style`).

```
app.oal ──oalc──► AndroidManifest.xml
                └► app_oal.ofs ──ofscc (aarch64)──► libapp.so ──► APK
```

- Compilador: `ofs/tools/oalc.ofs` (`OALC_INPUT`, `OALC_OUT_DIR`, `OALC_TARGET=android|desktop`).
- Runtime: `ofs/stdlib/oal_runtime.ofs` (núcleo neutro) + `oal_android.ofs` (backend). El usuario no importa la OAL: el archivo generado ya enlaza el runtime y `android.ofs`.
- Build: `ofs build --target android app.oal [-o app.apk] [--install] [--serial ID]` (ver [ANDROID.md](ANDROID.md)).

---

## Ejemplo

```oal
// lógica en OFS: las dos formas de abajo tienen el mismo efecto
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
  text biblioteca.resumen()
  button.primario "Importar juego" icon=import -> importar
  list juegos from biblioteca.juegos
    item title=item.nombre sub=item.motor -> abrir_juego(item)
    empty "Aún no hay juegos."

screen plugins
  bar "Plugins"
    button.icon "back" -> @back
  list from biblioteca.plugins
    item title=item.nombre sub=item.estado
    empty "Ningún plugin registrado."

on back -> volver
on pause -> quick_save
```

```ofs
// biblioteca.ofs
namespace biblioteca {
    vein juegos_count() -> stone { return 0 }
    vein juegos_nombre(i: stone) -> obsidian { return "" }
    vein juegos_motor(i: stone) -> obsidian { return "" }
    vein resumen() -> obsidian { return "0 juegos instalados" }
}
vein abrir_plugins() -> void { oal.go(1) }
vein volver() -> void { oal.back() }
```

---

## Reglas generales

- Una declaración por línea; la jerarquía la da la **indentación** (2 espacios o tab).
- Comentarios: `// ...` o `# ...` en línea propia.
- Textos entre comillas `"..."` (con `\"`, `\n`). Fuera de comillas, un valor es una **expresión OFS** (`biblioteca.resumen()`).
- `clave=valor` sin espacios alrededor del `=` (`title=item.nombre`, `corner=bottom_left`).
- También valen las palabras en portugués: `tela`, `barra`, `botao`, `texto`, `titulo`, `lista`, `grade`, `cartao`, `linha`, `vazio`, `retrato`, `paisagem`, `estilo`, `usa`, `ao`.
- Los errores salen como `oalc: linha N: ...` y no se genera nada.

## Nivel superior

| Declaración | Efecto |
|---|---|
| `use "archivo.ofs"` | incluye la lógica (ruta relativa al `.oal`) |
| `attach {F:archivo.ofs}` | igual que `use`, con la sintaxis del `attach` de OFS |
| `attach {modulo}` | forma corta de OFS: `modulo.ofs` junto al `.oal`, si no del stdlib (el guion pasa a `_`) |
| `app "Nombre" "paquete"` + propiedades | genera el `AndroidManifest.xml` |
| `style destino` + propiedades | estilo (ver abajo) |
| `screen nombre` + widgets | una pantalla; la primera es la inicial |
| `on evento -> acción` | ciclo de vida |

### `app`

| Propiedad | Ejemplo | Manifiesto |
|---|---|---|
| `version "a.b.c"` | `version "0.1.0"` | `versionName`; `versionCode = a*10000 + b*100 + c` |
| `code N` | `code 7` | `versionCode` explícito |
| `icon "archivo.png"` | | `@mipmap/ic_launcher` (solo PNG; la CLI copia a `res/mipmap/`) |
| `orientation any\|portrait\|landscape` | | `screenOrientation` (`sensorLandscape` para horizontal) |
| `permission a b ...` | `permission internet vibrate` | `android.permission.INTERNET`... (`storage` = lectura + escritura; un nombre con punto pasa tal cual) |
| `fullscreen` | | tema `NoTitleBar.Fullscreen` |
| `assets "carpeta"` | `assets "../android/assets"` | la carpeta va al `assets/` del APK (leer con `android.asset_read`) |

El nombre de la `.so` (`android.app.lib_name`) es el último segmento del paquete. La activity siempre declara `configChanges` de orientación/tamaño: girar **no** recrea la app, solo dispara `on rotate`.

## Widgets (`screen`)

| Widget | Forma | Genera |
|---|---|---|
| `bar "Título"` | hijos: `button` | barra fija arriba |
| `title "texto"` | | título |
| `text "texto"` / `text expr()` | | texto secundario |
| `button "etiqueta" -> acción` | `icon=nombre`, `.clase` | botón |
| `button.icon "nombre" -> acción` | | botón solo con icono |
| `card "título" "sub" -> acción` | `title=`, `sub=` | tarjeta |
| `list [id] [from fuente]` | hijos: `item`, `empty` | lista |
| `grid [id] [from fuente] [columns=2] [landscape=4]` | hijos: `card`, `empty` | cuadrícula con columnas por orientación |
| `row` | hijos | widgets lado a lado |
| `portrait` / `landscape` | hijos | solo aparecen en esa orientación |
| `fab "etiqueta" -> acción` / `fab.icon "nombre" -> acción` | `corner=` | botón flotante (capa encima, no se desplaza) |
| `dpad -> acción(item)` | `corner=` | D-pad virtual (capa flotante) |
| `diamond -> acción(item)` | `corner=`, `look=` | 4 botones de acción en rombo (capa flotante); `item` = botón tocado (`oal.SLOT_TOP/RIGHT/BOTTOM/LEFT`); `look` = `oal.PAD_KEYS` (teclas), `PAD_XBOX` (A B X Y), `PAD_PLAY` (✕ ○ □ △ en vectores, sin logotipos), `PAD_NINTENDO` (A B X Y espejado) |
| `image expr` | `height=` (dp; 0 = proporcional) | imagen PNG (ruta) ajustada al ancho |
| `canvas [full] draw=f touch=g` | `height=` | área dibujada por la app (ver abajo) |
| `tabs` | hijos: `tab "etiqueta" icon= selected=expr -> acción` | pestañas / segmentos lado a lado |
| `slider "etiqueta" value=expr min= max= -> acción(item)` | `icon=` | control deslizante; `item` = nuevo valor |
| `colorpicker value=expr -> acción(item)` | | selector de color (táctil): cuadrado saturación/valor, barra de matiz, vista previa y hex; `value`/`item` = `0xRRGGBB`; la acción corre mientras se arrastra |
| `toggle "etiqueta" value=expr -> acción(item)` | `icon=` | interruptor; `item` = 1 (encender) o 0 |
| `progress "etiqueta" value=expr` | | barra de progreso 0-100 |
| `field "pista" value=expr -> acción` | `icon=` | campo de texto (el toque llama a la acción) |
| `keyboard -> acción(item)` | | teclado en pantalla; `item` = código Unicode, 8 = borrar, 10 = confirmar. Capa hex (`oal.kb_layer = 2`): teclas 0-9, A-F, borrar, `#` (abc) y ↵. |

**Entrada de texto del sistema (`oal.edit_open`).** `oal.edit_open(título, texto, multilínea) -> bool` abre un Dialog de Android con un `EditText` nativo (stdlib `android_input.ofs`, solo JNI): teclado predeterminado del dispositivo (Gboard), cursor, desplazamiento del texto, pegar y autocompletar. Botones OK/Cancelar (`oal.edit_ok_label`/`edit_cancel_label`). El resultado llega en `on frame`: `oal.edit_state` 2 = confirmado, 3 = cancelado, con `oal.edit_text`; llama a `oal.edit_done()` al tratarlo. Devuelve `false` fuera de Android: usa `keyboard`, que sigue disponible como alternativa (p. ej. gamepad).

**Selector de color (`colorpicker`).** Widget táctil reutilizable: arrastra en el cuadrado (saturación × valor) y en la barra de matiz; la vista previa y el código `#RRGGBB` aparecen abajo. `value=` es el color actual (`0xRRGGBB`, entero) y `-> acción(item)` recibe el nuevo color en cada movimiento. El hex escrito queda a cargo de la app (un `field` + `keyboard` que llaman a la misma acción). Altura fija (~280 dp). Tema: `oal.set_palette_full(fondo, barra, superficie, superficie_realzada, texto, secundario, acento, botón, botón_pulsado, borde, sello)`; el texto sobre acento/botón se elige por contraste (`oal.contrast_ink`). El botón de la barra con icono `back` se coloca en el **lado izquierdo**, antes del título.

| Widget | Forma | Genera |
|---|---|---|
| `if expr` / `else` | hijos | bloques condicionales (expresión OFS) |

`corner`: `bottom_right` (predeterminado del `fab`), `bottom_left` (predeterminado del `dpad`), `top_right`, `top_left`, `bottom_row` (fila centrada abajo), `top_row`, `right_column`, `left_column` (columnas centradas) o una **expresión OFS** (`corner=app.lado_dpad()`). Las esquinas de abajo quedan sobre la `bottom_row`.

Las etiquetas, títulos, textos de `empty` y nombres de icono pueden ser expresiones: `button tr("jugar") icon=play`, `fab.icon player.icono_auto()`, `bar app.nombre()`. Clase e icono juntos: `fab.ctrl.icon "save"`.

`text` se parte en varias líneas al ancho de la pantalla (respeta `\n`); el aviso `empty` también.

### Navegación: barra inferior, menú desplegable, bottom sheet y mantener pulsado

| Widget / atributo | Efecto |
| --- | --- |
| `navbar` con hijos `tab "etiqueta" icon=nombre selected=expr -> acción` | barra de navegación inferior fija (icono + etiqueta; la pestaña `selected` se resalta con el color de acento del tema). El contenido se desplaza encima y los `fab` suben con ella. |
| `popup menu -> cerrar` / `popup sheet title=expr -> cerrar`, hijos `item title=... icon=... [show=cond] -> acción` | `menu`: menú desplegable bajo la barra, a la derecha (⋮); `sheet`: bottom sheet con título. Un fondo oscuro cubre la pantalla; tocar fuera llama a la acción de cerrar. Se muestra con `if app.menu_abierto()`. Clase: `item.perigo`. |
| `card ... long=acción(item)` / `item ... long=acción(item)` | mantener pulsado (~0,45 s) el card/item llama a la acción (el toque normal sigue en `-> acción`). Mover el dedo lo cancela. |
| `bar "" logo=nombre` | un icono (de `icons.ofs`) en lugar del texto del título de la barra. |

Runtime: `oal.nav_begin/nav_tab/nav_end`, `oal.popup_begin(sheet, título, cerrar)/popup_item/popup_end`, `oal.use_long(acción)`, `oal.bar_logo(nombre)`. La pila de pantallas y el botón atrás son de la app (`on back`); `oal.reset_to(pantalla)` cambia de pestaña vaciando la pila. Las pilas de `fab` superiores (`top_left`/`top_right`) se encogen para no tapar el D-pad/rombo del mismo lado.

### Listas y cuadrículas con datos (`from`)

`list juegos from biblioteca.juegos` llama a funciones del usuario por nombre:

- `biblioteca.juegos_count() -> stone`;
- por cada atributo `x=item.campo` del modelo, `biblioteca.juegos_campo(i) -> obsidian`;
- sin modelo, `biblioteca.juegos_item(i)`.

El modelo es el hijo `item` (lista) o `card` (cuadrícula). `empty "texto"` aparece cuando la cuenta es 0. En la acción, `item` es el índice (`-> abrir(item)`). Sin `from`, los hijos son ítems fijos: `item "título" "sub" -> acción`.

Atributos extra de `item`/`card` (literal o `item.campo`):

| Atributo | Efecto |
|---|---|
| `image=item.portada` | portada del card (PNG, recortada para llenar) o miniatura a la izquierda del ítem |
| `icon=item.icono` | icono (nombre de `icons.ofs`); en el card sin imagen pasa a ser la "portada" |
| `badge=item.motor` / `badge_icon=item.icono` | sello (icono + texto corto) en la esquina del card o a la derecha del ítem |

Las imágenes quedan en un caché (hasta 40, reducidas a 480 px); `oal.image_forget(ruta)` olvida una imagen regrabada.

Tras cambiar los datos, llama a `oal.refresh()` para reconstruir la pantalla.

### Iconos

`button.icon "nombre"`, `fab.icon "nombre"` e `icon=nombre` usan los iconos vectoriales del stdlib `icons.ofs`: `back`, `forward`, `reload`, `home`, `plus`, `close`, `lock`, `info`, `star`, `menu` (tres puntos), `hamburger`, `search`, `globe`, `download`/`import`, `history`, `settings`/`config`, `profile`, `stop`, `file`, `bookmark`, `shield`, `plugin`/`plugins`, `save`, `folder`/`load`, `play`, `gamepad`.

## Acciones (`->`)

| Acción | Efecto |
|---|---|
| `funcion` | llama a `funcion()` del `.ofs` |
| `funcion(args)` / `ns.funcion(item)` | llama con argumentos (`item` = índice del ítem) |
| `@pantalla` | abre la pantalla (apila) |
| `@back` | vuelve una pantalla; en la primera, cierra la app |
| `@quit` | cierra la app |
| `@portrait` / `@landscape` / `@auto` | fuerza la orientación en tiempo de ejecución |

## Ciclo de vida (`on`)

`on start|back|pause|resume|rotate|low_memory|frame|key -> acción`. `key` (tecla/botón): el runtime solo reenvía teclas de control (D-pad, A/B/X/Y, START/SELECT, Enter, Espacio, Esc, M, H); la app lee `oal.key_code` (códigos Android) y se filtran las repeticiones de tecla mantenida. El sistema trata las demás (volumen, etc.). Las teclas de **mando físico** (fuente gamepad) o el analógico marcan `oal.pad_connected = true` (se limpia al reanudar la app), para ocultar el gamepad virtual. `start` corre una vez, después de los estilos. `frame` corre en cada cuadro mientras `oal.set_animating(true)` (trabajo en trozos sin congelar la pantalla, animaciones, reloj del player). Sin `on back`, el atrás desapila la pantalla o cierra la app. Con `on back`, la app decide (llama a `oal.back()` o `oal.quit()`).

## Estilos (`style`)

```oal
style button.primario      // clase: se usa como button.primario / fab.primario / dpad.primario
  fill #8b6cff             // fondo
  ink #101020              // texto e icono
  curve 24                 // radio en dp
  opacity 60               // 0-100
  pressed fill #5f45c8     // estado pulsado
  pressed ink #ffffff
```

Los destinos sin clase cambian el valor predeterminado del tipo: `style button`, `style card`, `style list`, `style bar` (color de la barra), `style screen` (`fill` = fondo de la app, `ink` = texto). `style nombre` sin tipo crea la clase `nombre`. Colores: `#rgb`, `#rrggbb` o decimal.

## API del runtime (para el `.ofs`)

| Llamada | Efecto |
|---|---|
| `oal.refresh()` / `oal.redraw()` | reconstruye / redibuja |
| `oal.go(n)`, `oal.back()`, `oal.quit()` | navegación (índice de la pantalla en el orden del archivo) |
| `oal.set_orientation(oal.ORIENT_PORTRAIT\|ORIENT_LANDSCAPE\|ORIENT_AUTO)` | fuerza la orientación (JNI `setRequestedOrientation`) |
| `oal.pad_label(estilo, slot)`, `oal.pad_shape(estilo, slot)` | texto (A/B/Enter...) o icono (`icons.BTN_CROSS/CIRCLE/SQUARE/TRIANGLE`) de cada botón del `diamond` |
| `oal.set_overlay_opacity(0..100)` | opacidad de la capa flotante (`fab`, `dpad`) |
| `oal.dpad_dir`, `oal.dpad_x`, `oal.dpad_y` | dirección (`DIR_NONE/UP/RIGHT/DOWN/LEFT`) y vector (-100..100) del D-pad |
| `oal.width`, `oal.height`, `oal.is_portrait()`, `oal.dp(v)` | tamaño en px y conversión dp → px |
| `oal.log(texto)` | logcat (tag `oal`) |
| `oal.toast(texto)` | mensaje corto por encima de todo (desaparece solo) |
| `oal.set_animating(bool)`, `oal.now_ms` | cuadros continuos (`on frame`) y reloj en ms |
| `oal.reset_to(n)`, `oal.replace(n)`, `oal.stack_depth()` | navegación sin apilar / vaciando la pila |
| `oal.set_text_scale(%)`, `oal.set_overlay_scale(%)` | tamaño del texto; tamaño de `fab`/`dpad` |
| `oal.set_palette(bg, barra, superficie, superficie_alta, texto, apagado, acento, texto_acento)` | tema completo en tiempo de ejecución |
| `oal.image_get(ruta)`, `oal.image_forget(ruta)` | caché de PNG decodificado |
| `oal.cv()`, `oal.font()`, `oal.font_bold()`, `oal._fs(dp)` | para dibujar dentro de un `canvas` |
| `oal.kb_shift`, `oal.kb_layer` | estado del teclado en pantalla |

### `canvas`

```oal
screen player
  canvas full draw=player.paint touch=player.touch
  fab.icon "menu" corner=bottom_row -> abrir_menu
```

El `.ofs` define `paint(x, y, w, h)` (dibuja en `oal.cv()` en el área dada) y `touch(kind, dedo, x, y)` (`kind` 0 baja, 1 mueve, 2 sube; coordenadas relativas al canvas). `full` ocupa toda el área bajo la barra (no se desplaza; `fab`/`dpad` quedan encima). Llama a `oal.redraw()` cuando cambie el dibujo. El oalc genera `oal_app_paint`/`oal_app_touch`.

El D-pad llama a la acción en cada cambio de dirección, con `item` = dirección, y con `DIR_NONE` al soltar.

## Multitáctil

Cada dedo se sigue por separado (`android.touch_id`): se puede mantener el D-pad y tocar un `fab` a la vez. Solo el primer dedo desplaza el contenido. Los botones disparan al soltar; slider y D-pad acompañan el arrastre; el `canvas` recibe todos los dedos.

## Dibujo

Todo se dibuja por software en un buffer RGBA (`canvas` + `raster` + `ttf` + `icons`) y se copia al `ANativeWindow`. La fuente es la Roboto del sistema. El layout usa el núcleo neutro de OLL (`oll.new_node`/`add_child`/`layout_node`); las medidas son en dp (densidad leída de `AConfiguration`). Se respeta el área de las barras del sistema (`onContentRectChanged`) en vertical y horizontal. El contenido se desplaza arrastrando.

## Escritorio

`ofs run app.oal` abre la app en una ventana X11 del tamaño de un móvil (`oal_x11.ofs`). Es **experimental y sin soporte**: sirve solo para revisar el layout; el destino de OAL es Android.

## Límites

- Texto: campo de texto del sistema con `oal.edit_open` (`keyboard` + `field` como alternativa); sin fuentes CJK (la Roboto del sistema no tiene japonés).
- Gestos: solo toque, arrastrar (desplazamiento) y mantener pulsado (`long=`); sin `swipe`/`pinch`. Sin `intent` ni `storage` declarativos (usa `android.start_intent` y las rutas de `android.ofs`).
- Capas: contenido (se desplaza), barra, flotante y `canvas full`; un `canvas full` por pantalla.
- Render por software: las pantallas con muchas imágenes grandes son más lentas la primera vez (decodificación PNG en OFS).
