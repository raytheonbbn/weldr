# Weldr

## Overview

Have you ever needed to debug a program, only to find that the code dives into a network socket and comes back with an unreadable error?  Weldr is here to help.

Weldr solves the problem of analyzing a multi-program or distributed system by recompiling all software into a single executable.

## Installation

### Compatibility

Weldr needs to be able to build the software it's processing.  Currently, that means it relies on having a locally-installed C and C++ compiler tool chain that it understands.

Weldr is tested against the standard C developer tools
on Fedora 29 and CentOS 7.  It should also be compatible
with Ubuntu 18.04 and other similarly-recent GNU linux distributions.

Because of fundamental differences in build tools,
binary formats, and library architecture, Weldr currently will not work on non-linux platforms, including Mac OS X and Windows.

It is unknown if BSD variants and other non-GNU linux environments are similar enough to support Weldr.  Please feel free to try, but no guarantees.

### Before you Begin

Weldr needs Python 3.7 or later to run, and a compatible version of pip to install.

- **Fedora:** `sudo dnf install python3 pip3`
- **Ubuntu:** `sudo apt install python3 python3-pip`
- **CentOS 7:** Python 3.7 is not available from yum.  Install manually. 

Currently, the weldr package is only set up for a development installation.  It is strongly recommended that you install it within a python virtual environment: 

[https://docs.python-guide.org/dev/virtualenvs/](https://docs.python-guide.org/dev/virtualenvs/)

### Installation Process

1. Make sure the C/C++ developer tools are installed.
	- **Fedora and CentOS:** Execute `sudo setup.sh`.  This will do the work for you.
	- **Ubuntu:** Execute the following:
		- `sudo apt update`
		- `sudo apt install build-essential`
	- **Other Linux:** Find and install an equivalent package or set of packages, according to the package manager on your system.
2. Make sure `torch` is installed.  See separate git project: 
3. Install the package:
	- **In a Python 3 virtualenv:** `python setup.py develop`
	- **Bare Metal (NOT TESTED):** `sudo python3 setup.py develop`

You're done!  Have fun welding systems together!

## Using Weldr

### Input Projects

To weld programs into a single binary, weldr needs to hook the build process for each executable that will be integrated into the final binary.

An input "project" for weldr is a directory where you can build the project by executing `make`.  

**NOTE:** Weldr hooks the make process by overriding the make automatic variables `CC` and `CXX`.  If the Makefile targets do not use these variables, weldr will not capture everything it needs.  Also, if you rely on a custom setting for one or both of these variables, weldr may override them with incorrect values.

### System Definition File

Weldr needs user input to tell it which executables
from the input projects should become sub-programs within the welded binary, and how they should
be executed.

The current system definition format is much like a simplified scripting language:

- Each line must be a command, a comment, or blank.
- A command must be the name of an executable from one of the input projects.
- Comment lines start with `#`, as in shell scripting.

Commands are launched in separate threads in the order in which they are defined.

This format may be expanded in the future to include default arguments and IPC configurations like IP addresses or stream redirection.

### Running Weldr

Weldr exposes the following command line interface:

	weldr [options] -t path -s path project [--] [project ...]

**Required Arguments:**

- `-t path` Path to a working directory.  Weldr creates a lot of temporary files, so giving it a clean directory is advisable.  Weldr will create the working directory if it doesn't exist, and overwrite it if it does.
- `-s path` Path to the system definition file.
- `project` Path to the directory containing a make-based project to use as input.  You can specify more than one such path; weldr will process all of them.

**Useful Options:**

- `-D` Enable dynamic library modeling.  If any sub-program depends on a dynamic library higher up the chain than libc, you will need to specify this.
- `-S` Safe mode.  Weldr is broken up into multiple stages.  In safe mode, weldr will create a new working directory for each stage, with the stage name appended to the name you provided in the `-t` option.  The final results will be in `<dir_name>_compile`

### Running the Welded Binary

If weldr succeeds, it will produce a single binary named `run_me` in the working directory.  Running this binary will launch each subprogram in a separate thread.

 Running works a little differently than a normal program.  For full details, execute `./run_me --help`.  The help message will include usage information customized to the particular binary, including the names of the subprograms.

**WARNING:** If dynamically-linked, the welded binary depends on the working directory it was produced in for custom libraries.  Do not delete the working directory.

#### Standard Streams

Because a welded binary contains multiple programs all expecting sole access to stdin, stdout, and stderr, weldr redirects the streams for each subprogram into a separate file.  

These files are, by default, put in `/tmp`, and are named `<subprogram name>_<stream name>`

You can change the directory via the command line.  See the output of `--help` for more info.

#### Command Line Args

As with standard streams, each subprogram expects its own argv.  Weldr provides this by exposing an option `--<subprogram name>-args` for each subprogram.  The arguments must be presented in a comma-separated list.  Again, see the output of `--help` for specific details.


----------


Copyright (c) Raytheon BBN Technologies 2020, All Rights Reserved.