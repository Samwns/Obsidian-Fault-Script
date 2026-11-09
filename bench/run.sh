#!/usr/bin/env bash
# Benchmarks reproduzíveis do OFS. Uso: bash bench/run.sh
# Variáveis: RUNS=5 (medianas), INTERP_RUNS=1, INTERP_TIMEOUT=120 (s),
#            NATIVE_TIMEOUT=300 (s), MEM_LIMIT_MB=2048 (ulimit -v de cada
#            processo medido; um programa que estoura sai como "memória")
# Usa o compilador e o runtime do repositório (ofs/dist/ofscc, ofs/dist/magma.o,
# ofs/stdlib), não o OFS instalado no sistema.
# Gera bench/RESULTADOS.md e bench/results.json (o site copia o JSON para
# docs/assets/benchmark.json; ver src/site/src/build_site.ofs).
set -u
cd "$(dirname "${BASH_SOURCE[0]}")"
B="$PWD"; ROOT="$(cd .. && pwd)"
RUNS="${RUNS:-5}"; INTERP_RUNS="${INTERP_RUNS:-1}"; INTERP_TIMEOUT="${INTERP_TIMEOUT:-120}"
NATIVE_TIMEOUT="${NATIVE_TIMEOUT:-300}"; MEM_LIMIT_MB="${MEM_LIMIT_MB:-2048}"
OUT="$(mktemp -d)"; trap 'rm -rf "$OUT"' EXIT
# compilador, runtime, stdlib, wrapper e fontes copiados para um caminho sem espaços
mkdir -p $OUT/tool
cp "$ROOT/ofs/dist/ofscc" "$ROOT/ofs/dist/magma.o" $OUT/tool/ && cp -r "$ROOT/ofs/stdlib" $OUT/tool/ \
  || { echo "ofs/dist/ofscc, ofs/dist/magma.o ou ofs/stdlib ausente"; exit 1; }
export OFSCC=$OUT/tool/ofscc OFS_RUNTIME=$OUT/tool/magma.o OFS_STDLIB_PATH=$OUT/tool/stdlib NO_COLOR=1 OFS_PROGRESS=plain
# o wrapper `ofs` limita memória/CPU por conta própria; alinha com os limites daqui
export OFSCC_MAX_MEM=$((MEM_LIMIT_MB * 1024 * 1024)) OFSCC_MAX_CPU_SECONDS=$((INTERP_TIMEOUT + 60))
printf '#!/usr/bin/env bash\nexec bash "%s" "$@"\n' "$ROOT/src/packaging/linux/ofs" > $OUT/ofsw; chmod +x $OUT/ofsw
OFS=$OUT/ofsw
mkdir -p $OUT/src; cp -r ofs python $OUT/src/; [ -d c ] && cp -r c $OUT/src/; [ -d rust ] && cp -r rust $OUT/src/; [ -d go ] && cp -r go $OUT/src/
S=$OUT/src; HAVE_C=0; [ -d c ] && command -v clang >/dev/null && HAVE_C=1
BENCHES="empty fib loop strings sort alloc"
LANGS="ofs"
[ $HAVE_C = 1 ] && LANGS="$LANGS c"
[ -d c ] || echo "referência em C ausente (bench/c/): coluna C omitida"
command -v rustc >/dev/null && LANGS="$LANGS rust"
command -v go >/dev/null && [ -d go ] && LANGS="$LANGS go"
command -v python3 >/dev/null && LANGS="$LANGS python"
HAVE_TIME=0; [ -x /usr/bin/time ] && HAVE_TIME=1

for b in $BENCHES; do
  ( ulimit -v $((MEM_LIMIT_MB * 1024)); $OFS build $S/ofs/$b.ofs -o $OUT/ofs_$b ) >/dev/null 2>&1 || echo "falha ao compilar ofs/$b"
  [ $HAVE_C = 1 ] && { clang -O2 -o $OUT/c_$b $S/c/$b.c || exit 1; }
  command -v rustc >/dev/null && rustc -O -o $OUT/rust_$b $S/rust/$b.rs 2>/dev/null
  [[ " $LANGS " == *" go "* ]] && [ -f go/$b.go ] && go build -o $OUT/go_$b $S/go/$b.go
done

cmd_for() { # lang bench
  case $1 in
    python) echo "python3 $S/python/$2.py";;
    ofs-interp) echo "$OFS run $S/ofs/$2.ofs";;
    *) echo "$OUT/$1_$2";;
  esac
}
# Mensagens de falta de memória: runtime OFS ("ofs: sem memoria"), Python
# (MemoryError), Rust ("memory allocation of N bytes failed"), libc/C++.
OOM_RE='sem memoria|out of memory|MemoryError|memory allocation of|cannot allocate memory|bad_alloc'
# measure lang bench runs timeout -> "STATUS VALOR"
#   STATUS: ok | differs (VALOR = mediana em ms) | timeout | memory | failure (VALOR = rc)
# A saída da última execução é comparada com bench/expected/<bench>.txt.
measure() {
  local cmd; cmd=$(cmd_for $1 $2); local ts=() i s e rc
  for ((i=0;i<$3;i++)); do
    s=$(date +%s%N)
    ( ulimit -v $((MEM_LIMIT_MB * 1024)); exec timeout $4 $cmd ) >$OUT/o.txt 2>$OUT/e.txt
    rc=$?; e=$(date +%s%N)
    if [ $rc != 0 ]; then
      if [ $rc = 124 ]; then echo "timeout 0"
      elif grep -qiE "$OOM_RE" $OUT/e.txt || [ $rc = 137 ]; then echo "memory 0"
      else echo "failure $rc"; fi
      return
    fi
    ts+=($(( (e-s)/1000000 )))
  done
  local med; med=$(printf '%s\n' "${ts[@]}" | sort -n | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}')
  if cmp -s $OUT/o.txt "$B/expected/$2.txt"; then echo "ok $med"; else echo "differs $med"; fi
}
rss() { # lang bench -> MB (ou vazio)
  [ $HAVE_TIME = 1 ] || return
  local cmd; cmd=$(cmd_for $1 $2)
  ( ulimit -v $((MEM_LIMIT_MB * 1024)); /usr/bin/time -v timeout $NATIVE_TIMEOUT $cmd 2>&1 >/dev/null ) | awk '/Maximum resident/{printf "%.1f", $6/1024}'
}
jstr() { # string JSON
  local s; s=$(printf '%s' "$1" | tr -d '\000-\037'); s=${s//\\/\\\\}; s=${s//\"/\\\"}; printf '"%s"' "$s"
}
jnum() { [ -n "$1" ] && printf '%s' "$1" || printf 'null'; }
label() { # status valor -> texto da tabela
  case $1 in
    ok) echo "$2";; differs) echo "$2 (saída difere)";;
    timeout) echo "timeout (>${3}s)";; memory) echo "memória (>${MEM_LIMIT_MB} MB)";;
    *) echo "falha (rc=$2)";;
  esac
}

DATE=$(date -u +%Y-%m-%d); NOW=$(date -u +%Y-%m-%dT%H:%M:%SZ)
CPU=$(lscpu | awk -F: '/Model name/{gsub(/^ +/,"",$2);print $2; exit}'); THREADS=$(nproc); OSV=$(uname -sr); ARCH=$(uname -m)
MEMTOT=$(awk '/MemTotal/{printf "%.1f", $2/1048576}' /proc/meminfo 2>/dev/null)
V_OFS=$($OFS version 2>&1 | grep -aoE 'v?[0-9]+\.[0-9]+\.[0-9]+' | head -1)
V_CLANG=$(clang --version | head -1)
V_RUST=""; command -v rustc >/dev/null && V_RUST=$(rustc --version)
V_GO=""; [[ " $LANGS " == *" go "* ]] && V_GO=$(go version)
V_PY=""; command -v python3 >/dev/null && V_PY=$(python3 --version 2>&1)
jver() { [ -n "$1" ] && jstr "$1" || printf 'null'; }

R="$B/RESULTADOS.md"; J="$B/results.json"; RT="$OUT/RESULTADOS.md"; JT="$OUT/results.json"
COLS="ofs ofs-interp"; for l in c rust go python; do [[ " $LANGS " == *" $l "* ]] && COLS="$COLS $l"; done
{
echo "# Resultados dos benchmarks"; echo
echo "- Data: $DATE"
echo "- CPU: $CPU ($THREADS threads)"
echo "- SO: $OSV"
echo "- OFS: $V_OFS (ofs/dist/ofscc + ofs/dist/magma.o do repositório)"
echo "- clang: $V_CLANG"
[ -n "$V_RUST" ] && echo "- rustc: $V_RUST"
[ -n "$V_GO" ] && echo "- go: $V_GO"
[ -n "$V_PY" ] && echo "- python: $V_PY"
echo "- Método: mediana de $RUNS execuções do tempo de parede (ms); OFS interpretado: $INTERP_RUNS execução(ões), limite de ${INTERP_TIMEOUT}s; nativos: limite de ${NATIVE_TIMEOUT}s; limite de ${MEM_LIMIT_MB} MB de memória (ulimit -v) por processo."
echo "- Reproduzir: \`bash bench/run.sh\`"
echo
echo "## Tempo de parede (ms, menor é melhor)"; echo
printf '| Benchmark | OFS nativo | OFS interpretado |'; [[ " $LANGS " == *" c "* ]] && printf ' C -O2 |'
for l in rust go python; do [[ " $LANGS " == *" $l "* ]] && printf ' %s |' $l; done; echo
printf '|---|---|---|'; [[ " $LANGS " == *" c "* ]] && printf -- '---|'; for l in rust go python; do [[ " $LANGS " == *" $l "* ]] && printf -- '---|'; done; echo
} > "$RT"

LANG_LABELS='{"ofs": "OFS nativo", "ofs-interp": "OFS interpretado", "c": "C -O2", "rust": "Rust -O", "go": "Go", "python": "Python 3"}'
{
echo '{'
echo '  "schema": 3,'
echo "  \"date\": $(jstr "$DATE"),"
echo "  \"generated_at_utc\": $(jstr "$NOW"),"
echo "  \"machine\": {\"cpu\": $(jstr "$CPU"), \"threads\": $THREADS, \"os\": $(jstr "$OSV"), \"arch\": $(jstr "$ARCH"), \"memory_gb\": $(jnum "$MEMTOT")},"
echo "  \"versions\": {\"ofs\": $(jver "$V_OFS"), \"clang\": $(jver "$V_CLANG"), \"rustc\": $(jver "$V_RUST"), \"go\": $(jver "$V_GO"), \"python\": $(jver "$V_PY")},"
echo "  \"method\": {\"metric\": \"mediana do tempo de parede\", \"unit\": \"ms\", \"runs\": $RUNS, \"interp_runs\": $INTERP_RUNS, \"interp_timeout_s\": $INTERP_TIMEOUT, \"native_timeout_s\": $NATIVE_TIMEOUT, \"mem_limit_mb\": $MEM_LIMIT_MB, \"c_flags\": \"clang -O2\", \"rust_flags\": \"rustc -O\", \"reproduce\": \"bash bench/run.sh\"},"
printf '  "benchmarks": ['; f=1; for b in $BENCHES; do [ $f = 1 ] || printf ', '; f=0; printf '"%s"' $b; done; echo '],'
printf '  "languages": ['; f=1; for l in $COLS; do [ $f = 1 ] || printf ', '; f=0; printf '"%s"' $l; done; echo '],'
echo "  \"language_labels\": $LANG_LABELS,"
echo '  "results": {'
} > "$JT"

first=1
for b in $BENCHES; do
  row="| $b |"; jr=""
  for l in $COLS; do
    if [ $l = ofs-interp ]; then set -- $(measure $l $b $INTERP_RUNS $INTERP_TIMEOUT); to=$INTERP_TIMEOUT
    else set -- $(measure $l $b $RUNS $NATIVE_TIMEOUT); to=$NATIVE_TIMEOUT; fi
    st=$1; val=$2; txt=$(label $st $val $to)
    row="$row $txt |"
    case $st in
      ok|differs) j="{\"status\": \"$st\", \"ms\": $val, \"text\": $(jstr "$txt")}";;
      failure) j="{\"status\": \"failure\", \"ms\": null, \"rc\": $val, \"text\": $(jstr "$txt")}";;
      *) j="{\"status\": \"$st\", \"ms\": null, \"text\": $(jstr "$txt")}";;
    esac
    jr="$jr      \"$l\": $j,"$'\n'
  done
  echo "$row" >> "$RT"
  [ $first = 1 ] || echo "," >> "$JT"; first=0
  printf '    "%s": {\n%s    }' $b "${jr%,$'\n'}"$'\n' >> "$JT"
done
echo "" >> "$JT"; echo "  }," >> "$JT"

{
echo; echo "## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)"; echo
printf '| Linguagem | Binário vazio |'; for b in $BENCHES; do printf ' RSS %s |' $b; done; echo
printf '|---|---|'; for b in $BENCHES; do printf -- '---|'; done; echo
} >> "$RT"
JRSS=""; JSZ=""
for l in ofs c rust go python; do
  [[ " $LANGS " != *" $l "* ]] && continue
  if [ $l = python ]; then sz="n/a (interpretador)"; jsz=null
  else sz=$(stat -c %s $OUT/${l}_empty 2>/dev/null); jsz=$(jnum "$sz"); [ -n "$sz" ] || sz="n/d"; fi
  row="| $l | $sz |"; jl=""
  for b in $BENCHES; do
    m=$(rss $l $b); row="$row ${m:-n/d} |"; jl="$jl\"$b\": $(jnum "$m"), "
  done
  echo "$row" >> "$RT"
  JRSS="$JRSS    \"$l\": {${jl%, }},"$'\n'; JSZ="$JSZ\"$l\": $jsz, "
done
{
echo "  \"rss_mb\": {"; printf '%s' "${JRSS%,$'\n'}"; echo; echo "  },"
echo "  \"binary_size_bytes\": {${JSZ%, }}"
echo "}"
} >> "$JT"
{
echo
echo "A linha \`empty\` da tabela de tempo é o tempo de inicialização (programa que só imprime \`ok\`)."
echo "\"memória\" = o processo estourou o limite de ${MEM_LIMIT_MB} MB; \"timeout\" = passou do limite de tempo; \"falha\" = saiu com erro sem ser por memória."
} >> "$RT"
cp "$RT" "$R"; cp "$JT" "$J"
command -v python3 >/dev/null && { python3 -m json.tool "$J" >/dev/null || echo "AVISO: bench/results.json inválido"; }
cat "$R"
