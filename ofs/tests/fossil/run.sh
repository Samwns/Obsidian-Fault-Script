#!/usr/bin/env bash
# ofs fossil (Ideia 2): grava uma execucao interpretada (entrada padrao,
# relogio, aleatorio, ambiente, arquivo e Rift) e reproduz com o mundo
# trocado: a saida tem de ser a mesma, o arquivo de saida nao e reescrito, ir a
# um passo/linha/vein mostra o valor certo das variaveis, --back e o modo
# interativo voltam no tempo, e um programa alterado avisa (ou diverge).
#   OFSCC_BIN=<ofscc>  (padrao: ofs/dist/ofscc; ligado com -rdynamic)
#   FOSSIL_KEEP=1      mantem a pasta temporaria
# Memoria: ulimit -v 3000000 e timeout em tudo.
set -uo pipefail
ulimit -v 3000000
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
CC="${OFSCC_BIN:-${OFSCC:-$R/ofs/dist/ofscc}}"
WRAP="$R/src/packaging/linux/ofs"
SRC="$R/ofs/tests/fossil/mundo.ofs"
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-fossil-XXXXXX")"
cleanup() {
    if [ -n "${FOSSIL_KEEP:-}" ]; then echo "fossil: arquivos em $T"; else rm -rf "$T"; fi
}
trap cleanup EXIT
export OFS_STDLIB_PATH="$R/ofs/stdlib" NO_COLOR=1 OFS_PROGRESS=plain OFS_STRICT_FAULT=1
unset OFS_FOSSIL OFS_FOSSIL_FILE OFS_FOSSIL_TO OFS_FOSSIL_BACK OFS_FOSSIL_DIG OFS_FOSSIL_LAYER OFS_FOSSIL_WORLD
PASS=0
FAIL=0

[ -x "$CC" ] || { echo "fossil: ofscc nao encontrado em $CC" >&2; exit 2; }
# copia propria: no fim nenhum processo com este caminho pode sobrar (camadas)
cp "$CC" "$T/ofscc-fossil"
CC="$T/ofscc-fossil"

ok() { echo "  ok   $1"; PASS=$((PASS + 1)); }
bad() { echo "  FALHA $1"; [ -n "${2:-}" ] && echo "$2" | sed 's/^/        /' | head -20; FAIL=$((FAIL + 1)); }

# grava <fonte> <fossil> <stdin> <saida> [env...]
grava() {
    local src="$1" fz="$2" in="$3" out="$4"
    shift 4
    env "$@" OFSCC_MODE=run OFS_FOSSIL=record OFS_FOSSIL_FILE="$fz" OFSCC_INPUT="$src" \
        timeout 60 "$CC" < "$in" > "$out" 2> "$out.err"
}

# reproduz <fossil> <saida> [env...]  (stdin vazio: nada do mundo e lido)
reproduz() {
    local fz="$1" out="$2"
    shift 2
    env "$@" OFSCC_MODE=run OFS_FOSSIL=replay OFS_FOSSIL_FILE="$fz" \
        timeout 60 "$CC" < /dev/null > "$out" 2> "$out.err"
}

echo "fossil: ofscc = $CC"

# ── 1. gravar ──────────────────────────────────────────────────────────
printf 'ab1\nsamuel\n' > "$T/entrada.txt"
echo "linha original" > "$T/dados.txt"
grava "$SRC" "$T/m.fossil" "$T/entrada.txt" "$T/gravado.out" \
    FOSSIL_CASA=pedra FOSSIL_DADOS="$T/dados.txt" FOSSIL_SAIDA="$T/saida.txt"
rc=$?
if [ "$rc" = 0 ] && grep -q "^ola samuel de pedra$" "$T/gravado.out" && grep -q "^cabeca 4 ab1$" "$T/gravado.out" \
    && grep -q "^linha linha original$" "$T/gravado.out" && grep -q "^visto: samuel$" "$T/saida.txt" \
    && grep -q "gravado em" "$T/gravado.out.err"; then
    ok "gravar: programa roda igual ao ofs run e grava o .fossil"
else
    bad "gravar (rc=$rc)" "$(cat "$T/gravado.out" "$T/gravado.out.err")"
fi
if head -1 "$T/m.fossil" | grep -q "^OFS-FOSSIL 1$" && grep -aq "^E clock_gettime " "$T/m.fossil" \
    && grep -aq "^E rand " "$T/m.fossil" && grep -aq "^E ofs_read_line 1 " "$T/m.fossil" \
    && grep -aq "^M 1 4$" "$T/m.fossil" && grep -aq "^F " "$T/m.fossil"; then
    ok "formato: cabecalho versionado, Rift (bind), aleatorio, entrada, efeito de memoria e fim"
else
    bad "formato do .fossil" "$(head -c 600 "$T/m.fossil")"
fi

# ── 2. reproduzir com o mundo trocado ──────────────────────────────────
rm -f "$T/saida.txt"
echo "outra coisa" > "$T/dados.txt"
reproduz "$T/m.fossil" "$T/replay.out" FOSSIL_CASA=vidro FOSSIL_DADOS="$T/dados.txt" FOSSIL_SAIDA="$T/saida.txt"
rc=$?
if [ "$rc" = 0 ] && cmp -s "$T/gravado.out" "$T/replay.out" && ! grep -q "aviso\|divergiu" "$T/replay.out.err"; then
    ok "replay: saida identica com entrada, relogio, aleatorio, ambiente e arquivo trocados"
else
    bad "replay (rc=$rc)" "$(diff "$T/gravado.out" "$T/replay.out"; cat "$T/replay.out.err")"
fi
if [ ! -e "$T/saida.txt" ]; then
    ok "replay: escrita em arquivo vira no-op (conferida pelo hash)"
else
    bad "replay escreveu o arquivo de saida"
fi
# a mesma coisa pelo wrapper (`ofs fossil replay`)
OFSCC="$CC" timeout 60 "$WRAP" fossil replay "$T/m.fossil" < /dev/null > "$T/wrap.out" 2> "$T/wrap.err"
if cmp -s "$T/gravado.out" "$T/wrap.out"; then
    ok "wrapper: ofs fossil replay"
else
    bad "wrapper" "$(diff "$T/gravado.out" "$T/wrap.out"; cat "$T/wrap.err")"
fi

# ── 3. ir a um ponto ──────────────────────────────────────────────────
peso0="$(sed -n 's/^rodada 0 peso //p' "$T/gravado.out")"
peso1="$(sed -n 's/^rodada 1 peso //p' "$T/gravado.out")"
reproduz "$T/m.fossil" "$T/to_vein.out" OFS_FOSSIL_TO=pesar
if grep -q "| vein pesar$" "$T/to_vein.out" && grep -q 'nome: obsidian = "samuel"' "$T/to_vein.out" \
    && grep -q "acumulado: stone = 0" "$T/to_vein.out" && ! grep -q "^rodada" "$T/to_vein.out"; then
    ok "--to <vein>: para na entrada da vein com locais e globais (sem a saida do programa)"
else
    bad "--to pesar" "$(cat "$T/to_vein.out" "$T/to_vein.out.err")"
fi
reproduz "$T/m.fossil" "$T/to_line.out" OFS_FOSSIL_TO=L22
passo_l22="$(sed -n 's/^fossil: passo \([0-9]*\) .*/\1/p' "$T/to_line.out")"
if grep -q "mundo.ofs:22 " "$T/to_line.out" && grep -q "peso: stone = $peso0$" "$T/to_line.out"; then
    ok "--to L<linha>: valor da variavel no ponto (peso = $peso0, passo $passo_l22)"
else
    bad "--to L22" "$(cat "$T/to_line.out" "$T/to_line.out.err")"
fi
reproduz "$T/m.fossil" "$T/to_step.out" OFS_FOSSIL_TO="$passo_l22"
if grep -q "^fossil: passo $passo_l22 " "$T/to_step.out" && grep -q "peso: stone = $peso0$" "$T/to_step.out"; then
    ok "--to <passo>: o mesmo ponto pelo numero do passo"
else
    bad "--to passo" "$(cat "$T/to_step.out" "$T/to_step.out.err")"
fi
reproduz "$T/m.fossil" "$T/to_file.out" OFS_FOSSIL_TO=mundo.ofs:22
if grep -q "^fossil: passo $passo_l22 " "$T/to_file.out"; then
    ok "--to <arquivo.ofs>:<linha>"
else
    bad "--to arquivo:linha" "$(cat "$T/to_file.out" "$T/to_file.out.err")"
fi

# ── 4. voltar no tempo (--back) ────────────────────────────────────────
alvo=$((passo_l22 + 25))
for camada in 7 2000; do
    reproduz "$T/m.fossil" "$T/back.out" OFS_FOSSIL_TO="$alvo" OFS_FOSSIL_BACK=25 OFS_FOSSIL_LAYER=$camada
    rc=$?
    if [ "$rc" = 0 ] && grep -q "^fossil: passo $passo_l22 " "$T/back.out" && grep -q "peso: stone = $peso0$" "$T/back.out" \
        && grep -q "acumulado: stone = 0" "$T/back.out"; then
        ok "--back 25 (camadas a cada $camada passos): volta ao passo $passo_l22 com o estado de antes"
    else
        bad "--back (camada $camada, rc=$rc)" "$(cat "$T/back.out" "$T/back.out.err")"
    fi
done

# ── 5. modo interativo (--dig) ─────────────────────────────────────────
printf 'ir L22\nver peso\nsegue 12\nver acumulado\ncava 12\nver acumulado\nver peso\nestrato\nsai\n' > "$T/cmds.txt"
env OFSCC_MODE=run OFS_FOSSIL=replay OFS_FOSSIL_FILE="$T/m.fossil" OFS_FOSSIL_DIG=1 OFS_FOSSIL_LAYER=5 \
    timeout 60 "$CC" < "$T/cmds.txt" > "$T/dig.out" 2> "$T/dig.err"
rc=$?
pesos="$(grep -c "peso: stone = $peso0$" "$T/dig.out")"
acum="$(grep -o "  acumulado: stone = [0-9]*" "$T/dig.out" | sed 's/.*= //' | tr '\n' ' ')"
dois=$((peso0 + peso1))
if [ "$rc" = 0 ] && [ "$pesos" -ge 3 ] && grep -q "rodada 0 peso $peso0$" "$T/dig.out" \
    && [ "$acum" = "$dois 0 0 " ]; then
    ok "--dig: ir/ver/segue/cava/estrato (acumulado $dois depois de seguir, 0 depois de cavar)"
else
    bad "--dig (rc=$rc, acumulado: $acum, esperado: $dois 0 0, pesos: $pesos)" "$(cat "$T/dig.out" "$T/dig.err")"
fi

# ── 6. programa alterado ──────────────────────────────────────────────
cp "$SRC" "$T/copia.ofs"
grava "$T/copia.ofs" "$T/c.fossil" "$T/entrada.txt" "$T/c.out" \
    FOSSIL_CASA=pedra FOSSIL_DADOS="$T/dados.txt" FOSSIL_SAIDA="$T/saida2.txt"
echo "// comentario novo" >> "$T/copia.ofs"
reproduz "$T/c.fossil" "$T/c2.out"
if grep -q "o programa mudou" "$T/c2.out.err" && cmp -s "$T/c.out" "$T/c2.out"; then
    ok "programa alterado (so comentario): avisa e reproduz igual"
else
    bad "aviso de programa alterado" "$(cat "$T/c2.out.err")"
fi
sed -i 's/    forge agora = time(null)/    forge extra = rand()\n    forge agora = time(null)/' "$T/copia.ofs"
reproduz "$T/c.fossil" "$T/c3.out"
rc=$?
if [ "$rc" = 3 ] && grep -q "o programa mudou" "$T/c3.out.err" && grep -q "divergiu no evento 1" "$T/c3.out.err"; then
    ok "programa alterado (outra chamada externa): diverge com rc 3 e diz onde"
else
    bad "divergencia (rc=$rc)" "$(cat "$T/c3.out.err")"
fi

# ── 7. nenhum processo de camada sobra ────────────────────────────────
sobra="$(pgrep -f -- "$T/ofscc-fossil" 2>/dev/null | wc -l)"
if [ "$sobra" = 0 ]; then
    ok "camadas encerradas (nenhum processo sobrando)"
else
    bad "processos sobrando: $sobra"
fi

echo "fossil: $PASS ok, $FAIL falha(s)"
[ "$FAIL" = 0 ]
