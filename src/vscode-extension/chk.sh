#!/bin/sh
# uso: chk.sh arquivo.ofs
ulimit -v 3000000
OFS_STDLIB_PATH="/home/samns/Arquivos/Projetos/Em Desolvovimento/OFS/Obsidian-Fault-Script/ofs/stdlib" OFSCC_MODE=check OFSCC_INPUT=$1 timeout 30 "/home/samns/Arquivos/Projetos/Em Desolvovimento/OFS/Obsidian-Fault-Script/ofs/dist/ofscc" 2>&1 | tr '\r' '\n' | grep -E "^\[|^done|-->" | head -5
