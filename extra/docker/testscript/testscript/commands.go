// Copyright 2026 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/rogpeppe/go-internal/testscript"
)

func makeCustomCommands(verbose bool) map[string]func(*testscript.TestScript, bool, []string) {
	return map[string]func(*testscript.TestScript, bool, []string){
		"hello":        helloCmd,
		"sleep":        sleepCmd,
		"touch":        touchCmd,
		"filecontains": filecontainsCmd,
		"defer":        makeDeferCmd(verbose),
	}
}

func helloCmd(ts *testscript.TestScript, neg bool, args []string) {
	if neg {
		ts.Fatalf("hello does not support !")
	}
	if len(args) != 1 {
		ts.Fatalf("usage: hello <name>")
	}
	fmt.Fprintf(ts.Stdout(), "Hello, %s!\n", args[0])
}

func filecontainsCmd(ts *testscript.TestScript, neg bool, args []string) {
	if len(args) != 2 {
		ts.Fatalf("filecontains <file> <file>|<text>")
	}
	got := ts.ReadFile(args[0])
	want := args[1]
	if data, err := os.ReadFile(ts.MkAbs(want)); err == nil {
		want = string(data)
	}
	if strings.Contains(got, want) == neg {
		ts.Fatalf("filecontains %q; %q not found in file:\n%q", args[0], want, got)
	}
}

func sleepCmd(ts *testscript.TestScript, neg bool, args []string) {
	if neg {
		ts.Fatalf("sleep does not support !")
	}

	duration := time.Second
	if len(args) == 1 {
		d, err := time.ParseDuration(args[0])
		ts.Check(err)
		duration = d
	} else if len(args) > 1 {
		ts.Fatalf("usage: sleep [duration]")
	}

	time.Sleep(duration)
}

func touchCmd(ts *testscript.TestScript, neg bool, args []string) {
	if neg {
		ts.Fatalf("touch does not support !")
	}
	if len(args) != 1 {
		ts.Fatalf("touch <file>")
	}

	path := ts.MkAbs(args[0])

	err := os.MkdirAll(filepath.Dir(path), 0o750)
	ts.Check(err)

	file, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE, 0o644)
	ts.Check(err)
	err = file.Close()
	ts.Check(err)

	t := time.Now()
	err = os.Chtimes(path, t, t)
	ts.Check(err)
}

func makeDeferCmd(verbose bool) func(*testscript.TestScript, bool, []string) {
	return func(ts *testscript.TestScript, neg bool, args []string) {
		if len(args) < 1 {
			ts.Fatalf("usage: defer program [args...]")
		}
		command := args[0]
		commandArgs := append([]string(nil), args[1:]...)

		ts.Defer(func() {
			var err error
			if !verbose {
				shellCmd := fmt.Sprintf("%s %s > /dev/null 2>&1", command, strings.Join(commandArgs, " "))
				err = ts.Exec("sh", "-c", shellCmd)
			} else {
				err = ts.Exec(command, commandArgs...)
			}
			if neg {
				if err == nil {
					ts.Fatalf("deferred command unexpectedly succeeded: %s %s", command, strings.Join(commandArgs, " "))
				}
				return
			}
			if err != nil {
				ts.Fatalf("deferred command unexpectedly failed: %v", err)
			}
		})
	}
}
