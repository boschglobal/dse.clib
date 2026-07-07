// Copyright 2018 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//
// This file is derived from:
// github.com/rogpeppe/go-internal/cmd/testscript/main.go
//
// Modifications:
//   - added custom testscript DSL commands
//   - reduced runner functionality to the options used in this repository

package main

import (
	"errors"
	"flag"
	"fmt"
	"os"
	"strings"
	"sync/atomic"

	"github.com/rogpeppe/go-internal/testscript"
)

func main() {
	switch err := mainerr(); err {
	case nil:
	default:
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

type envVarsFlag struct {
	vals []string
}

func (e *envVarsFlag) String() string {
	return fmt.Sprintf("%v", e.vals)
}

func (e *envVarsFlag) Set(v string) error {
	e.vals = append(e.vals, v)
	return nil
}

func mainerr() error {
	fWork := flag.Bool("work", false, "print temporary work directory and do not remove when done")
	fContinue := flag.Bool("continue", false, "continue running the script if an error occurs")
	fVerbose := flag.Bool("v", false, "run tests verbosely")
	var envVars envVarsFlag
	flag.Var(&envVars, "e", "pass through environment variable to script in form NAME[=value]; if =value is omitted, reads from current environment (can appear multiple times)")
	flag.Parse()

	files := flag.Args()
	if len(files) == 0 {
		return fmt.Errorf("usage: %s [-work] [-continue] [-v] [-e NAME[=value]] FILE.txtar [...]", os.Args[0])
	}

	p := testscript.Params{
		Setup:           func(*testscript.Env) error { return nil },
		Files:           files,
		ContinueOnError: *fContinue,
		TestWork:        *fWork,
		Cmds:            customCommands,
	}

	origSetup := p.Setup
	p.Setup = func(env *testscript.Env) error {
		if err := origSetup(env); err != nil {
			return err
		}
		if *fWork {
			env.T().Log("temporary work directory: ", env.WorkDir)
		}
		for _, v := range envVars.vals {
			varName, _, ok := strings.Cut(v, "=")
			switch varName {
			case "":
				return fmt.Errorf("environment variable name cannot be empty")
			case "WORK":
				return fmt.Errorf("cannot override WORK variable")
			}
			if !ok {
				// NAME form: copy from current process environment;
				// if unset, the variable is passed as NAME= (empty value).
				v += "=" + os.Getenv(varName)
			}
			env.Vars = append(env.Vars, v)
		}
		return nil
	}

	r := &runT{
		verbose: *fVerbose,
	}
	r.Run("", func(t testscript.T) {
		testscript.RunT(t, p)
	})
	if r.failed.Load() {
		return failedRun
	}
	return nil
}

var (
	failedRun = errors.New("failed run")
	skipRun   = errors.New("skip")
)

type runT struct {
	verbose bool
	failed  atomic.Bool
}

func (r *runT) Skip(is ...any) {
	panic(skipRun)
}

func (r *runT) Fatal(is ...any) {
	r.Log(is...)
	r.FailNow()
}

func (r *runT) Parallel() {
	// no-op
}

func (r *runT) Log(is ...any) {
	msg := fmt.Sprint(is...)
	if !strings.HasSuffix(msg, "\n") {
		msg += "\n"
	}
	fmt.Print(msg)
}

func (r *runT) FailNow() {
	panic(failedRun)
}

func (r *runT) Run(name string, f func(t testscript.T)) {
	defer func() {
		switch err := recover(); err {
		case nil, skipRun:
		case failedRun:
			r.failed.Store(true)
		default:
			panic(fmt.Errorf("unexpected panic: %v [%T]", err, err))
		}
	}()
	f(r)
}

func (r *runT) Verbose() bool {
	return r.verbose
}
