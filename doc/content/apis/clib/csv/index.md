---
title: CSV API Reference
linkTitle: CSV
---
## CSV API


CSV API provides stream-oriented reading of delimiter-separated value files.
The first row defines the column names (headers), and each call to
`csv_next()` reads one data line into the current `fields` vector.


### Example


The following example demonstrates how to use the CSV API.

{{< readfile file="../examples/csv_file.c" code="true" lang="c" >}}




## Typedefs

### CsvDesc

```c
typedef struct CsvDesc {
  const char* file_name;
  FILE*       file;
  Vector      header;
  Vector      fields;
  size_t      line_maxlen;
  char*       line;
} CsvDesc;
```


## Functions

### csv_open

Open and parse a CSV file into a `CsvDesc` descriptor object.

The first row is consumed as the header row (column names); all subsequent
rows are stored as data rows. Fields may be separated by commas or
semicolons.

#### Parameters

path (const char*)
: Path to a CSV file. If NULL, the `CSV_FILE` environment variable is used.
  If the resolved path cannot be opened an empty `CsvDesc` is returned
  without error.

#### Returns

CsvDesc (struct)
: CSV descriptor object. Call `csv_close()` when finished.



### csv_count

Return the number of fields in the current parsed row.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.

#### Returns

size_t
: Number of fields in `CsvDesc.fields`.



### csv_header

Return the header (column name) for a column index.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.

col (size_t)
: Zero-based column index.

#### Returns

const char*
: Column name, or NULL if the index is out of range.



### csv_next

Read and parse the next data line.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.

#### Returns

int
: 0 on success, -ENODATA at end of file, -EINVAL on a parse error (bad
  token or wrong field count), -EOVERFLOW on truncated lines (line exceeds
  configured buffer), -ENOMEM on allocation failure.



### csv_field

Return a field value from the current parsed row.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.

col (size_t)
: Zero-based column index.

#### Returns

double
: Parsed value, or 0.0 if the index is out of range.



### csv_fields

Read multiple field values from the current row.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.

col (size_t[])
: Column index list.

val (double[])
: Output value array.

len (size_t)
: Number of requested columns.

#### Returns

void



### csv_close

Release all resources owned by a CSV descriptor.

#### Parameters

csv (CsvDesc*)
: CSV descriptor object.
