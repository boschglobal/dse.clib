// Copyright 2025 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

package ast

type YAMLVisitor interface {
	Visit(map[string]interface{}, *Index)
}

type InnerVisitor struct {
	YAMLVisitor
}

func (v InnerVisitor) Visit(node map[string]interface{}, idx *Index) {
	innerNodes, ok := node["inner"].([]interface{})
	if !ok {
		return
	}

	for _, innerNode := range innerNodes {
		if innerNodeMap, ok := innerNode.(map[string]interface{}); ok {
			v.YAMLVisitor.Visit(innerNodeMap, idx)
		}
	}
}

type FunctionDeclVisitor struct{}

func (v FunctionDeclVisitor) Visit(node map[string]interface{}, idx *Index) {
	kind, kindOk := node["kind"].(string)
	if !kindOk || kind != "FunctionDecl" {
		return
	}

	name, nameOk := node["name"].(string)
	if nameOk {
		idx.Functions = append(idx.Functions, name)
	}
}

type TypedefDeclVisitor struct {
	TypeList         []string
	AnonymousStructs map[string]string // Maps RecordDecl ID to typedef name
}

func (v *TypedefDeclVisitor) Visit(node map[string]interface{}, idx *Index) {
	kind, kindOk := node["kind"].(string)
	if !kindOk || kind != "TypedefDecl" {
		return
	}

	name, nameOk := node["name"].(string)
	if !nameOk {
		return
	}
	v.TypeList = append(v.TypeList, name)

	// Handle inline anonymous struct: typedef struct { ... } MyStruct;
	inner, innerOk := node["inner"].([]interface{})
	if !innerOk {
		return
	}
	for _, innerItem := range inner {
		innerItemMap, ok := innerItem.(map[string]interface{})
		if !ok {
			continue
		}
		innerKind, innerKindOk := innerItemMap["kind"].(string)
		if !innerKindOk || innerKind != "ElaboratedType" {
			continue
		}
		ownedTagDecl, hasOwned := innerItemMap["ownedTagDecl"].(map[string]interface{})
		if !hasOwned {
			continue
		}
		declKind, _ := ownedTagDecl["kind"].(string)
		declName, _ := ownedTagDecl["name"].(string)
		if declKind != "RecordDecl" || declName != "" {
			continue
		}
		declID, hasID := ownedTagDecl["id"].(string)
		if !hasID {
			continue
		}
		if v.AnonymousStructs == nil {
			v.AnonymousStructs = make(map[string]string)
		}
		v.AnonymousStructs[declID] = name
	}
}

type RecordDeclVisitor struct {
	TypeList         []string
	StructList       []string
	AnonymousStructs map[string]string // Maps RecordDecl ID to typedef name
}

func (v RecordDeclVisitor) Visit(node map[string]interface{}, idx *Index) {
	kind, kindOk := node["kind"].(string)
	if !kindOk || kind != "RecordDecl" {
		return
	}

	name, nameOk := node["name"].(string)
	inner, innerOk := node["inner"].([]interface{})
	if !innerOk {
		return
	}

	// Named struct or typedef
	if nameOk && name != "" {
		// Record -> Typedef
		if itemExists(v.TypeList, name) {
			for _, innerItem := range inner {
				innerItemMap, innerItemOk := innerItem.(map[string]interface{})
				if !innerItemOk {
					continue
				}
				v.processTypedefFieldDecl(name, innerItemMap, idx)
			}
			return
		}
		// Record -> Struct
		v.StructList = append(v.StructList, name)
		for _, innerItem := range inner {
			innerItemMap, innerItemOk := innerItem.(map[string]interface{})
			if !innerItemOk {
				continue
			}
			v.processStructFieldDecl(name, innerItemMap, idx)
		}
		return
	}

	// Anonymous struct typedef: typedef struct { ... } MyStruct;
	nodeID, hasID := node["id"].(string)
	if !hasID {
		return
	}
	typedefName, isAnonymous := v.AnonymousStructs[nodeID]
	if !isAnonymous {
		return
	}
	for _, innerItem := range inner {
		innerItemMap, innerItemOk := innerItem.(map[string]interface{})
		if !innerItemOk {
			continue
		}
		v.processTypedefFieldDecl(typedefName, innerItemMap, idx)
	}
}

func (v RecordDeclVisitor) processTypedefFieldDecl(typedefName string, innerItemMap map[string]interface{}, idx *Index) {
	innerKind, innerKindOk := innerItemMap["kind"].(string)
	if !innerKindOk {
		return
	}

	switch innerKind {
	case "FieldDecl":
		fieldName, fieldNameOk := innerItemMap["name"].(string)
		fieldTypeMap, fieldTypeMapOk := innerItemMap["type"].(map[string]interface{})
		if !fieldNameOk || !fieldTypeMapOk {
			return
		}

		fieldType, fieldTypeOk := fieldTypeMap["qualType"].(string)
		if fieldTypeOk {
			// Check if the field type is an anonymous struct
			if isAnonymousStruct(fieldTypeMap) {
				// Recursively parse the anonymous struct
				member := IndexMemberDecl{Name: fieldName, TypeName: "struct"}
				member.AnonymousStructMembers = v.parseAnonymousStruct(innerItemMap)
				idx.Typedefs[typedefName] = append(idx.Typedefs[typedefName], member)
			} else {
				member := IndexMemberDecl{Name: fieldName, TypeName: fieldType}
				idx.Typedefs[typedefName] = append(idx.Typedefs[typedefName], member)
			}
		}

	case "RecordDecl":
		// Handle anonymous nested structs in typedef
		_, nameOk := innerItemMap["name"].(string)
		if !nameOk {
			// No name means it's an anonymous struct
			anonymousMembers := v.parseAnonymousStruct(innerItemMap)
			member := IndexMemberDecl{Name: "", TypeName: "struct", AnonymousStructMembers: anonymousMembers}
			idx.Typedefs[typedefName] = append(idx.Typedefs[typedefName], member)
		}
	}
}

func (v RecordDeclVisitor) processStructFieldDecl(structName string, innerItemMap map[string]interface{}, idx *Index) {
	innerKind, innerKindOk := innerItemMap["kind"].(string)
	if !innerKindOk {
		return
	}

	switch innerKind {
	case "FieldDecl":
		fieldName, fieldNameOk := innerItemMap["name"].(string)
		fieldTypeMap, fieldTypeMapOk := innerItemMap["type"].(map[string]interface{})
		if !fieldTypeMapOk {
			return
		}

		// Check for anonymous structs
		if isAnonymousStruct(fieldTypeMap) {
			member := IndexMemberDecl{Name: fieldName, TypeName: "struct"}
			member.AnonymousStructMembers = v.parseAnonymousStruct(innerItemMap)
			idx.Structs[structName] = append(idx.Structs[structName], member)
		} else if fieldType, fieldTypeOk := fieldTypeMap["qualType"].(string); fieldTypeOk && fieldNameOk {
			member := IndexMemberDecl{Name: fieldName, TypeName: fieldType}
			idx.Structs[structName] = append(idx.Structs[structName], member)
		}

	case "RecordDecl":
		// Handle anonymous nested structs
		_, nameOk := innerItemMap["name"].(string)
		if !nameOk {
			anonymousMembers := v.parseAnonymousStruct(innerItemMap)
			member := IndexMemberDecl{Name: "", TypeName: "struct", AnonymousStructMembers: anonymousMembers}
			idx.Structs[structName] = append(idx.Structs[structName], member)
		}
	}
}

// Check if the type is an anonymous struct
func isAnonymousStruct(fieldTypeMap map[string]interface{}) bool {
	_, exists := fieldTypeMap["inner"]
	return exists
}

// Parse the fields of an anonymous struct recursively
func (v *RecordDeclVisitor) parseAnonymousStruct(recordNode map[string]interface{}) []IndexMemberDecl {
	var members []IndexMemberDecl
	innerNodes, innerNodesOk := recordNode["inner"].([]interface{})
	if !innerNodesOk {
		return members
	}

	for _, innerNode := range innerNodes {
		if innerNodeMap, ok := innerNode.(map[string]interface{}); ok {
			innerKind, innerKindOk := innerNodeMap["kind"].(string)
			if !innerKindOk {
				continue
			}

			switch innerKind {
			case "FieldDecl":
				fieldName, fieldNameOk := innerNodeMap["name"].(string)
				fieldTypeMap, fieldTypeMapOk := innerNodeMap["type"].(map[string]interface{})
				if fieldNameOk && fieldTypeMapOk {
					fieldType, fieldTypeOk := fieldTypeMap["qualType"].(string)
					if fieldTypeOk {
						members = append(members, IndexMemberDecl{Name: fieldName, TypeName: fieldType})
					}
				}

			case "RecordDecl":
				// Handle nested anonymous structs
				_, nameOk := innerNodeMap["name"].(string)
				if !nameOk {
					anonymousMembers := v.parseAnonymousStruct(innerNodeMap)
					member := IndexMemberDecl{Name: "", TypeName: "struct", AnonymousStructMembers: anonymousMembers}
					members = append(members, member)
				}
			}
		}
	}
	return members
}

func itemExists(list []string, targetItem string) bool {
	for _, item := range list {
		if item == targetItem {
			return true
		}
	}
	return false
}
