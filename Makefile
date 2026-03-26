# ─── tollvm Makefile ──────────────────────────────────────────────────────────
BUILD_DIR  := build
BINARY     := $(BUILD_DIR)/tollvm
NPROC      := $(shell nproc)

.PHONY: all build configure clean rebuild run help

# Default target
all: build

## configure  — roda cmake (só precisa rodar uma vez)
configure:
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

## build  — compila o projeto
build:
	@if [ ! -f "$(BUILD_DIR)/Makefile" ] && [ ! -f "$(BUILD_DIR)/build.ninja" ]; then \
		$(MAKE) configure; \
	fi
	@cmake --build $(BUILD_DIR) --parallel $(NPROC)

## rebuild  — limpa e recompila tudo do zero
rebuild: clean configure build

## clean  — remove o diretório de build
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Build directory removed."

## run FILE=<file.tm>  — compila e executa com um arquivo .tm
run: build
ifndef FILE
	@echo "Usage: make run FILE=examples/main.tm"
else
	@$(BINARY) $(FILE)
endif

## help  — lista os targets disponíveis
help:
	@echo ""
	@echo "  tollvm — Makefile targets"
	@echo ""
	@grep -E '^##' Makefile | sed 's/## /  make /g'
	@echo ""
