#!/usr/bin/env python3
"""Gera docs/screenshots/refract.gif a partir de uma execucao REAL de `ofs refract`.

Passos: (1) roda o wrapper `ofs refract` sobre uma copia de
ofs/examples/refract_gif.ofs; (2) edita o arquivo em 3 momentos (como um
editor ao salvar); (3) grava stdout+stderr com carimbo de tempo; (4) desenha
editor (esquerda) e terminal (direita) no tempo real gravado.

Uso:  python3 ofs/tools/gif/make_refract_gif.py [--keep-log log.json] [--out gif]
Requer Python 3 + Pillow. Memoria limitada com ulimit -v 3000000 + timeout.
"""
import argparse, json, os, shutil, subprocess, sys, tempfile, threading, time
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[3]
SRC = ROOT / "ofs/examples/refract_gif.ofs"
W, H = 900, 500
FONTS = ["/usr/share/fonts/nerd-fonts/JetBrainsMono/JetBrainsMonoNerdFont-Regular.ttf",
         "/usr/share/fonts/google-noto/NotoSansMono-Regular.ttf",
         "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf"]
BG, PANEL, PANEL2 = (22, 17, 34), (30, 24, 46), (14, 11, 22)
FG, DIM, PURPLE, GREEN, AMBER, CYAN = (222, 216, 240), (122, 112, 150), (150, 110, 255), (130, 220, 150), (255, 200, 90), (120, 210, 240)
HL = (70, 48, 120)

# edicoes: (segundos apos iniciar, trecho antigo, trecho novo)
EDITS = [
    (4.6, '"passo "', '"[v2] passo "'),
    (9.4, 's = s + "#"', 's = s + "="'),
    (14.2, '"[v2] passo " + t + "  " + barra(p % 12)',
           '"[v3] passo " + t + "  " + barra(p % 20)'),
]
END = 19.0

def record(log_path):
    tmp = Path(tempfile.mkdtemp(prefix="ofs-gif-"))
    prog = tmp / "refract_gif.ofs"
    shutil.copy(SRC, prog)
    env = dict(os.environ, NO_COLOR="1", OFS_PROGRESS="plain",
               OFSCC=str(ROOT / "ofs/dist/ofscc"), OFS_RUNTIME=str(ROOT / "ofs/dist/magma.o"),
               OFS_STDLIB_PATH=str(ROOT / "ofs/stdlib"), OFS_REFRACT_PERIOD="100")
    cmd = ["bash", "-c", f'ulimit -v 3000000; exec timeout 40 "{ROOT}/src/packaging/linux/ofs" refract "{prog}"']
    t0 = time.monotonic()
    lines, versions = [], [(0.0, prog.read_text())]
    p = subprocess.Popen(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
    def reader():
        for ln in p.stdout:
            lines.append((time.monotonic() - t0, ln.rstrip("\n")))
    th = threading.Thread(target=reader, daemon=True); th.start()
    # espera o primeiro passo aparecer; o relogio do roteiro comeca ali
    while not any(l.startswith("passo") for _, l in lines):
        time.sleep(0.05)
        if p.poll() is not None: sys.exit("ofs refract terminou cedo: " + repr(lines))
    base = next(t for t, l in lines if l.startswith("passo"))
    for at, old, new in EDITS:
        while time.monotonic() - t0 < base + at: time.sleep(0.01)
        txt = prog.read_text(); assert old in txt, old
        prog.write_text(txt.replace(old, new, 1))
        versions.append((time.monotonic() - t0, prog.read_text()))
    while time.monotonic() - t0 < base + END: time.sleep(0.05)
    p.terminate(); th.join(2)
    try: p.wait(3)
    except Exception: p.kill()
    shutil.rmtree(tmp, ignore_errors=True)
    data = {"base": base, "lines": lines, "versions": versions}
    Path(log_path).write_text(json.dumps(data))
    return data

def font(sz):
    for f in FONTS:
        if os.path.exists(f): return ImageFont.truetype(f, sz)
    sys.exit("sem fonte monoespacada")

F, FS, FT = font(12), font(11), font(12)

def code_lines(txt):
    ls = txt.split("\n")
    a = next(i for i, l in enumerate(ls) if l.startswith("vein barra"))
    b = next(i for i, l in enumerate(ls) if l.startswith("core main"))
    return [(i + 1, ls[i]) for i in range(a, b)]

KW = ("vein", "forge", "while", "return", "echo")
def draw_code_line(d, x, y, l, color_default=FG):
    import re
    pos = x
    for tok in re.findall(r'"[^"]*"|\w+|\s+|.', l):
        c = color_default
        if tok in KW: c = PURPLE
        elif tok.startswith('"'): c = GREEN
        elif tok.isdigit(): c = AMBER
        d.text((pos, y), tok, font=F, fill=c)
        pos += d.textlength(tok, font=F)

def render(data, out):
    base = data["base"]
    lines = [(t - base, l) for t, l in data["lines"]]
    vers = [(t - base, v) for t, v in data["versions"]]
    vers[0] = (-99, vers[0][1])
    # instantes de quadro: cada linha nova, cada edicao e a janela do "salvo"
    marks = {round(t, 2) for t, _ in lines if t >= -0.5} | {round(t, 2) for t, _ in vers[1:]}
    for t, _ in vers[1:]: marks |= {round(t + 0.9, 2)}
    marks |= {0.0, END}
    ts = sorted(m for m in marks if 0 <= m <= END)
    # funde quadros muito proximos
    merged = []
    for t in ts:
        if not merged or t - merged[-1] >= 0.12: merged.append(t)
    frames, durs = [], []
    for i, t in enumerate(merged):
        frames.append(frame(t, lines, vers))
        nxt = merged[i + 1] if i + 1 < len(merged) else t + 1.5
        durs.append(max(20, int(round((nxt - t) * 100)) * 10))
    pal = [f.convert("P", palette=Image.ADAPTIVE, colors=96) for f in frames]
    pal[0].save(out, save_all=True, append_images=pal[1:], duration=durs, loop=0, optimize=True, disposal=1)
    return len(frames), sum(durs) / 1000

def frame(t, lines, vers):
    im = Image.new("RGB", (W, H), BG); d = ImageDraw.Draw(im)
    d.text((W // 2, 18), "ofs refract — live edit, no restart, state kept", font=FT, fill=DIM, anchor="mm")
    top, bot, mid = 38, H - 14, 12 + 440
    lx0, lx1, rx0, rx1 = 12, 430, 442, W - 12
    # editor
    d.rounded_rectangle((lx0, top, lx1, bot), 8, fill=PANEL)
    d.rectangle((lx0, top, lx1, top + 26), fill=PANEL2)
    d.text((lx0 + 12, top + 13), "refract_gif.ofs", font=FS, fill=FG, anchor="lm")
    cur = [v for v in vers if v[0] <= t][-1]
    idx = vers.index(cur)
    prev = vers[idx - 1][1] if idx > 0 else cur[1]
    since = t - cur[0]
    pl = dict(code_lines(prev)); y = top + 40
    for n, l in code_lines(cur[1]):
        changed = idx > 0 and pl.get(n) != l and since < 5.0
        if changed: d.rectangle((lx0 + 4, y - 2, lx1 - 4, y + 17), fill=HL)
        d.text((lx0 + 12, y), f"{n:>2}", font=F, fill=DIM)
        draw_code_line(d, lx0 + 40, y, l)
        y += 20
    if idx > 0 and since < 0.9:
        d.rounded_rectangle((lx1 - 108, top + 4, lx1 - 8, top + 22), 6, fill=(60, 130, 90))
        d.text((lx1 - 58, top + 13), "● salvo", font=FS, fill=(235, 255, 240), anchor="mm")
    # terminal
    d.rounded_rectangle((rx0, top, rx1, bot), 8, fill=PANEL2)
    d.rectangle((rx0, top, rx1, top + 26), fill=PANEL)
    d.text((rx0 + 12, top + 13), "$ ofs refract refract_gif.ofs", font=FS, fill=FG, anchor="lm")
    shown = [(tt, l) for tt, l in lines if tt <= t]
    rows = (bot - top - 40) // 18
    shown = shown[-rows:]
    y = top + 36
    for _, l in shown:
        if l.startswith("ofs refract:") and "refratada" in l: c = AMBER
        elif l.startswith("ofs refract:"): c = CYAN
        else: c = FG
        txt = l if len(l) < 46 else l[:45] + "…"
        d.text((rx0 + 12, y), txt, font=F, fill=c)
        y += 18
    # relogio
    d.text((rx1 - 10, bot - 10), f"{max(t, 0):4.1f} s", font=FS, fill=DIM, anchor="rd")
    return im

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(ROOT / "docs/screenshots/refract.gif"))
    ap.add_argument("--keep-log", default=None, help="grava o log bruto (json)")
    ap.add_argument("--from-log", default=None, help="redesenha a partir de um log gravado")
    a = ap.parse_args()
    if a.from_log: data = json.loads(Path(a.from_log).read_text())
    else: data = record(a.keep_log or os.devnull)
    n, dur = render(data, a.out)
    print(f"{a.out}: {n} quadros, {dur:.1f} s, {Path(a.out).stat().st_size/1024:.0f} KB")
