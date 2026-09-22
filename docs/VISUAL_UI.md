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

## Camadas Visuais Nativas

O ecossistema OFS oferece duas camadas complementares de interface nativa:

1. **Modo Imediato (`canvas`, `window`, `ui`)**: Desenho procedural de baixo nível sobre buffers de pixels e loop de polling de janelas.
2. **Modo Declarativo (`oll`)**: Obsidian Layout Language (.oll), definindo árvores hierárquicas de componentes, estilização visual moderna e renderização automática.

---

## OLL — Obsidian Layout Language (UI Declarativa)

A OLL permite desacoplar a estrutura visual da lógica do programa:

```oll
window "Painel do Sistema" {
    width: 640
    height: 480
    bg_color: #1e1e2e
    padding: 24

    column {
        gap: 16
        header "Monitor de Recursos" { fg_color: #cba6f7 }

        card {
            bg_color: #313244
            border_color: #45475a
            border_width: 1
            padding: 16

            row {
                label "CPU" { fg_color: #89b4fa }
                badge "45%" { fg_color: #a6e3a1 }
            }
            progress { value: 45 bg_color: #45475a fg_color: #a6e3a1 }
        }

        row {
            gap: 12
            button "Atualizar" { bg_color: #89b4fa fg_color: #11111b on_click: "refresh" }
            button "Fechar" { bg_color: #f38ba8 fg_color: #11111b on_click: "close" }
        }
    }
}
```

### Executando Aplicativos OLL

```bash
# Executar aplicativo interativo nativo em janela X11 ou Win32 (60 FPS):
ofs-ui arquivo.oll

# Ou gerar preview raster em imagem PPM:
ofs ui arquivo.oll -o preview.ppm
```

---

## O que existe nesta etapa

- `UiRect` para geometria de layout imediato.
- `UiStyle` como estilo nativo inspirado em CSS, mas tipado na linguagem.
- Temas `ui.dark()` e `ui.light()`.
- OLL: Árvore hierárquica declarativa (`window`, `column`, `row`, `card`, `header`, `badge`, `progress`, `button`, `label`, `input`, `checkbox`).
- Controle de geometria da janela: `auto_center` (centralização dinâmica pelo monitor) e `resizable` (trancamento de escalabilidade com `WS_FIXED` para instaladores/wizards sem vazios brancos).
- Modelo de caixas avançado: `padding`, `gap`, `margin`, ancoragem de tags à direita sem colisão textual.
- Estados interativos determinísticos: `idle`, `hover` (realce de borda e fundo a 60 FPS com `IDC_HAND`), `active` e `disabled`.
- Cores hexadecimais nativas (`#1e1e2e`, `#89b4fa`, etc.).
- Desenho de painel, borda, botão, checkbox, slider e barras de progresso.
- Icones em OFS puro: plus, close, check e menu.
- Texto bitmap 5x7 em OFS puro para letras, numeros e pontuacao basica.
- Helpers de layout: `inset`, `column_item`, `row_item`, `split_left` e `split_right`.
- Entrada por mouse usando `input.mouse_x`, `input.mouse_y` e `input.is_down`.
- Backend nativo Win32/GDI no Windows e X11 no Linux com janela interativa 60 FPS.

## Roadmap Futuro

- Renderizador Wayland nativo sem dependência de XWayland no Linux.
- Suporte a fontes tipográficas vetoriais (TrueType / OpenType).
- Acessibilidade do sistema operacional e navegação completa por foco de teclado.

