# Standard Stream Stubs

## Objective

This library has one very specific goal; give each system component its own copies of the standard I/O streams.  Because weldr runs all system components in one binary, they would normally conflict for use of stdin/stdout/stderr; trying to read from or write to any one componet would be a mess.  

This library fixes that by redefining these streams as individual files in a stub object.  At weld-time, each system component gets its own copy of the stub file; symbol deconfliction will automatically detect the collision of the different stdouts, and rename them across the entire component so the analyst doesn't need to manually pick through the system code to find each use of a stdio stream.

Because a given terminal only has one set of streams, the stub initializer replaces them with ordinary file handles (for stdout and stderr) and named fifos (for stdin.)  To read from output streams, you just need to cat the correct file.  To write to an input stream, you just need to redirect a shell command _into_ the correct file.

**NOTE:** Currently, due to me being lazy about handling arguments passed to a welded binary, all stream handle paths are defined at compile-time, and are currently fixed by the main file generator to use your home directory.

**WARNING:** This is going to cause a problem if we need to override mkfifo; a wrapper library won't be able to distibguish between this call to mkfifo, where we actually want to make a fifo, and an IPC call.

Additionally, this library statically redefines a couple of stdio functions, such as printf, which reference a fixed stdio stream as part of their behavior.  All these do is force a static reference to that stream unique to each component, so that symbol deconfliction can clean things up.  For example; printf(...) is reimplemented to call fprintf(stdout,...).  When symbol deconfliction is done, each component has its own component0\_printf(...) that calls fprintf(component0\_stdout, ...).

## Source Structure

- **redefine\_streams.c:** This contains the entire model, including both stub initalization and the static overrides of stdio functions.

## Data Types

None needed.  This just pokes a bunch of existing interfaces.

## Global Fields
- **stdin/stdout/stderr:** These three FILE pointers override the symbols from stdio.h, allowing the symbol deconflictor to do its work.
- **stdin\_path/stdout\_path/stderr\_path:** These store file paths that 
