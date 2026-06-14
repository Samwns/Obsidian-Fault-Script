#!/usr/bin/env node

const { spawnSync } = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');

const root = path.resolve(__dirname, '..', '..');
const benchDir = path.join(root, 'website', 'benchmarks');
const sourceDir = path.join(benchDir, 'src');
const buildDir = path.join(benchDir, 'build');
const runner = path.join(buildDir, 'bench-runner');
const resultsFile = path.join(benchDir, 'results.json');

fs.rmSync(buildDir, { recursive: true, force: true });
fs.mkdirSync(buildDir, { recursive: true });

function commandExists(command) {
  return spawnSync('/bin/sh', ['-c', `command -v ${command}`], {
    stdio: 'ignore',
  }).status === 0;
}

function run(command, args, options = {}) {
  const result = spawnSync(command, args, {
    cwd: root,
    encoding: 'utf8',
    env: {
      ...process.env,
      OFS_STDLIB: path.join(root, 'dist', 'stdlib'),
    },
    maxBuffer: 32 * 1024 * 1024,
    ...options,
  });
  if (result.status !== 0) {
    throw new Error(`${command} ${args.join(' ')}\n${result.stdout}\n${result.stderr}`);
  }
  return result;
}

run('gcc', ['-O2', '-std=c11', path.join(benchDir, 'runner.c'), '-o', runner]);

function measured(command, args) {
  const result = run(runner, [command, ...args]);
  const lines = result.stdout.trim().split(/\r?\n/);
  const metrics = lines.pop().split(',');
  return {
    wall_ms: Number(metrics[0]),
    max_rss_kb: Number(metrics[1]),
    output: lines.join('\n').trim(),
  };
}

function samples(command, args, count) {
  const values = [];
  for (let index = 0; index < count; index += 1) {
    values.push(measured(command, args));
  }
  return values;
}

function summarize(values, field) {
  const ordered = values.map((value) => value[field]).sort((a, b) => a - b);
  const sum = ordered.reduce((total, value) => total + value, 0);
  return {
    best: Number(ordered[0].toFixed(4)),
    median: Number(ordered[Math.floor(ordered.length / 2)].toFixed(4)),
    mean: Number((sum / ordered.length).toFixed(4)),
    max: Number(ordered[ordered.length - 1].toFixed(4)),
  };
}

function fileSize(file) {
  if (typeof file === 'function') file = file();
  return fs.existsSync(file) ? fs.statSync(file).size : null;
}

const languages = [
  {
    name: 'OFS',
    available: () => fs.existsSync(path.join(root, 'dist', 'ofs')),
    compile: ['dist/ofs', ['build', 'website/benchmarks/src/cpu.ofs', '-o', 'website/benchmarks/build/cpu-ofs']],
    compileHello: ['dist/ofs', ['build', 'website/benchmarks/src/hello.ofs', '-o', 'website/benchmarks/build/hello-ofs']],
    cpu: [path.join(buildDir, 'cpu-ofs'), []],
    hello: [path.join(buildDir, 'hello-ofs'), []],
    artifact: path.join(buildDir, 'cpu-ofs'),
  },
  {
    name: 'C',
    available: () => commandExists('gcc'),
    compile: ['gcc', ['-O2', '-std=c11', 'website/benchmarks/src/cpu.c', '-o', 'website/benchmarks/build/cpu-c']],
    compileHello: ['gcc', ['-O2', '-std=c11', 'website/benchmarks/src/hello.c', '-o', 'website/benchmarks/build/hello-c']],
    cpu: [path.join(buildDir, 'cpu-c'), []],
    hello: [path.join(buildDir, 'hello-c'), []],
    artifact: path.join(buildDir, 'cpu-c'),
  },
  {
    name: 'C++',
    available: () => commandExists('g++'),
    compile: ['g++', ['-O2', '-std=c++17', 'website/benchmarks/src/cpu.cpp', '-o', 'website/benchmarks/build/cpu-cpp']],
    compileHello: ['g++', ['-O2', '-std=c++17', 'website/benchmarks/src/hello.cpp', '-o', 'website/benchmarks/build/hello-cpp']],
    cpu: [path.join(buildDir, 'cpu-cpp'), []],
    hello: [path.join(buildDir, 'hello-cpp'), []],
    artifact: path.join(buildDir, 'cpu-cpp'),
  },
  {
    name: 'Java',
    available: () => commandExists('javac') && commandExists('java'),
    prepare: () => fs.mkdirSync(path.join(buildDir, 'java'), { recursive: true }),
    compile: ['javac', ['-d', 'website/benchmarks/build/java', 'website/benchmarks/src/CpuBench.java']],
    compileHello: ['javac', ['-d', 'website/benchmarks/build/java', 'website/benchmarks/src/HelloBench.java']],
    cpu: ['java', ['-cp', path.join(buildDir, 'java'), 'CpuBench']],
    hello: ['java', ['-cp', path.join(buildDir, 'java'), 'HelloBench']],
    artifact: path.join(buildDir, 'java', 'CpuBench.class'),
  },
  {
    name: 'C#',
    available: () => commandExists('dotnet'),
    compile: ['dotnet', ['build', 'website/benchmarks/src/csharp/CpuBench.csproj', '-c', 'Release', '-o', 'website/benchmarks/build/csharp', '--no-incremental', '--nologo', '-v:q']],
    compileHello: ['dotnet', ['build', 'website/benchmarks/src/csharp-hello/HelloBench.csproj', '-c', 'Release', '-o', 'website/benchmarks/build/csharp-hello', '--no-incremental', '--nologo', '-v:q']],
    cpu: ['dotnet', [path.join(buildDir, 'csharp', 'CpuBench.dll')]],
    hello: ['dotnet', [path.join(buildDir, 'csharp-hello', 'HelloBench.dll')]],
    artifact: path.join(buildDir, 'csharp', 'CpuBench.dll'),
  },
  {
    name: 'Python',
    available: () => commandExists('python3'),
    prepare: () => fs.mkdirSync(path.join(buildDir, 'python'), { recursive: true }),
    compile: ['python3', ['-m', 'py_compile', 'website/benchmarks/src/cpu.py']],
    compileHello: ['python3', ['-m', 'py_compile', 'website/benchmarks/src/hello.py']],
    cpu: ['python3', [path.join(sourceDir, 'cpu.py')]],
    hello: ['python3', [path.join(sourceDir, 'hello.py')]],
    artifact: () => {
      const cache = path.join(sourceDir, '__pycache__');
      const match = fs.readdirSync(cache).find((name) => name.startsWith('cpu.') && name.endsWith('.pyc'));
      return path.join(cache, match);
    },
  },
  {
    name: 'Rust',
    available: () => commandExists('rustc'),
    compile: ['rustc', ['-O', 'website/benchmarks/src/cpu.rs', '-o', 'website/benchmarks/build/cpu-rust']],
    compileHello: ['rustc', ['-O', 'website/benchmarks/src/hello.rs', '-o', 'website/benchmarks/build/hello-rust']],
    cpu: [path.join(buildDir, 'cpu-rust'), []],
    hello: [path.join(buildDir, 'hello-rust'), []],
    artifact: path.join(buildDir, 'cpu-rust'),
  },
];

const results = [];
let expectedOutput = null;

for (const language of languages) {
  if (!language.available()) {
    results.push({ language: language.name, available: false, reason: 'toolchain not installed' });
    continue;
  }

  if (language.prepare) language.prepare();

  const compileRuns = samples(language.compile[0], language.compile[1], 3);
  run(language.compileHello[0], language.compileHello[1]);
  const cpuRuns = samples(language.cpu[0], language.cpu[1], 3);
  const startupRuns = samples(language.hello[0], language.hello[1], 10);
  const output = cpuRuns[0].output;

  if (expectedOutput === null) expectedOutput = output;

  results.push({
    language: language.name,
    available: true,
    correct: output === expectedOutput,
    checksum: output,
    cpu_ms: summarize(cpuRuns, 'wall_ms'),
    memory_rss_kb: summarize(cpuRuns, 'max_rss_kb'),
    startup_ms: summarize(startupRuns, 'wall_ms'),
    compile_ms: summarize(compileRuns, 'wall_ms'),
    artifact_bytes: fileSize(language.artifact),
  });
}

const payload = {
  schema: 2,
  generated_at_utc: new Date().toISOString(),
  machine: {
    platform: os.platform(),
    release: os.release(),
    architecture: os.arch(),
    cpu: os.cpus()[0]?.model || 'unknown',
    logical_cores: os.cpus().length,
    total_memory_bytes: os.totalmem(),
    node: process.version,
  },
  methodology: {
    cpu: 'Iterative Fibonacci with n varying from 25 to 32 across 5,000,000 iterations. Three process runs; lower is better.',
    memory: 'Peak resident set size reported by wait4/getrusage for the CPU process. Three runs; lower is better.',
    startup: 'Minimal hello-world process, ten runs. Includes runtime/VM startup; lower is better.',
    compile: 'Optimized build of the CPU source, three runs. Python measures bytecode compilation; lower is better.',
    artifact: 'Bytes of the primary executable, class, DLL, or bytecode artifact. Runtime dependencies are not included.',
    warning: 'Local measurements describe this machine and workload only. They are not universal language rankings.',
  },
  expected_checksum: expectedOutput,
  results,
};

fs.writeFileSync(resultsFile, `${JSON.stringify(payload, null, 2)}\n`);
console.log(`Benchmark written to ${path.relative(root, resultsFile)}`);
