# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
import pathlib
from ..argdef import ArgDef
from ..stage import Stage

class ResolveSymsStage(Stage):
    def __init__(self, args):
        super().__init__(args)

    @property
    def name(self):
        return "resolve_syms"

    @property
    def predicessor(self):
        return frozenset(["deconflict_syms"])

    def execute_torch(self):
        for (obj, tcf) in self.results.torch_paths.items():
            obj = self.working_dir / obj
            tcf = self.working_dir / tcf
            self.l.debug("Executing torch on {!s}".format(obj))
            # Not all libraries are ours.
            obj.chmod(0o775)
            with open(tcf, 'a') as f:
                if obj.name.endswith(".so"):
                    f.write('MOVE_SECTION,.dynamic,0x200000\n')
                    f.write('MOVE_SECTION,.dynstr,0x200000\n')
                    f.write('MAKE_SEGMENT,PT_LOAD,R|W|E,0x200000,.dynamic,.dynamic\n')
                    f.write('MAKE_SEGMENT,PT_LOAD,R|W|E,0x200000,.dynstr,.dynstr\n')
                f.write('SAVE,{:s},OVERWRITE\n'.format(obj.as_posix()))
            self.exec('torch', tcf.as_posix(), cwd=self.working_dir)

    def run(self):
        
        self.l.info("Modifying binaries.")
        self.execute_torch()
