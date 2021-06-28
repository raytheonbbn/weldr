# Weldr: Fusing linux executables, for science!

## What does weldr do?
The tool fuses a system of multiple executables into a single monolithic executable
for analysis purposes.  

When analyzing a binary, dynamic library calls and IPC create
gaps in data and control flow that are difficult to bridge.  There are approaches
to isolate a single binary and "patch" the holes left by the disconnected components.
However, this approach makes analyzing a connected system difficult or impossible.

The tool takes a different approach.  It replaces dynamic library calls with
statically-linked user-space code, and fuses all programs in the system
into a single, monolithic executable.  The data paths of the entire system
are traced explictly in the binary itself, allowing single-binary tools
to examine the entire system.

## How does weldr do it?
The tool works all its magic in the linking phase of compilation.

1) Hook Make to recover object files for each sub-program
2) Add stub libraries for sub-program-specific modeling.
3) Duplicate sub-programs to match a system specification doc. 
4) Deconflic symbols between sub-program instances; each sub-program instance has its own namespace within the fused binary.
5) Generate a driver object that configures the stub libraries, and launches each sub-program in its own thread.
6) Apply wrapper libraries to redefine library calls shared across sub-program instances.
7) Compile all the objects together into a single executable. 

## Requirements for target projects
As said above, the tool hooks Make in order to recover object files and compiler configurations
for each sub-program.  This means the make file needs to satisfy some requirements.

- The final compilation step for binaries or libraries must use the default variables, such as $(CC).
    The tool replaces these commands with hook scripts.

The tool has been expanded to understand libtools-based builds.  Libtools is really just an extension of Make,
so the same rules apply.

## Installing Weldr

### Dependencies 

NOTE: Weldr is only tested on the Fedora and Ubuntu Linux distributions.
It should work on other GNU/Linux platforms, but no guarantees.

To install and run the code, you need the following:

- Bash 4.1 or later
- Python 3.6 or later
- pip
- virtualenv (strongly recommended)

To actually weld binaries, weldr needs a Linux C/C++ development environment.
Many distributions have a package or package group that includes everything weldr needs:

- **Fedora:** `sudo dnf -y group install "C Development Tools and Libraries"` 
- **CentOS:** `sudo yum groupinstall -y "Development Tools"`
- **Ubuntu:** `sudo apt install build-essentials`

Finally, weldr relies on a companion tool - Torch - to rewrite object files.
You can clone Torch from [https://github.com/raytheonbbn/torch.git](https://github.com/raytheonbbn/torch.git).

### Installation

Weldr itself is a python package.  Currently, it is only configured for development installation.
It is strongly recommened that you install weldr within a python 3 virtual environment:

[https://docs.python-guide.org/dev/virtualenvs/](https://docs.python-guide.org/dev/virtualenvs/)

The recommended steps are as follows.

- Install platform dependencies
- Create and activate a Python 3 virtual environment
- Install Torch (it is also a python package)
- From the top-level directory of the weldr repository, execute `python setup.py develop`.

Once complete, check that the model libraries build correctly on your system:

- Find each directory containing a `Makefile`.
- From within each directory, execute `make clean all`.  This should complete successfuly.

## Running the Tool

The weldr package exposes a command-line interface:

weldr [OPTIONS] -s SYSTEM_DEF -t WORKING_DIR PROJECT_DIR [PROJECT_DIR...]

**Required Options:**

- `-s SYSTEM_DEF`: Path to a system definition file.  
- `-t WORKING_DIR`: Path for weldr to use as a working directory.  The directory will be created if it doesn't exist, and overwritten if it does.
- `PROJECT_DIR`: Build directory for one project to be included in the welded binary (the one containing the top-level Makefile)

**Some Optional Options:**

- `-v`: Turn on debug logging.  This also includes the output of Make or Torch.
- `-L LOG_FILE`: Print debug log output to LOG_FILE.
- `-S`: Turn on Safe Mode.  This executes every stage of weldr in a separate working directory, and saves internal state so you can restart the tool from an intermediate stage.
- `--start-stage NAME`: Start weldr at stage NAME, rather than from the beginning.  This requires successfully running all predicessor stages with Safe Mode enabled, and the same working directory.
- `--stop-stage NAME`: Stop weldr after stage NAME, rather than allowing it to run to completion.

For a complete list of options, execute `weldr -h`.

### The System Definition File

As mentioned above, weldr needs you to give it a system specification telling it which programs to run,
the order in which they should be launched, and any default command line arguments for each.

The document looks a lot like a shell script; each line takes the following form:

`[PRIMARY] command [arg...]`

- `PRIMARY`: This keyword designates a subprocess as the primary; if it exits, the entire program should terminate, rather than waiting for all subprograms to exit.  You can only specify one primary subprogram per configuration.
 `command`: A program from one of the projects getting welded.  This should be the path to the executable relative to the project build directory, minus the leading `./`.
 - `arg`: After `command`, you can include a series of space-separate arguments.  These will get passed as `argv` to the subprogram when the welded binary runs. Alternatively, you can leave these blank, and use the welded binary's CLI to provide `argv` for each subprogram. 

----------
Copyright (c) Raytheon BBN Technologies 2021, All Rights Reserved

This document does not contain technology or Technical Data controlled under either
the U.S. International Traffic in Arms Regulations or the U.S. Export Administration

Distribution A: Approved for Public Release, Distribution Unlimited
