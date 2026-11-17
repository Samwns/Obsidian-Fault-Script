#!/usr/bin/env bash
# Fault Refract (L0/L1 e onda 4): `ofs refract` troca ao vivo as veins
# editadas; onda 4: OSR (laco do main e laco nativo), migracao de monolith
# com instancias vivas e assinatura compativel (parametro com padrao).
# Cada caso roda o programa em segundo plano, espera pontos marcados na saida
# (tick=N), edita o arquivo por script e confere pela saida que a vein mudou
# sem reinicio e que o estado (contador global) continuou.
#   OFSCC_BIN=<ofscc>  (padrao: ofs/dist/ofscc; precisa ser ligado com -rdynamic)
#   REFRACT_KEEP=1     mantem a pasta temporaria (saidas de cada caso)
# Memoria: ulimit -v 3000000 e timeout em tudo.
set -uo pipefail
ulimit -v 3000000
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
CC="${OFSCC_BIN:-${OFSCC:-$R/ofs/dist/ofscc}}"
WRAP="$R/src/packaging/linux/ofs"
T="$(mktemp -d "${TMPDIR:-/tmp}/ofs-refract-XXXXXX")"
PIDS=()
cleanup() {
    local p
    for p in "${PIDS[@]}"; do kill "$p" 2>/dev/null; done
    wait 2>/dev/null
    if [ -n "${REFRACT_KEEP:-}" ]; then echo "refract: arquivos em $T"; else rm -rf "$T"; fi
}
trap cleanup EXIT
export OFS_STDLIB_PATH="$R/ofs/stdlib" NO_COLOR=1 OFS_PROGRESS=plain OFS_STRICT_FAULT=1
export OFS_QUARTZ_CACHE="$T/cache" OFS_REFRACT_PERIOD=50
unset OFS_CRYSTALLIZE_THRESHOLD OFS_QUARTZ_CC OFS_VERBOSE OFS_REFRACT_QUIET
PASS=0
FAIL=0

[ -x "$CC" ] || { echo "refract: ofscc nao encontrado em $CC" >&2; exit 2; }

# programa base: um laco que chama veins que o teste edita
base_program() { # base_program <arquivo>
    cat > "$1" <<'EOF'
claim rift
extern vein usleep(us: stone) -> stone

forge contador: stone = 0

vein versao() -> stone {
    return 1
}

vein mostrar(n: stone, v: stone) -> void {
    echo("tick=" + ofs_stone_to_obsidian(n) + " ver=" + ofs_stone_to_obsidian(v))
}

core main() {
    while (contador < 100000) {
        contador = contador + 1
        mostrar(contador, versao())
        usleep(50000)
    }
}
EOF
}

# espera_por <arquivo> <regex> [segundos]: 0 quando a regex aparece
espera_por() {
    local f="$1" re="$2" lim="${3:-20}" i=0
    while [ "$i" -lt $((lim * 20)) ]; do
        grep -qE "$re" "$f" 2>/dev/null && return 0
        sleep 0.05
        i=$((i + 1))
    done
    return 1
}

# inicia <dir> [env...]: roda app.ofs com refract em segundo plano
inicia() {
    local d="$1"; shift
    ( cd "$d" && exec env "$@" OFS_REFRACT=1 OFSCC_MODE=run OFSCC_INPUT=app.ofs timeout 90 "$CC" >out.txt 2>err.txt ) &
    PIDS+=("$!")
    LAST_PID="$!"
}

para() { kill "$LAST_PID" 2>/dev/null; wait "$LAST_PID" 2>/dev/null; }

# ticks 1, 2, 3... sem buraco nem recomeco (estado preservado, sem reinicio)
ticks_continuos() {
    awk '{ split($1, a, "="); if (a[2] != NR) bad = 1 } END { exit bad }' "$1/out.txt"
}

# uma unica execucao: a mensagem de inicio aparece uma vez so
sem_reinicio() {
    [ "$(grep -c 'observando' "$1/err.txt")" -le 1 ]
}

ok() { echo "  ok    $1"; PASS=$((PASS + 1)); }
falha() {
    echo "  FALHA $1: $2"
    echo "  --- stdout (fim) ---"; tail -n 5 "$3/out.txt" 2>/dev/null | sed 's/^/  /'
    echo "  --- stderr ---"; sed 's/^/  /' "$3/err.txt" 2>/dev/null
    FAIL=$((FAIL + 1))
}

# 1. L0: vein interpretada trocada pela interpretada nova
caso_interpretada() {
    local d="$T/interp"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha interpretada "nao comecou" "$d"; para; return; }
    sed -i 's/return 1$/return 2/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=2' || { falha interpretada "a vein nova nao entrou" "$d"; para; return; }
    espera_por "$d/err.txt" '1 vein refratada \(versao\)' 5 || { falha interpretada "sem mensagem de troca" "$d"; para; return; }
    sleep 0.2
    para
    ticks_continuos "$d" || { falha interpretada "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha interpretada "o programa reiniciou" "$d"; return; }
    ok "L0: vein interpretada trocada, estado preservado"
}

# 2. L1: vein cristalizada (nativa) recompilada num .so novo
caso_cristalizada() {
    if ! command -v "${OFS_QUARTZ_CC:-clang}" >/dev/null 2>&1; then
        echo "  pulado L1 (sem clang)"; return
    fi
    local d="$T/cristal"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=1 OFS_CRYSTALLIZE_THRESHOLD=3 OFS_VERBOSE=1
    espera_por "$d/err.txt" 'cristalizacao: .*vein\(s\) nativa' 60 || { falha cristalizada "nao cristalizou" "$d"; para; return; }
    sed -i 's/return 1$/return 3/' "$d/app.ofs"
    espera_por "$d/err.txt" 'recristalizando em segundo plano: .*versao' 20 || { falha cristalizada "nao recristalizou" "$d"; para; return; }
    espera_por "$d/out.txt" 'ver=3' 20 || { falha cristalizada "a vein nova nao entrou" "$d"; para; return; }
    espera_por "$d/err.txt" 'cristalizada\(s\) de novo' 60 || { falha cristalizada "o .so novo nao foi carregado" "$d"; para; return; }
    # depois do .so novo, a versao nativa nova continua dando 3
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 3)) ver=3" 20 || { falha cristalizada "a versao nativa nova nao responde" "$d"; para; return; }
    para
    grep -q 'ver=1' "$d/out.txt" || { falha cristalizada "nao rodou a versao antiga" "$d"; return; }
    ticks_continuos "$d" || { falha cristalizada "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha cristalizada "o programa reiniciou" "$d"; return; }
    ok "L1: vein cristalizada recompilada num .so novo, estado preservado"
}

# 3. erro de compilacao: a versao em execucao continua; corrigir aplica
caso_erro() {
    local d="$T/erro"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha erro "nao comecou" "$d"; para; return; }
    sed -i 's/return 1$/return 1 +/' "$d/app.ofs"
    espera_por "$d/err.txt" 'erro de compilacao' || { falha erro "sem mensagem de erro" "$d"; para; return; }
    grep -q 'ERRO SINTATICO' "$d/err.txt" || { falha erro "o diagnostico nao foi para o stderr" "$d"; para; return; }
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 3)) ver=1" || { falha erro "o programa parou com o erro" "$d"; para; return; }
    sed -i 's/return 1 +$/return 4/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=4' || { falha erro "a correcao nao entrou" "$d"; para; return; }
    para
    if grep -q 'ERRO' "$d/out.txt"; then falha erro "diagnostico no stdout do programa" "$d"; return; fi
    ticks_continuos "$d" || { falha erro "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha erro "o programa reiniciou" "$d"; return; }
    ok "erro de compilacao mantem a versao em execucao"
}

# 4. assinatura de vein em uso mudou: troca recusada, programa segue
caso_assinatura() {
    local d="$T/assin"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha assinatura "nao comecou" "$d"; para; return; }
    sed -i -e 's/vein versao() -> stone {/vein versao(k: stone) -> stone {/' -e 's/return 1$/return k/' \
        -e 's/mostrar(contador, versao())/mostrar(contador, versao(9))/' "$d/app.ofs"
    espera_por "$d/err.txt" 'troca recusada: a assinatura de .versao. mudou' || { falha assinatura "a troca nao foi recusada" "$d"; para; return; }
    grep -q "'main', em execucao" "$d/err.txt" || { falha assinatura "nao disse quem usa a vein" "$d"; para; return; }
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 3)) ver=1" || { falha assinatura "o programa nao seguiu com a versao antiga" "$d"; para; return; }
    para
    if grep -q 'ver=9' "$d/out.txt"; then falha assinatura "a versao recusada rodou" "$d"; return; fi
    ticks_continuos "$d" || { falha assinatura "o contador recomecou ou pulou" "$d"; return; }
    ok "mudanca de assinatura em uso recusada, versao antiga segue"
}

# 5. vein nova e global novo (o global existente mantem o valor)
caso_nova() {
    local d="$T/nova"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha nova "nao comecou" "$d"; para; return; }
    # contador ganha outro valor inicial no texto: o valor vivo continua
    sed -i -e 's/forge contador: stone = 0/forge contador: stone = 500\nforge bonus: stone = 5/' \
        -e 's/return 1$/return extra() + bonus/' "$d/app.ofs"
    printf '\nvein extra() -> stone {\n    return 70\n}\n' >> "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=75' || { falha nova "a vein nova nao entrou" "$d"; para; return; }
    espera_por "$d/err.txt" '1 vein nova \(extra\)' 5 || { falha nova "sem mensagem da vein nova" "$d"; para; return; }
    grep -q '1 global novo (bonus)' "$d/err.txt" || { falha nova "sem mensagem do global novo" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha nova "o contador recomecou ou pulou (global existente perdeu o valor)" "$d"; return; }
    sem_reinicio "$d" || { falha nova "o programa reiniciou" "$d"; return; }
    ok "vein nova e global novo entram ao vivo; global existente mantem o valor"
}

# 6. pelo wrapper `ofs refract --quiet`: sem avisos de troca, mas troca
caso_wrapper() {
    local d="$T/wrap"; mkdir -p "$d"; base_program "$d/app.ofs"
    ( cd "$d" && exec env OFSCC="$CC" OFS_RUNTIME="$R/ofs/dist/magma.o" OFS_CRYSTALLIZE=0 \
        timeout 90 bash "$WRAP" refract --quiet app.ofs >out.txt 2>err.txt ) &
    PIDS+=("$!"); LAST_PID="$!"
    espera_por "$d/out.txt" '^tick=3 ' || { falha wrapper "nao comecou" "$d"; para; return; }
    sed -i 's/return 1$/return 6/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=6' || { falha wrapper "a vein nova nao entrou" "$d"; para; return; }
    para
    if grep -q 'ofs refract:' "$d/err.txt"; then falha wrapper "--quiet nao silenciou" "$d"; return; fi
    ticks_continuos "$d" || { falha wrapper "o contador recomecou ou pulou" "$d"; return; }
    ok "ofs refract --quiet (wrapper)"
}

# programa com contratos (A5.1): mineral no parametro de `mostrar`
contract_program() { # contract_program <arquivo>
    cat > "$1" <<'EOF'
claim rift
extern vein usleep(us: stone) -> stone

mineral Ver = stone grain value > 0

forge contador: stone = 0

vein versao() -> stone {
    return 1
}

vein mostrar(n: stone, v: Ver) -> void bearing v < 10 {
    echo("tick=" + ofs_stone_to_obsidian(n) + " ver=" + ofs_stone_to_obsidian(v))
}

core main() {
    while (contador < 100000) {
        contador = contador + 1
        mostrar(contador, versao())
        usleep(50000)
    }
}
EOF
}

# 7. A5.1: bearing alterado conta como edicao da vein (entra ao vivo e vale)
caso_contrato() {
    local d="$T/contrato"; mkdir -p "$d"; contract_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha contrato "nao comecou" "$d"; para; return; }
    sed -i 's/bearing v < 10/bearing v > 5/' "$d/app.ofs"
    espera_por "$d/err.txt" '1 vein refratada \(mostrar\)' || { falha contrato "o contrato novo nao trocou a vein" "$d"; para; return; }
    espera_por "$d/err.txt" "CONTRATO VIOLADO .*bearing de 'mostrar' falhou: v > 5 \(v = 1\)" || { falha contrato "o bearing novo nao foi conferido" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha contrato "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha contrato "o programa reiniciou" "$d"; return; }
    ok "A5.1: bearing alterado troca a vein ao vivo e passa a valer"
}

# 8. A5.1: grain de um mineral usado na assinatura em uso = mudanca de tipo
caso_grain() {
    local d="$T/grain"; mkdir -p "$d"; contract_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha grain "nao comecou" "$d"; para; return; }
    sed -i 's/grain value > 0/grain value > -5/' "$d/app.ofs"
    espera_por "$d/err.txt" 'troca recusada: a assinatura de .mostrar. mudou' || { falha grain "a troca nao foi recusada" "$d"; para; return; }
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 3)) ver=1" || { falha grain "o programa nao seguiu com a versao antiga" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha grain "o contador recomecou ou pulou" "$d"; return; }
    ok "A5.1: grain de mineral em assinatura em uso e mudanca de tipo (recusada)"
}

# programa com geode (closure): criado uma vez no main, chamado no laco
geode_program() { # geode_program <arquivo>
    cat > "$1" <<'EOF'
claim rift
extern vein usleep(us: stone) -> stone

forge contador: stone = 0

vein mostrar(n: stone, v: stone) -> void {
    echo("tick=" + ofs_stone_to_obsidian(n) + " ver=" + ofs_stone_to_obsidian(v))
}

core main() {
    forge base = 1
    forge g = geode (x: stone) -> stone { return base + x }
    while (contador < 100000) {
        contador = contador + 1
        mostrar(contador, g(0))
        usleep(50000)
    }
}
EOF
}

# 9. geode: o corpo editado (mesmas capturas) troca ao vivo, inclusive no
#    geode ja criado; capturas diferentes = o geode ja criado segue antigo
caso_geode() {
    local d="$T/geode"; mkdir -p "$d"; geode_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha geode "nao comecou" "$d"; para; return; }
    sed -i 's/return base + x }/return base + x + 1 }/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=2' || { falha geode "o corpo novo do geode nao entrou" "$d"; para; return; }
    sed -i 's/return base + x + 1 }/return 7 + x }/' "$d/app.ofs"
    espera_por "$d/err.txt" 'refratad.*refratad' 5 || true
    sleep 1
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 4)) ver=2" || { falha geode "o geode ja criado devia seguir com o corpo antigo (capturas mudaram)" "$d"; para; return; }
    para
    grep -q 'ver=7' "$d/out.txt" && { falha geode "usou o corpo com outras capturas no geode antigo" "$d"; return; }
    ticks_continuos "$d" || { falha geode "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha geode "o programa reiniciou" "$d"; return; }
    ok "geode: corpo editado troca ao vivo; capturas novas so valem para geodes novos"
}

# ── onda 4 ────────────────────────────────────────────────────────────────
# programa com o laco no main e estado local (n, passo)
osr_program() { # osr_program <arquivo>
    cat > "$1" <<'EOF'
claim rift
extern vein usleep(us: stone) -> stone

forge contador: stone = 0

vein versao() -> stone {
    return 1
}

vein mostrar(n: stone, v: stone) -> void {
    echo("tick=" + ofs_stone_to_obsidian(n) + " ver=" + ofs_stone_to_obsidian(v))
}

core main() {
    forge passo: stone = 1
    forge n: stone = 0
    while (contador < 100000) {
        n = n + passo
        contador = n
        mostrar(n, versao())
        usleep(50000)
    }
}
EOF
}

# 10. OSR: o laco do main (sempre em execucao) passa para a versao nova no
#     proximo cabecalho, com as variaveis locais; variavel nova antes do
#     laco ganha o valor inicial
caso_osr_main() {
    local d="$T/osr_main"; mkdir -p "$d"; osr_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha osr_main "nao comecou" "$d"; para; return; }
    sed -i 's/mostrar(n, versao())/mostrar(n, versao() + 40)/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=41' || { falha osr_main "o laco do main nao trocou de versao" "$d"; para; return; }
    grep -q "laco de 'main' continua na versao nova (OSR)" "$d/err.txt" || { falha osr_main "sem mensagem da OSR" "$d"; para; return; }
    sed -i -e 's/    forge passo: stone = 1/    forge passo: stone = 1\n    forge extra: stone = 7/' \
        -e 's/versao() + 40)/versao() + 40 + extra)/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=48' || { falha osr_main "a variavel nova nao entrou com o valor inicial" "$d"; para; return; }
    grep -q 'variavel(is) nova(s) antes do laco, com o valor inicial: extra' "$d/err.txt" || { falha osr_main "sem mensagem da variavel nova" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha osr_main "o contador local recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha osr_main "o programa reiniciou" "$d"; return; }
    ok "OSR: laco do main troca de versao no meio, estado local preservado"
}

# 11. OSR nativo na Refract: o laco quente roda nativo (OSR), volta ao
#     interpretador pelo gancho quando o arquivo muda e vira nativo de novo
caso_osr_nativo() {
    if ! command -v "${OFS_QUARTZ_CC:-clang}" >/dev/null 2>&1; then
        echo "  pulado OSR nativo (sem clang)"; return
    fi
    local d="$T/osr_nat"; mkdir -p "$d"
    cat > "$d/app.ofs" <<'EOF'
core main() {
    forge n: stone = 0
    forge marca: stone = 0
    while (n < 4000000000) {
        n = n + 1
        if (n % 1000000 == 0) {
            marca = marca + 1
            echo("tick=" + ofs_stone_to_obsidian(marca) + " ver=1")
        }
    }
}
EOF
    inicia "$d" OFS_CRYSTALLIZE=1 OFS_VERBOSE=1
    espera_por "$d/err.txt" 'osr \(1 laco\)' 60 || { falha osr_nativo "o laco nao virou nativo" "$d"; para; return; }
    espera_por "$d/out.txt" '^tick=20 ' 20 || { falha osr_nativo "o laco nativo nao anda" "$d"; para; return; }
    sed -i 's/ ver=1")/ ver=2")/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=2' 20 || { falha osr_nativo "a edicao nao chegou ao laco nativo" "$d"; para; return; }
    grep -q "laco de 'main' continua na versao nova (OSR)" "$d/err.txt" || { falha osr_nativo "sem mensagem da OSR" "$d"; para; return; }
    local i=0
    while [ "$(grep -c 'osr (1 laco)' "$d/err.txt")" -lt 2 ] && [ "$i" -lt 600 ]; do sleep 0.05; i=$((i + 1)); done
    [ "$(grep -c 'osr (1 laco)' "$d/err.txt")" -ge 2 ] || { falha osr_nativo "a versao nova do laco nao virou nativa" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha osr_nativo "o contador local recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha osr_nativo "o programa reiniciou" "$d"; return; }
    ok "OSR nativo: laco nativo volta pelo gancho, troca e vira nativo de novo, com o estado"
}

# programa com monolith vivo (global e local)
mono_program() { # mono_program <arquivo>
    cat > "$1" <<'EOF'
claim rift
extern vein usleep(us: stone) -> stone

monolith Ponto {
    x: stone
    y: stone
}

forge contador: stone = 0
forge p: Ponto

vein mostrar(n: stone, q: Ponto) -> void {
    echo("tick=" + ofs_stone_to_obsidian(n) + " ver=" + ofs_stone_to_obsidian(q.x + q.y))
}

core main() {
    p.x = 1
    p.y = 2
    forge loc: Ponto
    loc.x = 30
    while (contador < 100000) {
        contador = contador + 1
        mostrar(contador, p)
        usleep(50000)
    }
}
EOF
}

# 12. migracao de monolith com instancias vivas: campo novo (padrao), ordem
#     nova, stone -> crystal converte; obsidian nao converte (recusa)
caso_monolith() {
    local d="$T/mono"; mkdir -p "$d"; mono_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha monolith "nao comecou" "$d"; para; return; }
    sed -i -e 's/^    x: stone$/    z: stone\n    y: crystal/' -e 's/^    y: stone$/    x: stone/' \
        -e 's/ofs_stone_to_obsidian(q.x + q.y))/ofs_stone_to_obsidian(q.x + q.y as stone + q.z + 100))/' "$d/app.ofs"
    espera_por "$d/out.txt" 'ver=103' || { falha monolith "as instancias nao migraram (x=1, y=2.0, z=0)" "$d"; para; return; }
    grep -q "monolith 'Ponto' migrado ao vivo (2 instancias); campo(s) novo(s) com o valor padrao: z; convertido(s): y stone->crystal" "$d/err.txt" || { falha monolith "sem relatorio da migracao" "$d"; para; return; }
    sed -i 's/^    x: stone$/    x: obsidian/' "$d/app.ofs"
    espera_por "$d/err.txt" "troca recusada: o monolith 'Ponto' .*nao converte \('x' de stone para obsidian\)" || { falha monolith "o tipo incompativel nao foi recusado" "$d"; para; return; }
    local n; n="$(wc -l < "$d/out.txt")"
    espera_por "$d/out.txt" "^tick=$((n + 3)) ver=103" || { falha monolith "o programa nao seguiu com o layout migrado" "$d"; para; return; }
    para
    ticks_continuos "$d" || { falha monolith "o contador recomecou ou pulou" "$d"; return; }
    sem_reinicio "$d" || { falha monolith "o programa reiniciou" "$d"; return; }
    ok "migracao de monolith com instancias vivas (campo novo, ordem, conversao; recusa o incompativel)"
}

# 13. assinatura compativel: parametro novo com valor padrao numa vein em uso
caso_assinatura_compat() {
    local d="$T/assin_ok"; mkdir -p "$d"; base_program "$d/app.ofs"
    inicia "$d" OFS_CRYSTALLIZE=0
    espera_por "$d/out.txt" '^tick=3 ' || { falha assinatura_compat "nao comecou" "$d"; para; return; }
    sed -i -e 's/vein versao() -> stone {/vein versao(k: stone = 7) -> stone {/' -e 's/return 1$/return k/' \
        -e 's/mostrar(contador, versao())/mostrar(contador, versao(9))/' "$d/app.ofs"
    espera_por "$d/err.txt" "a assinatura de 'versao' mudou de \(\)->stone para \(stone\)->stone: compativel \(1 parametro novo com padrao: k\)" || { falha assinatura_compat "a troca compativel nao foi aceita" "$d"; para; return; }
    espera_por "$d/out.txt" 'ver=9' || { falha assinatura_compat "a chamada nova nao entrou" "$d"; para; return; }
    para
    if grep -q 'troca recusada' "$d/err.txt"; then falha assinatura_compat "recusou a troca compativel" "$d"; return; fi
    ticks_continuos "$d" || { falha assinatura_compat "o contador recomecou ou pulou" "$d"; return; }
    ok "assinatura compativel (parametro novo com padrao) aceita numa vein em uso"
}

echo "== Fault Refract (ofs/tests/refract) =="
caso_interpretada
caso_cristalizada
caso_erro
caso_assinatura
caso_nova
caso_wrapper
caso_contrato
caso_grain
caso_geode
caso_osr_main
caso_osr_nativo
caso_monolith
caso_assinatura_compat
echo "refract: $PASS ok, $FAIL falha(s)"
[ "$FAIL" -eq 0 ]
