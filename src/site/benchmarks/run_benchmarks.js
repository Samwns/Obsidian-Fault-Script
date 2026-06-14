#!/usr/bin/env node

const { spawnSync } = require('node:child_process');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');

const root = path.resolve(__dirname, '..', '..', '..');
const benchDir = path.join(root, 'src/site', 'benchmarks');
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
      OFS_STDLIB: path.join(root, 'ofs', 'dist', 'stdlib'),
      OFS_CLANG_OPT: '-O3',
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
    available: () => fs.existsSync(path.join(root, 'ofs', 'dist', 'ofs')),
    compile: ['ofs/dist/ofs', ['build', 'src/site/benchmarks/src/cpu.ofs', '-o', 'src/site/benchmarks/build/cpu-ofs']],
    compileHello: ['ofs/dist/ofs', ['build', 'src/site/benchmarks/src/hello.ofs', '-o', 'src/site/benchmarks/build/hello-ofs']],
    cpu: [path.join(buildDir, 'cpu-ofs'), []],
    hello: [path.join(buildDir, 'hello-ofs'), []],
    artifact: path.join(buildDir, 'cpu-ofs'),
  },
  {
    name: 'C',
    available: () => commandExists('gcc'),
    compile: ['gcc', ['-O3', '-std=c11', 'src/site/benchmarks/src/cpu.c', '-o', 'src/site/benchmarks/build/cpu-c']],
    compileHello: ['gcc', ['-O3', '-std=c11', 'src/site/benchmarks/src/hello.c', '-o', 'src/site/benchmarks/build/hello-c']],
    cpu: [path.join(buildDir, 'cpu-c'), []],
    hello: [path.join(buildDir, 'hello-c'), []],
    artifact: path.join(buildDir, 'cpu-c'),
  },
  {
    name: 'C++',
    available: () => commandExists('g++'),
    compile: ['g++', ['-O3', '-std=c++17', 'src/site/benchmarks/src/cpu.cpp', '-o', 'src/site/benchmarks/build/cpu-cpp']],
    compileHello: ['g++', ['-O3', '-std=c++17', 'src/site/benchmarks/src/hello.cpp', '-o', 'src/site/benchmarks/build/hello-cpp']],
    cpu: [path.join(buildDir, 'cpu-cpp'), []],
    hello: [path.join(buildDir, 'hello-cpp'), []],
    artifact: path.join(buildDir, 'cpu-cpp'),
  },
  {
    name: 'Java',
    available: () => commandExists('javac') && commandExists('java'),
    prepare: () => fs.mkdirSync(path.join(buildDir, 'java'), { recursive: true }),
    compile: ['javac', ['-d', 'src/site/benchmarks/build/java', 'src/site/benchmarks/src/CpuBench.java']],
    compileHello: ['javac', ['-d', 'src/site/benchmarks/build/java', 'src/site/benchmarks/src/HelloBench.java']],
    cpu: ['java', ['-cp', path.join(buildDir, 'java'), 'CpuBench']],
    hello: ['java', ['-cp', path.join(buildDir, 'java'), 'HelloBench']],
    artifact: path.join(buildDir, 'java', 'CpuBench.class'),
  },
  {
    name: 'C#',
    available: () => commandExists('dotnet'),
    compile: ['dotnet', ['build', 'src/site/benchmarks/src/csharp/CpuBench.csproj', '-c', 'Release', '-o', 'src/site/benchmarks/build/csharp', '--no-incremental', '--nologo', '-v:q']],
    compileHello: ['dotnet', ['build', 'src/site/benchmarks/src/csharp-hello/HelloBench.csproj', '-c', 'Release', '-o', 'src/site/benchmarks/build/csharp-hello', '--no-incremental', '--nologo', '-v:q']],
    cpu: ['dotnet', [path.join(buildDir, 'csharp', 'CpuBench.dll')]],
    hello: ['dotnet', [path.join(buildDir, 'csharp-hello', 'HelloBench.dll')]],
    artifact: path.join(buildDir, 'csharp', 'CpuBench.dll'),
  },
  {
    name: 'Python',
    available: () => commandExists('python3'),
    prepare: () => fs.mkdirSync(path.join(buildDir, 'python'), { recursive: true }),
    compile: ['python3', ['-m', 'py_compile', 'src/site/benchmarks/src/cpu.py']],
    compileHello: ['python3', ['-m', 'py_compile', 'src/site/benchmarks/src/hello.py']],
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
    compile: ['rustc', ['-C', 'opt-level=3', 'src/site/benchmarks/src/cpu.rs', '-o', 'src/site/benchmarks/build/cpu-rust']],
    compileHello: ['rustc', ['-C', 'opt-level=3', 'src/site/benchmarks/src/hello.rs', '-o', 'src/site/benchmarks/build/hello-rust']],
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

  const compileRuns = samples(language.compile[0], language.compile[1], 5);
  run(language.compileHello[0], language.compileHello[1]);
  const cpuRuns = samples(language.cpu[0], language.cpu[1], 5);
  const startupRuns = samples(language.hello[0], language.hello[1], 15);
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
    cpu: 'Fibonacci iterativo com n variando de 25 a 32 por 5.000.000 iteracoes. Cinco processos; menor e melhor.',
    memory: 'Pico de memoria residente reportado por wait4/getrusage no processo de CPU. Cinco execucoes; menor e melhor.',
    startup: 'Processo hello-world minimo, quinze execucoes. Inclui startup de runtime/VM; menor e melhor.',
    compile: 'Build otimizado do codigo de CPU, cinco execucoes. Python mede compilacao para bytecode; menor e melhor.',
    artifact: 'Bytes do executavel, class, DLL ou bytecode principal. Dependencias de runtime nao entram no tamanho.',
    warning: 'Medicoes locais desta maquina e deste workload. Nao sao rankings universais de linguagens.',
  },
  expected_checksum: expectedOutput,
  results,
};

fs.writeFileSync(resultsFile, `${JSON.stringify(payload, null, 2)}\n`);
console.log(`Benchmark written to ${path.relative(root, resultsFile)}`);
