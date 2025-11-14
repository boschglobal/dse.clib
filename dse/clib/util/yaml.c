// Copyright 2023 Robert Bosch GmbH
//
// SPDX-License-Identifier: Apache-2.0

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <yaml.h>
#include <dse/testing.h>
#include <dse/clib/util/yaml.h>
#include <dse/logger.h>


#define HASHLIST_DEFAULT_SIZE 64
#define HASHMAP_DEFAULT_SIZE  16
#define EXPAND_VAR_MAXLEN     1023


/* Internal API. */
static YamlDocList* _parse_file(const char* filename, YamlDocList* doc_list);
static void         _destroy_node(YamlNode* node);
static void         _destroy_doc_list(YamlDocList* doc_list);


static char* __strdup__(const char* s)
{
    size_t len = strlen(s) + 1;
    void*  dup = malloc(len);
    return (char*)memcpy(dup, s, len);
}

/**
 *  dse_yaml_load_file
 *
 *  Load all YAML documents from a file and return the list of documents.
 *
 *  Parameters
 *  ----------
 *  filename : const char*
 *      The filename to parse for YAML documents (i.e. delimited by '---').
 *  doc_list : YamlDocList*
 *      List of documents, if set (not NULL) then parsed documents will be
 *      appended to that list.
 *
 *  Returns
 *  -------
 *      YamlDocList* : List of parsed documents, including previously parsed
 *          documents if doc_list was provided as an argument.
 */
DLL_PUBLIC YamlDocList* dse_yaml_load_file(
    const char* filename, YamlDocList* doc_list)
{
    return _parse_file(filename, doc_list);
}


/**
 *  dse_yaml_destroy_node
 *
 *  Release the resources consumed by a node instance.
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node to be destroyed.
 *
 *  Returns
 *  -------
 */

DLL_PUBLIC void dse_yaml_destroy_node(YamlNode* node)
{
    return _destroy_node(node);
}


/**
 *  dse_yaml_destroy_doc_list
 *
 *  Release the resources consumed by a doc_list instance.
 *
 *  Parameters
 *  ----------
 *  doc_list : YamlDocList*
 *      List of documents to be destroyed.
 *
 *  Returns
 *  -------
 */
DLL_PUBLIC void dse_yaml_destroy_doc_list(YamlDocList* doc_list)
{
    return _destroy_doc_list(doc_list);
}


/**
 *  dse_yaml_load_single_doc
 *
 *  Load and return the first YAML Document found in the file `filename`.
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC YamlNode* dse_yaml_load_single_doc(const char* filename)
{
    YamlDocList* doc_list = _parse_file(filename, NULL);
    YamlNode*    node = hashlist_at(doc_list, 0);
    for (uint32_t i = 1; i < hashlist_length(doc_list); i++) {
        YamlNode* doc = hashlist_at(doc_list, i);
        _destroy_node(doc);
    }
    hashlist_destroy(doc_list);
    free(doc_list);
    doc_list = NULL;
    return node;
}


/**
 *  dse_yaml_get_scalar
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC const char* dse_yaml_get_scalar(YamlNode* node, const char* name)
{
    YamlNode* _ = dse_yaml_find_node(node, name);
    if (_ && _->scalar) {
        if (node->__inter__) node->__inter__(_);
        return _->scalar;
    }
    return NULL;
}


/**
 *  dse_yaml_get_array
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC const char** dse_yaml_get_array(
    YamlNode* node, const char* name, size_t* len)
{
    YamlNode* array_node = dse_yaml_find_node(node, name);

    int          null_term = len ? 0 : 1;
    size_t       array_length = hashlist_length(&array_node->sequence);
    const char** array = calloc(array_length + null_term, sizeof(const char*));
    for (size_t i = 0; i < array_length; i++) {
        YamlNode* _item = hashlist_at(&array_node->sequence, i);
        array[i] = _item->name;
    }

    if (len) *len = array_length;
    return array;
}


/**
 *  dse_yaml_get_bool
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node in doc to be loaded.
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC int dse_yaml_get_bool(YamlNode* node, const char* name, bool* value)
{
    if (node == NULL && name == NULL && value == NULL) return EINVAL;
    const char* _scalar = dse_yaml_get_scalar(node, name);
    if (_scalar == NULL) return EINVAL;
    /* True? */
    const char* bool_true[] = { "y", "Y", "yes", "Yes", "YES", "true", "True",
        "TRUE", "on", "On", "ON", NULL };
    for (const char** p = bool_true; *p; p++) {
        if (strcmp(*p, _scalar)) continue;
        *value = true;
        return 0;
    }
    /* False? */
    const char* bool_false[] = { "n", "N", "no", "No", "NO", "false", "False",
        "FALSE", "off", "Off", "OFF", NULL };
    for (const char** p = bool_false; *p; p++) {
        if (strcmp(*p, _scalar)) continue;
        *value = false;
        return 0;
    }
    return EINVAL;
}


/**
 *  dse_yaml_get_uint
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node in doc to be loaded.
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC int dse_yaml_get_uint(
    YamlNode* node, const char* name, unsigned int* value)
{
    if (node == NULL || name == NULL || value == NULL) return EINVAL;
    const char* _scalar = dse_yaml_get_scalar(node, name);
    if (_scalar == NULL) return EINVAL;
    /* Integer? */
    errno = 0;
    char* endptr = NULL;
    int   _int = strtol(_scalar, &endptr, 0);
    if (errno == 0 && _scalar != endptr && *endptr == '\0' && _int >= 0) {
        *value = (unsigned int)_int;
        return 0;
    }
    /* Fallback to bool? */
    bool _bool;
    if (dse_yaml_get_bool(node, name, &_bool) == 0) {
        *value = _bool;
        return 0;
    }
    return EINVAL;
}


/**
 *  dse_yaml_get_int
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node in doc to be loaded.
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC int dse_yaml_get_int(YamlNode* node, const char* name, int* value)
{
    const char* _scalar = dse_yaml_get_scalar(node, name);
    if (node == NULL || name == NULL || value == NULL || _scalar == NULL)
        return EINVAL;
    /* Integer? */
    errno = 0;
    char* endptr = NULL;
    int   _int = strtol(_scalar, &endptr, 0);
    if (errno == 0 && _scalar != endptr && *endptr == '\0') {
        *value = _int;
        return 0;
    }
    /* Fallback to bool? */
    bool _bool;
    if (dse_yaml_get_bool(node, name, &_bool) == 0) {
        *value = _bool;
        return 0;
    }
    return EINVAL;
}


/**
 *  dse_yaml_get_string
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node in doc to be loaded.
 *
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC int dse_yaml_get_string(
    YamlNode* node, const char* name, const char** value)
{
    const char* _scalar = dse_yaml_get_scalar(node, name);
    if (node == NULL || name == NULL || value == NULL || _scalar == NULL)
        return EINVAL;
    /* Integer? */
    int _int;
    if (dse_yaml_get_int(node, name, &_int) == 0) {
        *value = NULL;
        return EINVAL;
    }
    /* bool? */
    bool _bool;
    if (dse_yaml_get_bool(node, name, &_bool) == 0) {
        *value = NULL;
        return EINVAL;
    }
    /* String */
    *value = _scalar;
    return 0;
}


/**
 *  dse_yaml_get_double
 *
 *  Parameters
 *  ----------
 *  node : YamlNode*
 *      node in doc to be loaded.
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC int dse_yaml_get_double(
    YamlNode* node, const char* name, double* value)
{
    const char* _scalar = dse_yaml_get_scalar(node, name);
    if (_scalar == NULL) return EINVAL;
    for (unsigned int i = 0; i < strlen(_scalar); i++) {
        double val = atof(_scalar);
        if (((_scalar[i] >= '0' && _scalar[i] <= '9') || _scalar[i] == '.')) {
            *value = val;
            return 0;
        }
    }
    return EINVAL;
}


/**
 *  dse_yaml_find_node
 *
 *  Parameters
 *  ----------
 *
 *  Returns
 *  -------
 *
 */
DLL_PUBLIC YamlNode* dse_yaml_find_node(YamlNode* root, const char* path)
{
    char* _path = __strdup__(path);

    /* Start the search at the root, look for the first token. */
    YamlNode* node = root;
    char*     saveptr;
    char*     token = strtok_r(_path, "/", &saveptr);
    while (token && node) {
        if (node->node_type == YAML_SEQUENCE_NODE) {
            /* path does not support sequences (arrays). If this is the
            last token in the path, then return the node. */
            token = strtok_r(NULL, "/", &saveptr);
            if (token) node = NULL;
        } else if (node->node_type == YAML_MAPPING_NODE) {
            /* Navigate down the Node Tree. */
            node = hashmap_get(&node->mapping, token);
            token = strtok_r(NULL, "/", &saveptr);
        } else if (node->node_type == YAML_SCALAR_NODE) {
            /* If there is another token, then the search went past the
            end of the Node Tree Branch. End the search. */
            token = strtok_r(NULL, "/", &saveptr);
            if (token) node = NULL;
        } else {
            /* End the search. */
            node = NULL;
        }
    }
    free(_path);
    return node;
}


/**
 *  dse_yaml_find_node_in_seq
 *
 *  Select a YAML sequence node based on a list of selectors. If all selectors
 *  match for a sequence node, that node is returned.
 *
 *  Parameters
 *  ----------
 *  root : YamlNode*
 *      Starting node for this search.
 *  path : const char*
 *      Path to the sequence.
 *  selector : const char**
 *      Array of selector paths, relative to the path on the root node.
 *  value : const char**
 *      Array of selector values.
 *  len : uint32t
 *      Length of the selector/value array.
 *
 *  Returns
 *  -------
 *      YamlNode* : The found YAML Node, otherwise NULL.
 */
DLL_PUBLIC YamlNode* dse_yaml_find_node_in_seq(YamlNode* root, const char* path,
    const char** selector, const char** value, uint32_t len)
{
    YamlNode* node;
    YamlNode* seq;
    YamlNode* seq_node;

    /* Search for sequence. */
    seq = dse_yaml_find_node(root, path);
    if (!seq) return NULL;
    /* Search and return sequence node. */
    for (uint32_t j = 0; j < hashlist_length(&seq->sequence); j++) {
        seq_node = hashlist_at(&seq->sequence, j);
        /* Check that each selector matches. */
        uint32_t match_count = 0;
        for (uint32_t sel = 0; sel < len; sel++) {
            node = dse_yaml_find_node(seq_node, selector[sel]);
            if (node && node->scalar) {
                if (strcmp(node->scalar, value[sel]) == 0) {
                    match_count++;
                    continue; /* Match, try the next. */
                }
            }
            break; /* No match. */
        }
        if (len == match_count) return seq_node;
    }
    return NULL;
}


/**
 *  dse_yaml_find_node_in_doclist
 *
 *  Find a YAML node in a list of documents.
 *
 *  Parameters
 *  ----------
 *  doc_list : YamlDocList*
 *      List of documents.
 *  kind : const char*
 *      Search only in this kind of document.
 *  path : const char*
 *      Search for a node with this path.
 *
 *  Returns
 *  -------
 *      YamlNode* : The found YAML Node, otherwise NULL.
 */
DLL_PUBLIC YamlNode* dse_yaml_find_node_in_doclist(
    YamlDocList* doc_list, const char* kind, const char* path)
{
    YamlNode* doc;
    YamlNode* node;

    if (doc_list == NULL) return NULL;

    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        doc = hashlist_at(doc_list, i);
        /* Correct kind? */
        node = dse_yaml_find_node(doc, "kind");
        if (node && node->scalar) {
            if (strcmp(node->scalar, kind) != 0) continue;
        }
        /* Search and return node. */
        node = dse_yaml_find_node(doc, path);
        if (node) return node;
    }
    return NULL;
}


/**
 *  dse_yaml_find_doc_in_doclist
 *
 *  Find a YAML node in a list of documents.
 *
 *  Parameters
 *  ----------
 *  doc_list : YamlDocList*
 *      List of documents.
 *  kind : const char*
 *      Search only in this kind of document.
 *  selector : const char**
 *      Array of selector paths, relative to the path on the root node.
 *  value : const char**
 *      Array of selector values.
 *  len : uint32t
 *      Length of the selector/value array.
 *
 *  Returns
 *  -------
 *      YamlNode* : The found YAML Node, otherwise NULL.
 */
DLL_PUBLIC YamlNode* dse_yaml_find_doc_in_doclist(YamlDocList* doc_list,
    const char* kind, const char** selector, const char** value, uint32_t len)
{
    YamlNode* doc;
    YamlNode* node;

    if (doc_list == NULL) return NULL;
    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        doc = hashlist_at(doc_list, i);
        /* Correct kind? */
        node = dse_yaml_find_node(doc, "kind");
        if (node && node->scalar) {
            if (strcmp(node->scalar, kind) != 0) continue;
        }
        /* Check that each selector matches. */
        uint32_t match_count = 0;
        for (uint32_t sel = 0; sel < len; sel++) {
            node = dse_yaml_find_node(doc, selector[sel]);
            if (node && node->scalar) {
                if (strcmp(node->scalar, value[sel]) == 0) {
                    match_count++;
                    continue; /* Match, try the next. */
                }
            }
            break; /* No match. */
        }
        if (len == match_count) return doc;
    }
    return NULL;
}


/**
 *  dse_yaml_find_node_in_seq_in_doclist
 *
 *  Find a YAML node, in a sequence, in a list of documents.
 *
 *  Parameters
 *  ----------
 *  doc_list : YamlDocList*
 *      List of documents.
 *  kind : const char*
 *      Search only in this kind of document.
 *  path : const char*
 *      Path to the sequence.
 *  selector : const char*
 *      Selector path.
 *  value : const char*
 *      Selector value.
 *
 *  Returns
 *  -------
 *      YamlNode* : The found YAML Node, otherwise NULL.
 */
YamlNode* dse_yaml_find_node_in_seq_in_doclist(YamlDocList* doc_list,
    const char* kind, const char* path, const char* selector, const char* value)
{
    YamlNode* doc;
    YamlNode* node;
    YamlNode* seq;
    YamlNode* seq_node;

    if (doc_list == NULL) return NULL;

    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        doc = hashlist_at(doc_list, i);
        /* Correct kind? */
        node = dse_yaml_find_node(doc, "kind");
        if (node && node->scalar) {
            if (strcmp(node->scalar, kind) != 0) continue;
        }
        /* Search for sequence. */
        seq = dse_yaml_find_node(doc, path);
        if (!seq) continue;
        /* Search and return sequence node. */
        for (uint32_t j = 0; j < hashlist_length(&seq->sequence); j++) {
            seq_node = hashlist_at(&seq->sequence, j);
            node = dse_yaml_find_node(seq_node, selector);
            if (node && node->scalar) {
                if (strcmp(node->scalar, value) == 0) return seq_node;
            }
        }
    }
    return NULL;
}


static YamlNode* _create_node(char* name, YamlNode* parent)
{
    YamlNode* node = calloc(1, sizeof(YamlNode));
    node->node_type = YAML_NO_NODE;
    if (name) node->name = __strdup__(name);
    node->parent = parent;
    /* Attach this Node into the parents storage class. */
    if (parent) {
        if (parent->node_type == YAML_MAPPING_NODE) {
            assert(node->name);
            if (node->name) {
                /* Prevent duplicate dict keys, last wins. */
                void* o = hashmap_remove(&parent->mapping, node->name);
                if (o) _destroy_node(o);
                hashmap_set(&parent->mapping, node->name, node);
            }
        } else if (parent->node_type == YAML_SEQUENCE_NODE) {
            hashlist_append(&parent->sequence, node);
        }
    }
    /* Return the new node. */
    return node;
}


static void _set_node_scalar(YamlNode* node, char* value)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SCALAR_NODE;
    node->scalar = __strdup__(value);
}


static void _set_node_mapping(YamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_MAPPING_NODE;
    hashmap_init_alt(&node->mapping, HASHMAP_DEFAULT_SIZE, NULL);
}


static void _set_node_sequence(YamlNode* node)
{
    assert(node->node_type == YAML_NO_NODE);
    node->node_type = YAML_SEQUENCE_NODE;
    hashlist_init(&node->sequence, HASHLIST_DEFAULT_SIZE);
}


static YamlDocList* _create_doc_list(void)
{
    YamlDocList* doc_list = calloc(1, sizeof(HashList));
    if (doc_list == NULL) {
        log_error("Error creating document list");
        return NULL;
    }
    if (hashlist_init(doc_list, HASHLIST_DEFAULT_SIZE) != HASHMAP_SUCCESS) {
        if (errno == 0) errno = ECANCELED;
        log_error("Error creating document list");
        free(doc_list);
        return NULL;
    }
    return doc_list;
}


static YamlDocList* _parse_file(const char* filename, YamlDocList* doc_list)
{
    errno = 0;

    /* Either append to the provided doc_list or create a new one. */
    if (doc_list == NULL) {
        doc_list = _create_doc_list();
        if (doc_list == NULL) return NULL;
    }
    /* Open the YAML file. */
    FILE* file_handle = fopen(filename, "r");
    if (file_handle == NULL) {
        if (errno == 0) errno = EINVAL;
        log_error("Error opening file: %s", filename);
        return doc_list;
    }
    /* Setup the YAML parser*/
    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        if (errno == 0) errno = ECANCELED;
        log_error("Error initializing parser");
        return doc_list;
    }
    yaml_parser_set_input_file(&parser, file_handle);


    /* Parse the YAML documents contained in the file using an event parser. */
    YamlNode*    doc = NULL;
    YamlNode*    node = NULL;
    yaml_event_t event;
    do {
        /* Parse the next event. */
        if (!yaml_parser_parse(&parser, &event)) {
            if (errno == 0) errno = ECANCELED;
            log_error("Error while parsing YAML event");
            goto error_parse;
        }
        /* Process the event. */
        switch (event.type) {
        /* Document events. */
        case YAML_DOCUMENT_START_EVENT:
            assert(doc == NULL);
            doc = node = _create_node(NULL, node);
            log_trace("%p/%p: YAML_DOCUMENT_START_EVENT", doc, node);
            break;
        case YAML_DOCUMENT_END_EVENT:
            log_trace("%p/%p: YAML_DOCUMENT_END_EVENT", doc, node);
            hashlist_append(doc_list, doc);
            doc = node = NULL; /* Reset the document pointers. */
            break;
        /* Node events. */
        case YAML_SCALAR_EVENT:
            /* Gets called individually for name and value. */
            if (node->node_type == YAML_MAPPING_NODE ||
                node->node_type == YAML_SEQUENCE_NODE) {
                /* Create a child node with value as its key/name. */
                node = _create_node((char*)event.data.scalar.value, node);
                log_trace("  %p/%p: YAML_SCALAR_EVENT name=%s", node->parent,
                    node, (char*)event.data.scalar.value);
                /* If the parent (i.e. node at entry) is a YAML_SEQUENCE_NODE
                 * then this is a simple array (values only). */
                if (node->parent->node_type == YAML_SEQUENCE_NODE) {
                    _set_node_scalar(node, (char*)event.data.scalar.value);
                    /* This node is complete. */
                    node = node->parent;
                }
            } else {
                /* The child node is scalar, set the node_type and value. */
                _set_node_scalar(node, (char*)event.data.scalar.value);
                log_trace("  %p/%p: YAML_SCALAR_EVENT name=%s value=%s",
                    node->parent, node, node->name,
                    (char*)event.data.scalar.value);
                /* This node is complete. */
                node = node->parent;
            }
            break;
        case YAML_MAPPING_START_EVENT:
            assert(doc);
            assert(node);
            if (node->node_type == YAML_SEQUENCE_NODE) {
                /* This mapping is an item of the parent sequence, create
                a node and append to the sequence. */
                node = _create_node(NULL, node);
            }
            _set_node_mapping(node);
            log_trace("%p/%p: YAML_MAPPING_START_EVENT name=%s type=%d", doc,
                node, node->name, node->node_type);
            break;
        case YAML_SEQUENCE_START_EVENT:
            assert(doc);
            assert(node);
            _set_node_sequence(node);
            log_trace("%p/%p: YAML_SEQUENCE_START_EVENT name=%s type=%d", doc,
                node, node->name, node->node_type);
            break;
        case YAML_MAPPING_END_EVENT:
            log_trace("%p/%p: YAML_MAPPING_END_EVENT name=%s type=%d", doc,
                node, node->name, node->node_type);
            node = node->parent;
            break;
        case YAML_SEQUENCE_END_EVENT:
            log_trace("%p/%p: YAML_SEQUENCE_END_EVENT name=%s type=%d", doc,
                node, node->name, node->node_type);
            node = node->parent;
            break;
        /* Other events, ignored. */
        case YAML_STREAM_START_EVENT:
        case YAML_STREAM_END_EVENT:
        case YAML_NO_NODE:
        default:
            break;
        }
        if (event.type == YAML_STREAM_END_EVENT) break;
        yaml_event_delete(&event);
    } while (true);

error_parse:
    /* Release the stack objects used in the parsing. */
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(file_handle);

    /* Return the parsed YAML documents. */
    return doc_list;
}


static void _destroy_node(YamlNode* node)
{
    if (node == NULL) return;
    if (node->node_type == YAML_MAPPING_NODE) {
        for (uint32_t i = 0; i < node->mapping.number_nodes; ++i) {
            if (node->mapping.nodes[i]) {
                _destroy_node(node->mapping.nodes[i]->value);
            }
        }
        hashmap_destroy(&node->mapping);
    }
    if (node->node_type == YAML_SEQUENCE_NODE) {
        for (uint32_t i = 0; i < hashlist_length(&node->sequence); i++) {
            _destroy_node(hashlist_at(&node->sequence, i));
        }
        hashlist_destroy(&node->sequence);
    }
    free(node->name);
    free(node->scalar);
    free(node);
}


static void _destroy_doc_list(YamlDocList* doc_list)
{
    if (doc_list == NULL) return;
    for (uint32_t i = 0; i < hashlist_length(doc_list); i++) {
        YamlNode* doc = hashlist_at(doc_list, i);
        _destroy_node(doc);
    }
    hashlist_destroy(doc_list);
    free(doc_list);
}

/**
 *  dse_yaml_expand_vars
 *
 *  Expand environment variables in a string according to typical shell
 *  variable expansion (i.e ${FOO} or ${BAR:-default}).
 *
 *  Parameters
 *  ----------
 *  source : const char*
 *      The string containing environment variables to expand.
 *
 *  Returns
 *  -------
 *      char* : String with environment variable expanded. Caller to free.
 */
DLL_PUBLIC char* dse_yaml_expand_vars(const char* source)
{
    char* source_copy = strdup(source);
    char* haystack = source_copy;
    char* result = calloc(EXPAND_VAR_MAXLEN + 1, sizeof(char));

    while (haystack) {
        /* Search for START. */
        char* _var = strstr(haystack, "${");
        if (_var == NULL) {
            strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
            break;
        }
        /* Copy any preceding chars to the result. */
        *_var = '\0';
        strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
        /* Search for END. */
        _var += 2;
        char* _var_end = strstr(_var, "}");
        if (_var_end == NULL) {
            /* Did not find the end, GIGO. */
            strncat(result, haystack, EXPAND_VAR_MAXLEN - strlen(result));
            break;
        }
        *_var_end = '\0';
        haystack = _var_end + 1; /* Setup for next iteration. */

        /* Does the VAR have a DEFAULT? */
        char* _def = strstr(_var, ":-");
        if (_def) {
            *_def = '\0';
            _def += 2;
        }
        /* Do the lookup. */
        char* _env_val = getenv(_var);
        if (_env_val) {
            strncat(result, _env_val, EXPAND_VAR_MAXLEN - strlen(result));
        } else if (_def) {
            strncat(result, _def, EXPAND_VAR_MAXLEN - strlen(result));
        } else {
            /* No var, no default, GIGO. */
            strncat(result, _var, EXPAND_VAR_MAXLEN - strlen(result));
        }
    }

    free(source_copy);
    return result; /* Caller to free. */
}
