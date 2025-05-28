# Interceptor Example

The Interceptor Library should allow the data flow of a Functional Block to be
intercepted and modified _without_ the target ECU being aware of the
modification; in particular the schedule(), task() and (function block) call()
should operate without modification.

A Function Block is considered to have the following semantic:

> DATA_IN -> Call() -> DATA_OUT

and the Interceptor Library attempts to install hooks to modify the data flow as follows:

>  DATA_IN -> hook_in() -> Call() -> hook_out() -> DATA_OUT

Hook functions modify the related data blocks such that the Call() operates on
an altered data state. For this method to work the Call() function must only
reference stateful data from the known/specified data blocks.


Methods evaluated:
  * __Wrap__ - call() functions are replaced with functions which operate the in/out
    hooks as well as calling the original call() function. The runtime/consumer
    provides the wrap functions. <br>See examples in the `wrap` folder.
  * __Hook__ - call() functions are boxed with in/out weak linked function calls.
    The runtime/consumer provides the hook functions. <br>See examples in the
    `hook` folder.
  * __ELF Hooking__ - (not proven) see https://lief.re/doc/latest/tutorials/04_elf_hooking.html


## Build

```bash
$ cd dse.clib

# Build.
$ make clean
$ make

# Run (edit main.c to select a particular scenario).
$ make
$ cd dse/clib/examples/build/_out/examples/interceptor/bin; ./main; cd -
task_init: ECU AB
task_5ms: ECU AB
fb_call: HOOK_IN FOO
fb_call: REAL FOO
fb_call: HOOK_OUT FOO
fb_call: HOOK_IN BAR
fb_call: REAL BAR
fb_call: HOOK_OUT BAR
task_exit: ECU AB
```


## Debug

```bash
# Inspect symbol tables.
$ readelf -s dse/clib/examples/build/_out/examples/interceptor/lib/ecu_a.so

# Dump symbol table.
$ nm dse/clib/examples/build/_out/examples/interceptor/lib/wrapped_ecu_a.so
```
