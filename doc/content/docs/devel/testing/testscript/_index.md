---
title: "Testing with Testscript"
linkTitle: "Testscript"
draft: false
tags:
- Developer
- Testing
- Testscript
github_repo: "https://github.com/boschglobal/dse.clib"
github_subdir: "doc"
---

## Testscript for E2E Testing

DSE projects use Testscript to run end-to-end (E2E) tests written in the
[txtar](https://pkg.go.dev/golang.org/x/tools/txtar) format. The containerised
runtime provides the Testscript DSL runner, Docker, Taskfile, Valgrind, and a
set of standard CLI tools.


## Quick Start

### Code Layout

```
<dse.repo>
├── tests/[testscript]          <-- Directory containing Testscript tests.
│   └── e2e                     <-- Collection of E2E tests.
│       └── testcase.txtar      <-- Individual test cases (txtar format).
└── Makefile                    <-- High-level build automation with `test_e2e` target.
```

### Run Tests

> Hint: In DSE repos the following commands are normally represented by Makefile target `test_e2e`. See subsequent sections of this document for details.
```bash
# Run all E2E tests (shell expands the wildcard)
TEST='tests/testscript/e2e/*.txtar'
docker run -it --rm \
    -e ENTRYDIR=$(pwd) \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v $(pwd):/repo \
    $(TESTSCRIPT_IMAGE) \
        -e ENTRYDIR=$(pwd) \
        ${TEST}

# Run a single test by setting TEST explicitly
TEST='tests/testscript/e2e/my_test.txtar'
docker run -it --rm \
    -e ENTRYDIR=$(pwd) \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v $(pwd):/repo \
    $(TESTSCRIPT_IMAGE) \
        -e ENTRYDIR=$(pwd) \
        ${TEST}
```

### Minimal Test File

```txtar
# Minimal E2E test
env NAME=my_test
exec sh -e $WORK/test.sh
stdout 'expected output'

-- test.sh --
docker run --name $NAME -i --rm \
    ghcr.io/boschglobal/dse-image:latest
```


## Debugging

### Verbose Output

Pass `-v` to see the full script execution log including all commands and
their output:

```bash
docker run ... $(TESTSCRIPT_IMAGE) -v tests/testscript/e2e/my_test.txtar
```

### Inspect the Work Directory

Pass `-work` to retain and print the temporary work directory after the test
runs. This allows manual inspection of files created during the test:

```bash
docker run ... $(TESTSCRIPT_IMAGE) -work tests/testscript/e2e/my_test.txtar
```

### Continue on Error

Pass `-continue` to let the script run past the first failure, useful to see
all failures in a single run:

```bash
docker run ... $(TESTSCRIPT_IMAGE) -continue tests/testscript/e2e/my_test.txtar
```

### Pass Environment Variables

Use `-e` to inject host environment variables into the script:

```bash
docker run ... $(TESTSCRIPT_IMAGE) \
    -e ENTRYDIR=$(pwd) \
    -e MY_VAR=value \
    tests/testscript/e2e/my_test.txtar
```

If only a name is given (`-e NAME`), the value is read from the current
process environment. If unset the variable is passed as empty.


## Command Reference

### Standard Commands

Built-in commands from
[rogpeppe/go-internal/testscript](https://pkg.go.dev/github.com/rogpeppe/go-internal/testscript).
Commands marked `[!]` support the negation prefix.

| Command | Description |
|---------|-------------|
| `cd dir` | Change working directory for subsequent commands. |
| `chmod perm path...` | Change permissions (octal mode, e.g. `0644`). |
| `[!] cmp file1 file2` | Check files have identical content. `file1` may be `stdout` or `stderr`. On mismatch, a diff is printed. |
| `[!] cmpenv file1 file2` | Like `cmp`, but environment variables in `file2` are substituted first. |
| `cp src... dst` | Copy files to a target file or directory. `src` may be `stdout` or `stderr`. |
| `env [key=value...]` | With no args, print the environment (useful for debugging). Otherwise set variables. |
| `[!] exec program [args...] [&]` | Run a program. Append `&` to run in the background; `&word&` to name the background job. |
| `[!] exists [-readonly] file...` | Check that files or directories exist (or do not exist). |
| `[!] grep [-count=N] pattern file` | Match a regexp against a file's content. `-count=N` requires exactly N matches. |
| `kill [-SIGNAL] [command]` | Terminate background commands. Signal may be `KILL` (default) or `INT`. Optional `command` targets a named job. |
| `mkdir path...` | Create directories (no-op if they already exist). |
| `mv path1 path2` | Rename a file or directory. |
| `rm file...` | Remove files or directories. |
| `skip [message]` | Mark the test as skipped. |
| `[!] stderr [-count=N] pattern` | Match a regexp against stderr of the most recent `exec`/`wait`. |
| `stdin file` | Set stdin for the next `exec` command. File may be `stdout` or `stderr`. |
| `stop [message]` | Stop the test early, marking it as passing. |
| `[!] stdout [-count=N] pattern` | Match a regexp against stdout of the most recent `exec`/`wait`. |
| `symlink file -> target` | Create a symbolic link. |
| `unquote file...` | Strip leading `>` characters from each line in the file. |
| `wait [command]` | Wait for all background commands (or a named job) to finish. |


### Custom Commands

These commands are registered by the DSE Testscript runtime in
`extra/docker/testscript/testscript/commands.go`.

#### `hello <name>`

Prints `Hello, <name>!` to stdout. Useful for smoke-testing the DSL runner.
Does not support `!`.

```txtar
hello world
stdout 'Hello, world!'
```

#### `sleep [duration]`

Sleeps for the given duration (default: `1s`). Accepts Go duration strings
such as `500ms`, `2s`, `1m30s`. Does not support `!`.

```txtar
sleep 500ms
```

#### `touch <file>`

Creates an empty file at `<file>`, including any missing parent directories.
Updates the modification time if the file already exists. Does not support `!`.

```txtar
touch path/to/file.txt
exists path/to/file.txt
```

#### `[!] filecontains <file> <text|file2>`

Checks whether `<file>` contains `<text>`. `<file>` is read relative to the
current working directory and also accepts `stdout` or `stderr` as special
values (content of the most recent `exec`/`wait` output). If `<text>` is
itself a resolvable path inside the work directory, the contents of that file
are used as the expected value. Supports `!` negation.

```txtar
filecontains output.log 'expected string'
! filecontains output.log 'ERROR'
filecontains result.txt expected.txt
```

#### `defer program [args...]`

Registers a command to run as a deferred cleanup action at the end of the test.
Multiple defers execute in reverse registration order (like Go's `defer`
statement). Supports `!` to assert that the deferred command is expected to
fail — if it unexpectedly succeeds, the test fails.

In verbose mode (`-v`) the deferred command's output is shown; otherwise it
is suppressed (`> /dev/null 2>&1`).

```txtar
# Start a container and guarantee removal when the test ends
exec docker run --name my_svc -d my-image:latest
defer docker rm -f my_svc

exec sh -e $WORK/test.sh
stdout 'expected result'
```

```txtar
# Assert that a deferred command is expected to fail
! defer docker inspect non_existent_container
```


## Environment Variables

Variables automatically available inside test scripts:

| Variable | Description |
|----------|-------------|
| `WORK` | Temporary work directory. Cleaned up after the test unless `-work` is passed. |
| `REPODIR` | Mapped to `/repo` inside the Testscript environment. |
| `ENTRYDIR` | Full host path of the repository root (pass via `-e ENTRYDIR=...`). |
| `ENTRYHOSTDIR` | Host-relative path of the repository root, mapped to `/repo`. |
| `ENTRYWORKDIR` | (Container runtime) Host path of the generated work directory, mapped to `/workdir`. |
| `WORKDIR` | (Container runtime) Path to `/workdir` inside the Testscript environment. |
| `BUILDDIR` | (Go test runner only) Mapped to `/builddir`; contains the path of the Makefile that started Testscript. |
| `PATH` | Actual host `PATH`. |
| `HOME` | Set to `/no-home` (`USERPROFILE` on Windows). |
| `TMPDIR` | Set to `$WORK/.tmp`. |
| `devnull` | Value of `os.DevNull`. |

> Hint: When mounting volumes into a container launched from a test script,
> use `$ENTRYDIR` (the full host path) rather than `/repo`. Docker resolves
> mount paths from the host's perspective, not from inside the running
> Testscript container.


## Testing Features and Integrations

The Testscript container provides a minimal set of pre-installed tools:

### Docker

Docker CLI and Docker Compose are included and configured to access the host
Docker socket via `/var/run/docker.sock`.

### Taskfile

[Task](https://taskfile.dev/) is included, along with standard CLI tools
(`curl`, `jq`, `yq`, `git`, `make`, `rsync`, `zip`/`unzip`) for use in task
workflows.

### Valgrind

Valgrind (memcheck) is included for memory error detection in native binaries.

### txtar

The `txtar` CLI is included for encoding and decoding txtar archives outside
of the Testscript runner.


## Testing Techniques

### E2E / Smoke Test

A minimal end-to-end test that launches a containerised workflow and checks
its output:

```txtar
env NAME=minimal_inst
env SIM=dse/clib/build/_out/examples/minimal
exec sh -e $WORK/test.sh
stdout 'expected result'

-- test.sh --
IMAGE="${IMAGE:-ghcr.io/boschglobal/dse-image:latest}"
docker run --name $NAME -i --rm \
    -v $ENTRYDIR/$SIM:/sim \
    $IMAGE
```

### Deferred Cleanup

Use `defer` to guarantee container removal even when a test step fails:

```txtar
exec docker run --name my_svc -d my-image:latest
defer docker rm -f my_svc

exec sh -e $WORK/test.sh
stdout 'expected result'

-- test.sh --
docker exec my_svc some-command
```

### Checking File Contents

```txtar
exec some-tool --output $WORK/result.txt
filecontains $WORK/result.txt 'expected output'
! filecontains $WORK/result.txt 'ERROR'
```


## Makefile Integration

An example Makefile target for running Testscript tests. The `TEST` variable
selects test scripts; with the wildcard default below it runs all matching
`.txtar` files.

```makefile
TESTSCRIPT_IMAGE ?= ghcr.io/boschglobal/dse-testscript:latest
TESTSCRIPT_OPTS  ?=
TEST             ?= $(wildcard tests/testscript/e2e/*.txtar)

.PHONY: testscript
testscript:
	docker run -it --rm \
		-e ENTRYDIR=$(CURDIR) \
		-v /var/run/docker.sock:/var/run/docker.sock \
		-v $(CURDIR):/repo \
		$(TESTSCRIPT_IMAGE) $(TESTSCRIPT_OPTS) \
		-e ENTRYDIR=$(CURDIR) \
		$(TEST)
```

```bash
# Run all tests
make testscript

# Run a single test
make testscript TEST=tests/testscript/e2e/my_test.txtar

# Run verbosely and keep the work directory
make testscript TESTSCRIPT_OPTS="-v -work" TEST=tests/testscript/e2e/my_test.txtar
```


## References / Links

- [Testscript](https://pkg.go.dev/github.com/rogpeppe/go-internal/testscript)
- [txtar](https://pkg.go.dev/golang.org/x/tools/txtar)
- [Taskfile](https://taskfile.dev/)
- [DSE CLib](https://github.com/boschglobal/dse.clib)
