---
title: INI FIle API Reference
linkTitle: INI
---
## INI File API


Simple INI File API for reading and modifying INI files.


### Example


The following example demonstrates how to use the INI File API.

{{< readfile file="../examples/ini_file.c" code="true" lang="c" >}}




## Typedefs

### IniDesc

```c
typedef struct IniDesc {
    int lines;
}
```

## Functions

### ini_close

Release any resources allocated by the INI File object.

#### Parameters

ini (IniDesc*)
: INI File object.



### ini_delete_key

Delete the specified key from the INI File object.

#### Parameters

ini (IniDesc*)
: INI File object.

key (const char*)
: The key to delete.



### ini_expand_vars

Expand environment variables contained within the INI File values.

#### Example INI File

```ini
user=${USER:-default-user}`
```

#### Parameters

ini (IniDesc*)
: INI File object.



### ini_get_val

Set a key-value pair on the INI File object.

#### Parameters

ini (IniDesc*)
: INI File object.

key (const char*)
: The key to get.

overwrite (bool)
: When true, if the key already exists then overwrite with the provided value.

#### Returns

char*
: The corresponding value of key, or NULL if key is was not found.



### ini_open

Configure and load an INI File object.

#### Parameters

path (const char*)
: Path to an INI File to load. If NULL or missing no error will occur.

#### Returns

IniDesc (struct)
: INI File object.



### ini_set_val

Set a key-value pair on the INI File object.

#### Parameters

ini (IniDesc*)
: INI File object.

key (const char*)
: The key to set.

val (const char*)
: The corresponding value to set.

overwrite (bool)
: When true, if the key already exists then overwrite with the provided value.



### ini_write

Write the key-value pair of the INI File object to the named file.

#### Parameters

ini (IniDesc*)
: INI File object.

path (const char*)
: The file to write the key-value pairs to.



