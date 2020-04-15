# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import pathlib

from ..argdef import ArgDef
from ..stage import Stage

class MakeArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument('-b', '--build-tool', action='store', default='cc',
                            help='Specify a build tool.  Accepts "libtool" and "cc".')
        parser.add_argument('--make-arg', action='append', dest='make_args', default=[],
                            help='Add an argument to be passed to Make.')


class MakeStage(Stage):
    def __init__(self, args):
        super().__init__(args)
        self.tracer_script = pathlib.Path(self.args.model_dir, "trace_make.sh")

    @property
    def valid(self):
        return True

    @property
    def name(self):
        return "make"

    @property
    def predicessor(self):
        return frozenset(["init"])

    def run(self):
        
        for d in self.args.projects:
            self.l.info("Handling project at {:s}".format(d))
            self.l.debug("Extra make arguments: {!s}".format(self.args.make_args))
            self.exec('make', 'clean', cwd=d, silence_stdout=not self.args.verbose)
            if self.args.build_tool == "cc":
                self.exec('make', 
                    'CC={!s} -c cc -o {!s} -- '.format(self.tracer_script, self.working_dir),
                    'CXX={!s} -c c++ -o {!s} -- '.format(self.tracer_script, self.working_dir),
                    *self.args.make_args,
                    cwd=d, silence_stdout=not self.args.verbose)
            elif self.args.build_tool == "libtool":
                self.exec('make',
                    'LIBTOOL="{!s} -c libtool -o {!s} -- "'.format(self.tracer_script, self.working_dir)
                    , cwd=d, silence_stdout=not self.args.verbose)
            else:
                raise Exception("Unrecognized build tool: {:s}".format(self.args.build_tool))

