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
- Entrada por mouse usando `input.mouse_x`, `input.mouse_y` e `input.is_down`.

## Limites atuais

- O runtime C ainda usa stubs para janela real neste repositório-base; a API compila, mas abrir janela de verdade exige backend nativo por plataforma.
- Texto visual ainda não tem rasterizador de fonte. Por enquanto a UI desenha formas, widgets e icones.
- Layout responsivo, foco de teclado, acessibilidade e arvore de componentes ainda sao roadmap.

## Proximo backend nativo

Para Windows, a rota natural e implementar `ofs_window_*` em Win32/GDI ou Win32 + framebuffer.
Para Linux, a rota natural e X11 ou Wayland, mantendo a mesma ABI C usada por `window.ofs`.
