# Convenience wrappers around the PlatformIO commands used throughout this
# project. Requires `pio` (PlatformIO Core) on your PATH.
#
# Run `make` or `make help` to see what's available.

.DEFAULT_GOAL := help

.PHONY: help build upload upload-ota flash test monitor clean

help: ## Show this help
	@echo "Available targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-12s\033[0m %s\n", $$1, $$2}'

build: ## Compile the firmware (env: esp32dev)
	pio run

upload: ## Build and flash over USB - needed for the very first flash
	pio run -t upload

upload-ota: ## Build and flash over WiFi - device must already be running OTA-enabled firmware
	pio run -e esp32dev_ota -t upload

flash: upload ## Alias for `upload`

test: ## Run the pure-logic unit tests on this machine (no hardware needed, see test/README)
	pio test -e native

monitor: ## Open the serial monitor (115200 baud, matches platformio.ini)
	pio device monitor

clean: ## Remove build artifacts
	pio run -t clean
