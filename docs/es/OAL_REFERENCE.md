# Referencia OAL (v1)

OAL (Obsidian Android Language, `.oal`) describe lo que es **propio de Android** en una app OFS: el manifiesto, las pantallas táctiles, los estilos, las acciones y el ciclo de vida. La lógica sigue en `.ofs`. Es hermana de ODL (documento → HTML), OES (estilo → CSS) y OLL (UI de escritorio), y cumple el papel del CSS/OES en Android (bloques `style`).

- Compilador: `ofs/tools/oalc.ofs`. Runtime: `ofs/stdlib/oal_runtime.ofs` + `oal_android.ofs` (se enlazan solos; no se importa la OAL).
- Build: `ofs build --target android app.oal [-o app.apk] [--install] [--serial ID]` (ver [ANDROID.md](ANDROID.md)).

## Ejemplo

```oal
// lógica en OFS: las dos formas son equivalentes
use "biblioteca.ofs"
// attach {F:biblioteca.ofs}

app "M.O.O.N" "com.fixedseat.moon"
  version "0.1.0"
  permission internet

style button.primario
  fill #8b6cff
  ink #101020
  curve 24
  pressed fill #5f45c8

screen biblioteca
  bar "M.O.O.N"
    button.icon "plugins" -> abrir_plugins
  button.primario "Importar juego" icon=import -> importar
  list juegos from biblioteca.juegos
    item title=item.nombre -> abrir(item)
    empty "Sin juegos."

on back -> volver
```

## Reglas

- Una declaración por línea; jerarquía por **indentación**. Comentarios `//` o `#`.
- Texto entre comillas; sin comillas, el valor es una expresión OFS.
- Nivel superior: `use "x.ofs"` = `attach {F:x.ofs}` = `attach {x}` (forma corta del OFS), `app`, `style`, `screen`, `on`.
- `app`: `version`, `code`, `icon` (PNG), `orientation any|portrait|landscape`, `permission ...`, `fullscreen`.
- Widgets: `bar`, `title`, `text`, `button` (`icon=`, `.clase`), `button.icon "nombre"`, `card`, `list`/`grid` (`from fuente`, `item`, `card`, `empty`, `columns=`, `landscape=`), `row`, `portrait`, `landscape`, `fab`, `dpad` (`corner=`), `diamond` (`corner=`, `look=`: 4 botones de acción en rombo, estilos `oal.PAD_KEYS/XBOX/PLAY/NINTENDO`; formas vectoriales sin logotipos).
- `from fuente`: llama `fuente_count()` y `fuente_campo(i)` por cada `x=item.campo`.
- Acciones: `funcion`, `funcion(item)`, `@pantalla`, `@back`, `@quit`, `@portrait`, `@landscape`, `@auto`.
- Ciclo de vida: `on back|pause|resume|rotate|low_memory -> accion`.
- Estilos: `fill`, `ink`, `curve`, `opacity`, `pressed fill`, `pressed ink`; destinos `button`, `card`, `list`, `bar`, `screen` o una clase.
- Iconos: los del stdlib `icons.ofs` (`back`, `settings`, `plugins`, `save`, `load`, `play`, `menu`, `hamburger`, `gamepad`, `import`…).
- API: `oal.refresh()`, `oal.go(n)`, `oal.back()`, `oal.quit()`, `oal.set_orientation(...)`, `oal.set_overlay_opacity(...)`, `oal.dpad_dir`, `oal.toast(texto)`, `oal.set_animating(bool)` + `oal.now_ms`, `oal.reset_to(n)`, `oal.replace(n)`, `oal.set_text_scale(%)`, `oal.set_overlay_scale(%)`, `oal.set_palette(...)`, `oal.image_get`/`image_forget`, `oal.cv()`.
- **Nuevo en v1:**
  - `image expr` (PNG ajustada al ancho), `canvas [full] draw=f touch=g` (área dibujada por la app: `f(x, y, w, h)`, `g(tipo, dedo, x, y)` con 0 baja, 1 mueve, 2 sube), `tabs` + `tab ... selected=expr`, `slider ... value= min= max= -> accion(item)`, `toggle ... value= -> accion(item)`, `progress ... value=`, `field "pista" value=`, `keyboard -> accion(item)` (Unicode, 8 = borrar, 10 = confirmar; `oal.kb_layer = 2` muestra el teclado hex 0-9 A-F), `colorpicker value=expr -> accion(item)` (selector de color táctil: cuadrado saturación/valor, barra de matiz, vista previa y hex; `value`/`item` = `0xRRGGBB`; la acción corre mientras se arrastra; el hex escrito queda a cargo de la app con `field` + `keyboard`; tema completo con `oal.set_palette_full(fondo, barra, superficie, superficie_realzada, texto, secundario, acento, botón, botón_pulsado, borde, sello)`), `if expr` / `else`. Un botón de barra con icono `back` se coloca en el **lado izquierdo**, antes del título.
  - `item`/`card`: `image=`, `icon=` dinámico, `badge=` + `badge_icon=` (sello).
  - `text` y `empty` en varias líneas; etiquetas e iconos como expresiones; `fab.clase.icon`.
  - `corner`: además `bottom_row`, `top_row`, `right_column`, `left_column` o una expresión.
  - Eventos `on start`, `on frame` y `on key` (teclas de mando/teclado en `oal.key_code`; `oal.pad_connected` indica mando físico); `app`: `assets "carpeta"`, `icon` PNG copiado a `res/mipmap/`.
  - Multitáctil: cada dedo por separado (D-pad + `fab` a la vez).

## Escritorio

`ofs run app.oal` abre una ventana X11 del tamaño de un móvil: **experimental, sin soporte**.

## Límites

Sin campo de texto del sistema (usa `keyboard` + `field`); sin fuentes CJK; sin gestos ni `intent`/`storage` declarativos; un `canvas full` por pantalla; render por software.
