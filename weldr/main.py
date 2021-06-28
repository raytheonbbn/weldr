# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
import argparse
import sys
from importlib import import_module
from pathlib import Path
from pkgutil import iter_modules
from .driver import Driver
from .argdef import ArgDef

def parse_args():
    parser = argparse.ArgumentParser('weldr - binary fusion')
    parser.add_argument('projects', nargs='+', help='The root directories of the projects to weld.')
    parser.add_argument('-t', '--temp-dir', action='store', required='true',
                        help='Directory path to use as scratch space.')
    parser.add_argument('-M', '--model-dir', default=Path(__file__).parents[1], help="Specify the directory that the weldr models live in")
    parser.add_argument('-S', '--safe-mode', action='store_true',
                        help='Execute weldr in safe mode.  Instead of overwriting the temp directory, weldr will create a new one for each phase of processing.  Useful for tracing bugs, and required if you want to be able to use the --skip-make flag later.')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Log the details.')
    parser.add_argument('--start-stage', action='store',
                        help='Start execution at a later stage.  Only valid if safe mode is on, and you have a safe mode folder for the predicessor stage.')
    parser.add_argument('--stop-stage', action='store',
                        help='Stop execution at an earlier stage.  Only valid if safe mode is on.')
    parser.add_argument('-L', '--log-file', action='store',
                        help='Write logs to a file out.log in the current directory.')
    parser.add_argument('-vv', '--very-verbose', action='store_true',
                        help='Log external tool calls.  There are a lot of them.')

    # Other arguments get added by the individual stages as they need them.
    # By the time this is done, the full spec will be available using -h.

    load_stage_args(parser)
    return parser.parse_args()

def load_stage_args(parser):
    # Load all modules in .stages
    for x in iter_modules([Path(__file__).parent / 'stages']):
        module = import_module('.stages.' + x.name, package='weldr')

    # This will dynamically register the ArgDef subclasses.
    for x in ArgDef.__subclasses__():
        # Execute each one as we find it.
        x().add_args(parser)

def main():
    args = parse_args()
    weldr = Driver(args)
    return weldr.run()

if __name__ == '__main__':
    sys.exit(main())
