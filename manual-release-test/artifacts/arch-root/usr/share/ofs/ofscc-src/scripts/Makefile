# OFS Compiler Makefile
# Convenient build targets for Unix-like systems

SHELL := /bin/bash
.PHONY: all clean help bootstrap test validate release

# Configuration
BUILD_DIR ?= build-ofscc
INSTALL_DIR ?= dist
JOBS ?= $(shell nproc || echo 4)
VERSION ?= 1.0.0
BUILD_TYPE ?= Release

# Compiler detection
OFSCC = $(shell if [ -f "$(INSTALL_DIR)/ofscc" ]; then echo "$(INSTALL_DIR)/ofscc"; elif [ -f "ofscc_v3" ]; then echo "./ofscc_v3"; elif [ -f "ofscc_v2" ]; then echo "./ofscc_v2"; else echo ""; fi)

# ─────────────────────────────────────────────────────────────────────────
# Default target
# ─────────────────────────────────────────────────────────────────────────

all: bootstrap validate

# ─────────────────────────────────────────────────────────────────────────
# Bootstrap - Build everything
# ─────────────────────────────────────────────────────────────────────────

bootstrap:
	@echo "🔨 Building OFS Compiler..."
	@bash scripts/bootstrap-linux.sh --jobs $(JOBS)

# ─────────────────────────────────────────────────────────────────────────
# C++ build - Compile C++ compiler only
# ─────────────────────────────────────────────────────────────────────────

cpp-build:
	@echo "📦 Building C++ compiler..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && \
		cmake .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) && \
		cmake --build . -j $(JOBS)
	@echo "✅ C++ build complete"

cpp-clean:
	@rm -rf $(BUILD_DIR)
	@echo "✅ C++ build artifacts cleaned"

# ─────────────────────────────────────────────────────────────────────────
# Validation
# ─────────────────────────────────────────────────────────────────────────

validate:
	@bash scripts/validate.sh -v

# ─────────────────────────────────────────────────────────────────────────
# Testing
# ─────────────────────────────────────────────────────────────────────────

test:
	@bash scripts/utils.sh test

test-lexer:
	@bash scripts/utils.sh test-lexer

test-parser:
	@bash scripts/utils.sh test-parser

test-semantic:
	@bash scripts/utils.sh test-semantic

# ─────────────────────────────────────────────────────────────────────────
# Compilation utilities
# ─────────────────────────────────────────────────────────────────────────

compile:
	@[ -z "$(FILE)" ] && echo "Usage: make compile FILE=program.ofs" && exit 1
	@bash scripts/utils.sh compile $(FILE) -O2

check:
	@[ -z "$(FILE)" ] && echo "Usage: make check FILE=program.ofs" && exit 1
	@bash scripts/utils.sh check $(FILE)

tokens:
	@[ -z "$(FILE)" ] && echo "Usage: make tokens FILE=program.ofs" && exit 1
	@bash scripts/utils.sh tokens $(FILE)

ast:
	@[ -z "$(FILE)" ] && echo "Usage: make ast FILE=program.ofs" && exit 1
	@bash scripts/utils.sh ast $(FILE)

ir:
	@[ -z "$(FILE)" ] && echo "Usage: make ir FILE=program.ofs" && exit 1
	@bash scripts/utils.sh ir $(FILE)

asm:
	@[ -z "$(FILE)" ] && echo "Usage: make asm FILE=program.ofs" && exit 1
	@bash scripts/utils.sh asm $(FILE)

# ─────────────────────────────────────────────────────────────────────────
# Examples
# ─────────────────────────────────────────────────────────────────────────

examples: check-compiler
	@echo "🧪 Compiling examples..."
	@for file in ofs/examples/{hello,calculator,fizzbuzz,variables,factorial}.ofs; do \
		if [ -f "$$file" ]; then \
			echo "  - Compiling $$file"; \
			$(OFSCC) build "$$file" -o "/tmp/example_$$$$" -O2 > /dev/null || exit 1; \
		fi; \
	done
	@echo "✅ All examples compiled successfully"

# ─────────────────────────────────────────────────────────────────────────
# Release
# ─────────────────────────────────────────────────────────────────────────

release:
	@bash scripts/create-release.sh $(VERSION)-native
	@echo ""
	@echo "📦 Release created:"
	@ls -lh releases/ | grep -E ".tar.gz|.zip|.dmg" | awk '{print "   - " $$9 " (" $$5 ")"}'

# ─────────────────────────────────────────────────────────────────────────
# Benchmarks
# ─────────────────────────────────────────────────────────────────────────

benchmark:
	@bash scripts/utils.sh benchmark

# ─────────────────────────────────────────────────────────────────────────
# Utilities
# ─────────────────────────────────────────────────────────────────────────

version:
	@bash scripts/utils.sh version

clean:
	@bash scripts/utils.sh clean

rebuild:
	@bash scripts/utils.sh rebuild

# ─────────────────────────────────────────────────────────────────────────
# Installation
# ─────────────────────────────────────────────────────────────────────────

install: bootstrap
	@echo "📦 Installing OFS compiler..."
	@install -D -m755 $(INSTALL_DIR)/ofscc $(DESTDIR)/usr/local/bin/ofscc
	@[ -d "$(INSTALL_DIR)/stdlib" ] && cp -r $(INSTALL_DIR)/stdlib $(DESTDIR)/usr/local/share/ofs/
	@echo "✅ Installation complete: $(DESTDIR)/usr/local/bin/ofscc"

uninstall:
	@rm -f $(DESTDIR)/usr/local/bin/ofscc
	@rm -rf $(DESTDIR)/usr/local/share/ofs/
	@echo "✅ Uninstalled"

# ─────────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────────

check-compiler:
	@if [ -z "$(OFSCC)" ]; then \
		echo "❌ OFS compiler not found. Run 'make bootstrap' first."; \
		exit 1; \
	fi
	@echo "✅ Using compiler: $(OFSCC)"

# ─────────────────────────────────────────────────────────────────────────
# Help
# ─────────────────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "OFS Compiler Makefile"
	@echo ""
	@echo "Building & Compilation:"
	@echo "  make bootstrap              - Full build & bootstrap (default)"
	@echo "  make cpp-build              - Build C++ compiler only"
	@echo "  make rebuild                - Clean and rebuild everything"
	@echo ""
	@echo "Validation & Testing:"
	@echo "  make validate               - Validate compiler build"
	@echo "  make test                   - Run all tests"
	@echo "  make test-lexer             - Run lexer tests"
	@echo "  make test-parser            - Run parser tests"
	@echo "  make test-semantic          - Run semantic tests"
	@echo ""
	@echo "Compilation Tasks:"
	@echo "  make compile FILE=file.ofs  - Compile OFS file"
	@echo "  make check FILE=file.ofs    - Type-check OFS file"
	@echo "  make tokens FILE=file.ofs   - Show lexer tokens"
	@echo "  make ast FILE=file.ofs      - Show AST"
	@echo "  make ir FILE=file.ofs       - Show LLVM IR"
	@echo "  make asm FILE=file.ofs      - Show assembly"
	@echo ""
	@echo "Examples & Release:"
	@echo "  make examples               - Compile example programs"
	@echo "  make release                - Create release packages"
	@echo "  make benchmark              - Run performance benchmarks"
	@echo ""
	@echo "Installation:"
	@echo "  make install                - Install compiler to /usr/local"
	@echo "  make uninstall              - Remove installed compiler"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean                  - Clean build artifacts"
	@echo "  make version                - Show compiler version"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=dir               - Build directory (default: build-ofscc)"
	@echo "  INSTALL_DIR=dir             - Install directory (default: dist)"
	@echo "  JOBS=n                      - Parallel jobs (default: nproc)"
	@echo "  VERSION=x.y.z               - Release version (default: 1.0.0)"
	@echo "  BUILD_TYPE=Release|Debug    - Build type (default: Release)"
	@echo ""
	@echo "Examples:"
	@echo "  make bootstrap              # Default: full build"
	@echo "  make test-lexer             # Just run lexer tests"
	@echo "  make compile FILE=hello.ofs # Compile hello.ofs"
	@echo "  make release VERSION=1.1.0  # Create 1.1.0 release"
	@echo "  make install DESTDIR=/opt   # Install to /opt"
	@echo ""
