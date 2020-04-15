# Fake Stream Socket Wrapper

## Objective
This library is the first example of an IPC model, allowing binaries that would normally communicate via the network.
Currently, it's set up to model the application-layer behavior of TCP sockets.  This has two main limitations:

- I'm not sure how applicable the behavior is to other protocols that sockets speak.  It may be in the future that this library will need to get forked for other Domain/type/protocol triples.
- It removes any underlying protcol implementation, meaning that analyzing the welded binary won't give any insight into protocls below the application layer.

The illusion of network comms is achieved using concurrency control.  In keeping with weldr's design principle of componets as threads in a single binary, two threads acting as client/server will interact appropriately for the bind/listen/connect/accept lifecycle.  In reality, all that's happening is that each thread waits on some shared lock for the other to perform its bit of the setup process.

## Source Structure

- **fake\_socket.c:** This file contains the bulk of the model implementation, as well as the library initializers.
- **wrap\_network.c:** This file contains the overridden library functions, defining the interface between the user code and the model.

## Data Types

- **SOCKET\_MODE:** This enum defines the different modes our fake sockets can be in.  This is used to sanity-check that a socket is at the correct life-cycle stage for a given operation, and to determine how the underlying fake\_filebuf struct should get used.
- **fake\_socket:** This struct defines all of the metadata and concurrency state needed to fake the application-layer behavior of a streaming socket.  This is designed to be plugged in as the implementation struct for fake\_fd objects.
- **sock\_mgr:** This struct defines all the concurrency and data allocation needed for this library, so I only need to allocate one global variable instead of several.

## Global Fields

- **mgr\_for\_socks:** This is the global instance of sock\_mgr.
- **sock\_ctl:** This is the global instance of fd\_control defining the interface between the file descriptor and socket models.
