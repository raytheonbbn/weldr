# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import pathlib
from ..argdef import ArgDef
from ..stage import Stage

class InitArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument("-a", "--analyze", action="store_true",
                            help="Instead of welding the projects, return reports on the linking environment required by each binary within the project.")

class InitStage(Stage):
    def __init__(self, args):
        super().__init__(args)

    @property
    def name(self):
        return "init"

    @property
    def valid(self):
        return not self.args.analyze

    def run(self):
        self.l.info("Building all library models.")
        for makefile in pathlib.Path(self.args.model_dir).glob("*/Makefile"):
            self.l.debug("Making {!s}".format(makefile))
            self.exec('make', 'clean', cwd=makefile.parent, silence_stdout=not self.args.verbose)
            self.exec('make', cwd=makefile.parent, silence_stdout=not self.args.verbose)
