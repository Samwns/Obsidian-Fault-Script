#!/bin/sh

# Keep compiler experiments below the memory needed by the editor and system.
ulimit -v 786432
ulimit -t 20
ulimit -f 524288

if command -v taskset >/dev/null 2>&1; then
    exec timeout --signal=TERM --kill-after=2s 45s taskset -c 0 "$@"
fi

exec timeout --signal=TERM --kill-after=2s 45s "$@"