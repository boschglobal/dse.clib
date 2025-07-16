// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package test_workingdir

import (
	"os"
	"path"

	"runtime"
)

func init() {
	_, filename, _, _ := runtime.Caller(0)
	dir := path.Join(path.Dir(filename), "..", "..", "..")
	err := os.Chdir(dir)
	if err != nil {
		panic(err)
	}
}
