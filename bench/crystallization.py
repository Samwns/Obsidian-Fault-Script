#!/usr/bin/env python3
"""Compare AOT, AST, and cold/warm crystallization, including startup costs.

Usage: python3 bench/crystallization.py [--runs 3] [--output report.json]
OFSCC and OFS_RUNTIME override the repository compiler and runtime.
"""
import argparse
import json
import os
from pathlib import Path
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runs', type=int, default=3)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.runs < 1:
        parser.error('--runs must be positive')
    env = os.environ.copy()
    env.update(OFSCC=env.get('OFSCC', str(ROOT / 'ofs/dist/ofscc')),
               OFS_RUNTIME=env.get('OFS_RUNTIME', str(ROOT / 'ofs/dist/magma.o')),
               OFS_STDLIB_PATH=str(ROOT / 'ofs/stdlib'), NO_COLOR='1',
               OFS_PROGRESS='plain', OFS_STRICT_FAULT='1', OFS_VERBOSE='0',
               OFS_CRYSTALLIZE_THRESHOLD='1000')
    expected = (ROOT / 'bench/expected/fib.txt').read_bytes().strip()

    def run(command, run_env):
        start = time.perf_counter()
        result = subprocess.run(
            ['bash', '-c', 'ulimit -v 3000000; exec timeout "$@"',
             'ofs-bench', '120', *map(str, command)],
            cwd=ROOT, env=run_env, capture_output=True, check=True)
        if result.stdout.strip() != expected:
            raise RuntimeError('Benchmark output mismatch: ' + repr(result.stdout))
        return round((time.perf_counter() - start) * 1000, 2)

    with tempfile.TemporaryDirectory(prefix='ofs-crystallization-') as work:
        work = Path(work)
        source = ROOT / 'bench/ofs/fib.ofs'
        build_env = env.copy()
        build_env.update(OFSCC_MODE='ir', OFSCC_INPUT=str(source),
                         OFSCC_C_OUT=str(work / 'fib.ll'))
        subprocess.run(['bash', '-c', 'ulimit -v 3000000; exec timeout 120 "$OFSCC"'],
                       env=build_env, cwd=ROOT, capture_output=True, check=True)
        # Same optimization level as the crystallization worker.
        cc = env.get('OFS_QUARTZ_CC', 'clang')
        version = subprocess.check_output([cc, '--version'], text=True)
        flags = ['-mllvm', '-opaque-pointers'] if 'version 14.' in version else []
        subprocess.run(['timeout', '120', cc, *flags, '-Wno-override-module',
                        '-O2', str(work / 'fib.ll'), env['OFS_RUNTIME'], '-lm',
                        '-o', str(work / 'fib')], check=True, capture_output=True)
        samples = {}
        samples['aot'] = [run([work / 'fib'], env) for _ in range(args.runs)]
        env.update(OFSCC_MODE='run', OFSCC_INPUT=str(source), OFS_CRYSTALLIZE='0')
        samples['ast'] = [run([env['OFSCC']], env) for _ in range(args.runs)]
        env['OFS_CRYSTALLIZE'] = '1'
        samples['cold'] = []
        samples['warm'] = []
        for i in range(args.runs):
            env['OFS_QUARTZ_CACHE'] = str(work / ('cache-' + str(i)))
            samples['cold'].append(run([env['OFSCC']], env))
            samples['warm'].append(run([env['OFSCC']], env))
        report = {'benchmark': 'fib(35)', 'runs': args.runs,
                  'compiler': env['OFSCC'], 'clang': version.splitlines()[0],
                  'threshold': 1000, 'includes_startup': True,
                  'milliseconds': samples,
                  'median_ms': {k: round(statistics.median(v), 2)
                                for k, v in samples.items()},
                  'limitations': 'Call-count promotion; no OSR of active loops. '
                                 'Warm means cached native module, not zero startup overhead.'}
        data = json.dumps(report, ensure_ascii=False, indent=2) + '\n'
        print(data, end='')
        if args.output:
            args.output.write_text(data)


if __name__ == '__main__':
    main()
