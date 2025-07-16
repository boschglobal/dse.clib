// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package ast

type Index struct {
	Functions []string
	Typedefs  map[string][]IndexMemberDecl
	Structs   map[string][]IndexMemberDecl
}

type IndexMemberDecl struct {
	Name                   string
	TypeName               string
	AnonymousStructMembers []IndexMemberDecl
}
