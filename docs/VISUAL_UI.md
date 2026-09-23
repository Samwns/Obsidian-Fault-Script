# Biblioteca Visual Nativa do OFS

O OFS disponibiliza camadas de interface gráfica nativas escritas em OFS puro sobre os módulos `window` e `canvas`.

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

---

## Camadas Visuais Nativas

O ecossistema OFS oferece duas abordagens complementares para o desenvolvimento de interfaces nativas:

1. **Modo Imediato (`canvas`, `window`, `ui`)**: Renderização direta e procedural sobre buffers de pixels raster, com controle de loop de eventos via polling.
2. **Modo Declarativo (`oll`)**: Obsidian Layout Language (`.oll`), que define árvores hierárquicas de componentes visuais, estilos modernos e renderização automática com taxa de quadros estável.

---

## OLL — Obsidian Layout Language (UI Declarativa)

A linguagem OLL permite desacoplar a definição e o layout da interface da lógica de execução do programa:

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

### Execução de Aplicativos OLL

```bash
# Executar aplicativo interativo nativo em janela X11 ou Win32:
ofs-ui arquivo.oll

# Ou gerar renderização raster estática em formato PPM:
ofs ui arquivo.oll -o preview.ppm
```

---

## Recursos Suportados

- `UiRect`: Estrutura geométrica para caixas e retângulos de layout.
- `UiStyle`: Estilização tipada com paletas de cores, bordas e preenchimentos.
- Temas padrão integrados: `ui.dark()` e `ui.light()`.
- OLL: Hierarquia declarativa de nós (`window`, `column`, `row`, `card`, `header`, `badge`, `progress`, `button`, `label`, `input`, `checkbox`).
- Dimensionamento e ancoragem: `auto_center` (centralização automática pelo monitor) e `resizable` (controle de redimensionamento da janela).
- Modelo de caixas completo: `padding`, `gap` e `margin`.
- Estados de controles interativos: `idle`, `hover`, `active` e `disabled`.
- Notação de cores hexadecimais nativas (`#1e1e2e`, `#89b4fa`).
- Primitivas visuais e ícones: botões, checkboxes, sliders, barras de progresso, ícones de adição, fechamento, confirmação e menu.
- Renderização tipográfica bitmap 5x7 integrada para caracteres alfanuméricos e pontuação básica.
- Captura de eventos de ponteiro com `input.mouse_x`, `input.mouse_y` e `input.is_down`.
- Drivers nativos de janela para Win32 no Windows e X11 no Linux.
