#!/usr/bin/env bash
# Benchmarks reproduzíveis do OFS. Uso: bash bench/run.sh
# Variáveis: RUNS=5 (medianas), INTERP_RUNS=1, INTERP_TIMEOUT=120 (s)
set -u
cd "$(dirname "${BASH_SOURCE[0]}")"
B="$PWD"
RUNS="${RUNS:-5}"; INTERP_RUNS="${INTERP_RUNS:-1}"; INTERP_TIMEOUT="${INTERP_TIMEOUT:-120}"
OUT="$(mktemp -d)"; trap 'rm -rf "$OUT"' EXIT
# wrapper e fontes copiados para um caminho sem espaços
printf '#!/usr/bin/env bash\nexec bash "%s" "$@"\n' "$B/../src/packaging/linux/ofs" > $OUT/ofsw; chmod +x $OUT/ofsw
OFS=$OUT/ofsw
mkdir -p $OUT/src; cp -r ofs python $OUT/src/; [ -d c ] && cp -r c $OUT/src/; [ -d rust ] && cp -r rust $OUT/src/; [ -d go ] && cp -r go $OUT/src/
S=$OUT/src; HAVE_C=0; [ -d c ] && command -v clang >/dev/null && HAVE_C=1
BENCHES="empty fib loop strings sort alloc"
LANGS="ofs"
[ -d c ] && command -v clang >/dev/null && LANGS="$LANGS c"
[ -d c ] || echo "referência em C ausente (bench/c/): coluna C omitida"
command -v rustc >/dev/null && LANGS="$LANGS rust"
command -v go >/dev/null && LANGS="$LANGS go"
command -v python3 >/dev/null && LANGS="$LANGS python"
HAVE_TIME=0; [ -x /usr/bin/time ] && HAVE_TIME=1

for b in $BENCHES; do
  $OFS build $S/ofs/$b.ofs -o $OUT/ofs_$b >/dev/null 2>&1 || echo "falha ao compilar ofs/$b"
  [ $HAVE_C = 1 ] && { clang -O2 -o $OUT/c_$b $S/c/$b.c || exit 1; }
  command -v rustc >/dev/null && rustc -O -o $OUT/rust_$b $S/rust/$b.rs 2>/dev/null
  command -v go >/dev/null && [ -f go/$b.go ] && go build -o $OUT/go_$b $S/go/$b.go
done

cmd_for() { # lang bench
  case $1 in
    python) echo "python3 $S/python/$2.py";;
    ofs-interp) echo "$OFS run $S/ofs/$2.ofs";;
    *) echo "$OUT/$1_$2";;
  esac
}
# mediana (ms) de N execuções; imprime "ms" ou "ERRO"
measure() { # lang bench runs timeout
  local cmd; cmd=$(cmd_for $1 $2); local ts=() i s e
  for ((i=0;i<$3;i++)); do
    s=$(date +%s%N)
    timeout $4 $cmd >$OUT/o.txt 2>/dev/null || { [ $? = 124 ] && echo ERRO_T || echo ERRO_F; return; }
    e=$(date +%s%N); ts+=($(( (e-s)/1000000 )))
  done
  printf '%s\n' "${ts[@]}" | sort -n | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}'
}
check() { # lang bench -> OK/DIFERE
  local cmd; cmd=$(cmd_for $1 $2)
  timeout ${3:-120} $cmd 2>/dev/null | cmp -s - "$B/expected/$2.txt" && echo OK || echo DIFERE
}
rss() { # lang bench -> MB
  [ $HAVE_TIME = 1 ] || { echo n/d; return; }
  local cmd; cmd=$(cmd_for $1 $2)
  /usr/bin/time -v timeout $INTERP_TIMEOUT $cmd 2>&1 >/dev/null | awk '/Maximum resident/{printf "%.1f", $6/1024}'
}

R="$B/RESULTADOS.md"; J="$B/results.json"; RT="$OUT/RESULTADOS.md"
{
echo "# Resultados dos benchmarks"; echo
echo "- Data: $(date -u +%Y-%m-%d)"
echo "- CPU: $(lscpu | awk -F: '/Model name/{gsub(/^ +/,"",$2);print $2}') ($(nproc) threads)"
echo "- SO: $(uname -sr)"
echo "- OFS: $($OFS version 2>&1 | grep -aoE 'v?[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
echo "- clang: $(clang --version | head -1)"
command -v rustc >/dev/null && echo "- rustc: $(rustc --version)"
command -v go >/dev/null && echo "- go: $(go version)"
command -v python3 >/dev/null && echo "- python: $(python3 --version)"
echo "- Método: mediana de $RUNS execuções do tempo de parede (ms); OFS interpretado: $INTERP_RUNS execução(ões), limite de ${INTERP_TIMEOUT}s."
echo "- Reproduzir: \`bash bench/run.sh\`"
echo
echo "## Tempo de parede (ms, menor é melhor)"; echo
printf '| Benchmark | OFS nativo | OFS interpretado |'; [[ " $LANGS " == *" c "* ]] && printf ' C -O2 |'
for l in rust go python; do [[ " $LANGS " == *" $l "* ]] && printf ' %s |' $l; done; echo
printf '|---|---|---|'; [[ " $LANGS " == *" c "* ]] && printf -- '---|'; for l in rust go python; do [[ " $LANGS " == *" $l "* ]] && printf -- '---|'; done; echo
echo '{' > "$J"; echo "  \"date\": \"$(date -u +%Y-%m-%d)\", \"unit\": \"ms\", \"results\": {" >> "$J"
first=1
for b in $BENCHES; do
  row="| $b |"; jr=""
  for l in ofs ofs-interp c rust go python; do
    [[ $l != ofs-interp && " $LANGS " != *" $l "* ]] && continue
    if [ $l = ofs-interp ]; then
      v=$(measure $l $b $INTERP_RUNS $INTERP_TIMEOUT)
      if [ "$v" = ERRO_T ]; then v="timeout (>${INTERP_TIMEOUT}s)"; elif [ "$v" = ERRO_F ]; then v="falha (saída 1, sem resultado)"; else [ "$(check $l $b $INTERP_TIMEOUT)" = OK ] || v="$v (saída difere)"; fi
    else
      v=$(measure $l $b $RUNS 300)
      [[ "$v" == ERRO* ]] || [ "$(check $l $b)" = OK ] || v="$v (saída difere)"
    fi
    row="$row $v |"; jr="$jr\"$l\": \"$v\", "
  done
  echo "$row" >> $OUT/table.md
  [ $first = 1 ] || echo "," >> "$J"; first=0
  printf '    "%s": {%s}' $b "${jr%, }" >> "$J"
done
cat $OUT/table.md
echo; echo "## Tamanho do binário (bytes, programa vazio) e memória máxima (MB)"; echo
echo "| Linguagem | Binário vazio | RSS vazio | RSS fib | RSS strings | RSS sort | RSS alloc |"
echo "|---|---|---|---|---|---|---|"
for l in ofs c rust go python; do
  [[ " $LANGS " != *" $l "* ]] && continue
  if [ $l = python ]; then sz="n/a (interpretador)"; else sz=$(stat -c %s $OUT/${l}_empty); fi
  echo "| $l | $sz | $(rss $l empty) | $(rss $l fib) | $(rss $l strings) | $(rss $l sort) | $(rss $l alloc) |"
done
echo
echo "A linha \`empty\` da tabela de tempo é o tempo de inicialização (programa que só imprime \`ok\`)."
} > "$RT"
cp "$RT" "$R"
echo "" >> "$J"; echo "  }" >> "$J"; echo "}" >> "$J"
cat "$R"
