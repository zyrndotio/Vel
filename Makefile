BUILD_DIR ?= build
BUILD_TYPE ?= Release

.PHONY: configure build test clean install package

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) --config $(BUILD_TYPE) --parallel 2

test: build
	ctest --test-dir $(BUILD_DIR) --build-config $(BUILD_TYPE) --output-on-failure

install: build
	cmake --install $(BUILD_DIR) --config $(BUILD_TYPE)

package: build
	cmake --build $(BUILD_DIR) --target package --config $(BUILD_TYPE)

clean:
	rm -rf $(BUILD_DIR)
