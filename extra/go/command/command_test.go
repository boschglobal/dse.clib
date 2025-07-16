// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package command

import (
	"bytes"
	"flag"
	"fmt"
	"testing"

	"github.com/stretchr/testify/assert"
)

type FooCommand struct {
	Command
}

func NewFooCommand(name string) *FooCommand {
	c := &FooCommand{
		Command: Command{
			Name:    name,
			FlagSet: flag.NewFlagSet(name, flag.ExitOnError),
		},
	}
	return c
}

func (c FooCommand) Name() string {
	return c.Command.Name
}

func (c FooCommand) FlagSet() *flag.FlagSet {
	return c.Command.FlagSet
}

func (c *FooCommand) Parse(args []string) error {
	return c.FlagSet().Parse(args)
}

func (c *FooCommand) Run() error {
	fmt.Fprintf(flag.CommandLine.Output(), "bar")
	return nil
}

func TestFooCommand(t *testing.T) {
	cmd := NewFooCommand("foo")
	args := []string{}
	output := new(bytes.Buffer)
	flag.CommandLine.SetOutput(output)

	count := 0
	cmd.FlagSet().VisitAll(func(*flag.Flag) { count += 1 })

	assert.Equal(t, "foo", cmd.Name())
	assert.Equal(t, 0, count)
	assert.False(t, cmd.FlagSet().Parsed())
	assert.Nil(t, cmd.Parse(args))
	assert.True(t, cmd.FlagSet().Parsed())
	assert.NotContains(t, output.String(), "bar")
	assert.Nil(t, cmd.Run())
	assert.Contains(t, output.String(), "bar")
}
