# File Executer

File Executer is a C project that scans text files, tokenizes their contents,
and builds a concurrent word index. A bounded job queue connects the directory
scanner to a pool of worker threads, and the index records each word's files
and occurrence counts.

## Requirements

- GCC or another C11-compatible compiler
- POSIX threads and semaphores (`pthread` and `semaphore`)

## Build

Build all test executables from the repository root:

```sh
make
```

The current `src/main.c` is a placeholder, so the project is built through its
test executables for now.

## Run tests

Run the complete test suite from the repository root:

```sh
make test
```

For a clean rebuild:

```sh
make rebuild
```

The integration and failure tests use paths relative to the repository root.

## Repository layout

- `include/` - public headers for the scanner, tokenizer, queue, index, and workers.
- `src/` - implementation code for file scanning, tokenization, indexing, queueing, and workers.
- `tests/` - unit, worker, integration, and failure-oriented test sources.
- `tests/*.txt` - small and medium text fixtures used by the tests.
- `tests/stress/` - 50-file stress-test fixture set.
- `tests/empty/` - empty-directory fixture.
- `build/` - generated test executables; it can be recreated with the build commands above.
- `.vscode/` - editor and debugging configuration.