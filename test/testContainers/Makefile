#
# AppScope - Integration Test Runner
#
# See ./README.md for details.
#

COMPOSE ?= $(shell which docker-compose 2>/dev/null)
ifeq (, $(COMPOSE))
    $(error Missing docker-compose)
endif

DOCKER ?= $(shell which docker 2>/dev/null)
ifeq (, $(DOCKER))
    $(error Missing docker)
endif

ARCH ?= $(shell uname -m)

COMPOSE := $(COMPOSE) -f docker-compose.yml -f docker-compose.$(ARCH).yml
TESTS := $(shell TAG=$(ARCH) $(COMPOSE) config --services | sort -V)

help:
	@echo "AppScope Integration Test Runner"
	@echo "  \`make help\` - show this info"
	@echo "  \`make all\` - run all tests, stop of first failure"
	@echo "  \`make build\` - build all test images"
	@echo "  \`make push\` - push all test images"
	@echo "  \`make (test)\` - run a single test"
	@echo "  \`make (test)-build\` - build the test's image"
	@echo "  \`make (test)-shell\` - run a shell in the test's container"
	@echo "Tests: $(TESTS)"

tests:
	@TAG=$(ARCH) $(COMPOSE) config --services | sort -V

all:
	@STARTED=; RAN=0; SKIP=0; PASS=0; FAIL=0; TOTAL=$(words $(TESTS)); OK=; ERR=; \
	for TEST in $(TESTS); do \
		if [ -n "$(START)" -a -z "$$STARTED" ]; then \
			if [ "$(START)" = "$$TEST" ]; then \
				STARTED=1; \
			else \
				SKIP=$$((SKIP+1)); \
				continue; \
			fi; \
		fi; \
		if $(MAKE) -s $${TEST}; then  \
			RAN=$$((RAN+1)); \
			PASS=$$((PASS+1)); \
			OK="$${OK}$${TEST} "; \
		else \
			RAN=$$((RAN+1)); \
			FAIL=$$((FAIL+1)); \
			ERR="$${ERR}$${TEST} "; \
			if [ -z "$(CONTINUE)" ]; then \
				echo >&2 "error: aborting \`make all\` on test failure"; \
				echo >&2 "       ran $${RAN}/$${TOTAL} tests, skipped $${SKIP}"; \
				echo >&2 "       $${PASS} passed, $${OK}"; \
				echo >&2 "       $${FAIL} failed, $${ERR}"; \
				exit 1; \
			fi; \
		fi; \
	done; \
	if [ $$PASS -gt 0 ]; then \
		echo "info: ran $${RAN}/$${TOTAL} tests, skipped $${SKIP}"; \
		echo "      $${PASS} passed, $${OK}"; \
		echo "      $${FAIL} failed, $${ERR}"; \
	else \
		echo >&2 "error: ran $${RAN}/$${TOTAL} tests, skipped $${SKIP}"; \
		echo >&2 "      $${PASS} passed, $${OK}"; \
		echo >&2 "      $${FAIL} failed, $${ERR}"; \
		exit 1; \
	fi

$(TESTS):
	@[ -n "$(NOBUILD)" ] || $(MAKE) -s $(@)-build
	@echo
	@echo "==================================================================="
	@echo " $(shell date)"
	@echo " Running \"$@\" Test"
	@echo "==================================================================="
	@TAG=$(ARCH) $(COMPOSE) run --rm $(if $(CI),,--name appscope-test-$(@)) $@ || \
		{ echo >&2 "error: $(@) test failed."; exit 1; }

build: $(addsuffix -build, $(TESTS))

# --
# Check if an image already exists locally with the tag before pulling it again
# so we can build a newer version with newer configs and not have to rebuilt it
# every time. That's what ID is for. Should have no effect in CI as the local
# images list should be empty.
# --
%-build: IMAGE=ghcr.io/criblio/appscope-test-$(@:%-build=%):$(ARCH)
%-build: ID=$(shell $(DOCKER) images -q --filter=reference=$(IMAGE))
%-build:
	@[ -n "$(filter $(@:%-build=%),$(TESTS))" ] || \
		{ echo >&2 "error: $(@:%-build=%) isn't a test."; exit 1; }
	@echo
	@echo "==================================================================="
	@echo " $(shell date)"
	@echo " Building Image for \"$(@:%-build=%)\" Test"
	@echo "==================================================================="
	-@[ -n "$(ID)" -o -n "$(NOPULL)" ] || $(DOCKER) pull $(IMAGE)
	@TAG=$(ARCH) $(COMPOSE) build \
		--progress tty \
		$(@:%-build=%)

push: $(addsuffix -push, $(TESTS))

%-push: IMAGE=ghcr.io/criblio/appscope-test-$(@:%-push=%):$(ARCH)
%-push:
	@[ -n "$(filter $(@:%-push=%),$(TESTS))" ] || \
		{ echo >&2 "error: $(@:%-push=%) isn't a test."; exit 1; }
	@[ -n "$(NOBUILD)" ] || $(MAKE) -s $(@:%-push=%)-build
	@echo
	@echo "==================================================================="
	@echo " $(shell date)"
	@echo " Pushing Image for \"$(@:%-push=%)\" Test"
	@echo "==================================================================="
	@time $(DOCKER) push $(IMAGE)

%-shell: CMD?=/bin/bash
%-shell:
	@[ -n "$(filter $(@:%-shell=%),$(TESTS))" ] || \
		{ echo >&2 "error: $(@:%-shell=%) isn't a test."; exit 1; }
	@[ -n "$(NOBUILD)" ] || $(MAKE) -s $(@:%-shell=%)-build
	@echo
	@echo "==================================================================="
	@echo " $(shell date)"
	@echo " Shell in Container for \"$(@:%-shell=%)\" Test"
	@echo "==================================================================="
	@TAG=$(ARCH) $(COMPOSE) run --rm \
		--service-ports \
		--name appscope-test-$(@:%-shell=%) \
		$(@:%-shell=%) $(CMD)

%-exec: CMD?=/bin/bash
%-exec:
	@[ -n "$(filter $(@:%-exec=%),$(TESTS))" ] || \
		{ echo >&2 "error: $(@:%-exec=%) isn't a test."; exit 1; }
	@echo
	@echo "==================================================================="
	@echo " $(shell date)"
	@echo " Exec'ing into Container for \"$(@:%-exec=%)\" Test"
	@echo "==================================================================="
	@TAG=$(ARCH) $(DOCKER) exec -it appscope-test-$(@:%-exec=%) $(CMD)

.PHONY: help all build push $(TESTS) %-build %-push %-shell

# vim: sw=4 ts=4 noet :
