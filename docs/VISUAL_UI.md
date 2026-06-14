# Biblioteca visual nativa OFS

OFS agora tem uma primeira camada de UI escrita em OFS puro sobre `window` e `canvas`.

```ofs
attach {ui}

core main() {
    forge cv = canvas.create(96, 64)
    forge style = ui.dark()
    forge button = ui.rect(12, 14, 22, 16)

    window.create("OFS UI", 96, 64)
    while (window.poll()) {
        canvas.clear(cv, style.bg)
        ui.draw_button(cv, button, style, ui.contains(button, input.mouse_x(), input.mouse_y()), false)
        ui.draw_plus_icon(cv, button.x + 7, button.y + 4, style.text)
        ui.draw_text(cv, 12, 36, "OFS UI", style.text, 1)
        canvas.present(cv)
    }
    window.destroy()
    canvas.destroy(cv)
}
```

## O que existe nesta etapa

- `UiRect` para geometria de layout.
- `UiStyle` como estilo nativo inspirado em CSS, mas tipado na linguagem.
- Temas `ui.dark()` e `ui.light()`.
- Desenho de painel, borda, botão, checkbox e slider.
- Icones em OFS puro: plus, close, check e menu.
- Texto bitmap 5x7 em OFS puro para letras, numeros e pontuacao basica.
- Helpers de layout: `inset`, `column_item`, `row_item`, `split_left` e `split_right`.
- Entrada por mouse usando `input.mouse_x`, `input.mouse_y` e `input.is_down`.
- Backend nativo Win32/GDI no Windows e X11 carregado dinamicamente no Linux.

## Limites atuais

- Linux usa X11 quando `libX11.so.6` e `DISPLAY` existem; sem display ele entra em modo headless para CI.
- Wayland puro, acessibilidade completa, foco de teclado rico e arvore declarativa de componentes ainda sao roadmap.
- O renderer de texto atual e bitmap 5x7; fonte proporcional/Unicode ainda nao existe.

## Proximo backend nativo

O contrato `ofs_window_*` ja esta pronto para trocar ou somar backends sem mudar codigo OFS.
Os proximos passos naturais sao Wayland nativo no Linux e uma camada de componentes com estado/foco.
