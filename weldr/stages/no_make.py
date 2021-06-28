# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
from ..stage import Stage

class SkipMakeStage(Stage):
    def __init__(self, args):
        super().__init__(args)

    @property
    def valid(self):
        return False

    @property
    def tag(self):
        return "make"

    @property
    def name(self):
        return "no_make"
    
    @property
    def predicessor(self):
        return frozenset(["init"])

    @property
    def preserve_old_working_dir(self):
        return False

    def run(self):
        make_path = self.working_dir.parent / self.working_dir.name.replace('no_', '')
        if not make_path.exists():
            raise Exception("Could not find an extant make directory at {!s}".format(make_path))
        # Turns out, cp doesn't parse wildcards; bash does.
        for p in make_path.glob('*'):
            self.exec('cp', '-R', p.as_posix(), self.working_dir.as_posix()) 
        self.l.info("No rebuild requested.")
