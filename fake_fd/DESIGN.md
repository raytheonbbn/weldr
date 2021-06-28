# Fake File Descriptor Wrapper

## Objective

This library creates the concept of a fake file-like object; anything in linux that uses
file descriptors (which is a lot.)  Since most of these objects will share a couple of
core libc functions, I've extracted their functionality and underlying structs here.

Currently, this library doesn't model any data channels itself.  It's here as a template and support
library for other models that operate on file-like objects, such as sockets.

## Source Structure

- **fake\_fd.c**: This file contains the bulk of the model implementation, as well as library initializer routines.
- **wrap\_??.c**: This file contains the overridden libc functions, translating between the fused binaries and the model.

## Data Types

All of these are defined in include/fake\_fd.h

- **fake\_fd:** The fake file descriptor object.  This registers that a given file descriptor is to be handled by weldr, rather than the normal library interfaces.  As stated above, this structure doesn't really model anything itself. Instead, it wraps an implementation struct that stores all the implementation-specific data for this particular kind of file, and stores a function table that defines how common operations should interact with that struct.
- **fake\_filebuf:** If a fake\_fd struct rides above a channel-specific implementation, fake\_filebufs ride below them.  Most file-like objects behave in pretty much the same way (they all use and thus obey the interfaces for read and write.)  The fake\_filebuf struct captures that behavior and allows for concurrency-controlled two-way comms.  Exactly how this structure is used is up to the specific implementation that wraps it.
- **fd\_control:** This struct is a function pointer table that stores the interface between common file descriptor operations and the implementation they wrap.  Each implementation library should define its own fd\_control struct, and pass it to any fake\_fd it creates.  This struct currently serves four major roles:
	- Provides a common constructor signature for implementation structs.
	- Provides specific data access to members of the underlying fake\_filebuf struct, for use in read() and write().
	- Provides a getter for the fake\_filebuf struct itself.
	- Provides a handler for poll() and ppoll(), which use their own status checks.
	- (Future) Will eventually need to handle select() as well, if and when it gets fully implemented.
- **fd\_mgr:** A global manager struct, allowing me to define one global variable instead of the several needed to manage concurrency and memory for fake\_fd objects and operations.

## Global Fields

- **mgr\_for\_fds:** This is the global instance of fd\_mgr.  NOTE: It's named oddly because the static linker will happily merge all symbols named "mgr" from the input objects, and the resulting binary will happily run with it until something realizes one of the two structs' mebers gets corrupted.  If you create your own implementation library, name its manager something similarly unique.

----------
Copyright (c) Raytheon BBN Technologies 2021, All Rights Reserved

This document does not contain technology or Technical Data controlled under either
the U.S. International Traffic in Arms Regulations or the U.S. Export Administration

Distribution A: Approved for Public Release, Distribution Unlimited
