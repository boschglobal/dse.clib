// Copyright 2025 Robert Bosch GmbH

#include <stdio.h>
#include <dse/clib/ini/ini.h>

#define INI_FILE "dse_clib.ini"

int main(void)
{
    // Open and read an INI file (if it exists).
    IniDesc ini = ini_open(INI_FILE);

    // Get an INI value.
    printf("foo (initial value) = %s\n", ini_get_val(&ini, "foo"));

    // Set an INI value.
    ini_set_val(&ini, "foo", "fubar", true); /* Overwrite existing value. */
    ini_set_val(&ini, "bar", "bar", false);  /* Only if not already set. */
    printf("foo (updated value) = %s\n", ini_get_val(&ini, "foo"));
    printf("bar (default value) = %s\n", ini_get_val(&ini, "bar"));

    // Expand an environment var.
    ini_set_val(&ini, "user", "${USER}", true);
    ini_expand_vars(&ini);
    printf("user (env value) = %s\n", ini_get_val(&ini, "user"));

    // Delete a value.
    ini_delete_key(&ini, "user");
    printf("user (deleted) = %s\n", ini_get_val(&ini, "user"));

    // Save the INI file.
    ini_write(&ini, INI_FILE);

    // Close the desc object (and release used memory).
    ini_close(&ini);
    return 0;
}
