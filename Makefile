CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS := -Iinclude
LDLIBS := -pthread

COMMON_SOURCES := \
	src/file_scanner.c \
	src/index.c \
	src/job_queue.c \
	src/tokenizer.c \
	src/worker.c \
	tests/test_helpers.c
HEADERS := $(wildcard include/*.h)

TEST_BINS := \
	build/queue_tests \
	build/worker_tests \
	build/integration_test \
	build/failure_tests

build/queue_tests: TEST_SOURCE := tests/tests.c
build/worker_tests: TEST_SOURCE := tests/tests_worker.c
build/integration_test: TEST_SOURCE := tests/integration_test.c
build/failure_tests: TEST_SOURCE := tests/failure_tests.c

.PHONY: all test clean rebuild

all: $(TEST_BINS)

build:
	mkdir -p $@

$(TEST_BINS): $(COMMON_SOURCES) $(HEADERS) | build
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COMMON_SOURCES) $(TEST_SOURCE) -o $@ $(LDLIBS)

test: all
	@for test in $(TEST_BINS); do ./$$test; done

clean:
	rm -rf build

rebuild: clean
	$(MAKE) all