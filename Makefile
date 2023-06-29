PACKAGE_NAME := $(shell awk '/"name":/ {gsub(/[",]/, "", $$2); print $$2}' package.json)
DEST_DIR ?= /usr/local
DIST_TEST=dist/manifest.json
NODE_MODULES_TEST=node_modules/

all: $(DIST_TEST)

$(DIST_TEST): $(NODE_MODULES_TEST) $(shell find src/ -type f) package.json webpack.config.js
	NODE_ENV=$(NODE_ENV) node_modules/.bin/webpack

watch:
	NODE_ENV=$(NODE_ENV) npm run watch

clean:
	rm -rf dist/
	rm -f $(SPEC)

install: $(DIST_TEST)
	mkdir -p $(DEST_DIR)/share/cockpit/$(PACKAGE_NAME)
	cp -r dist/* $(DEST_DIR)/share/cockpit/$(PACKAGE_NAME)

devel-install: $(DIST_TEST)
	mkdir -p ~/.local/share/cockpit
	ln -s `pwd`/dist ~/.local/share/cockpit/$(PACKAGE_NAME)

devel-uninstall:
	rm -f ~/.local/share/cockpit/$(PACKAGE_NAME)

$(NODE_MODULES_TEST): package.json
	env -u NODE_ENV npm install --ignore-scripts
	env -u NODE_ENV npm prune

.PHONY: all clean install devel-install devel-uninstall
