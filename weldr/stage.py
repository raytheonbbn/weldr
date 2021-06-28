# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
import logging
import pathlib
import pickle
import subprocess

class Stage:
    def __init__(self, args):
        self.args = args
        self.l = logging.getLogger("weldr.{:s}".format(self.name))
        self.l.debug("Initializing {!s}".format(self.__class__.__name__))
        
        if self.args.safe_mode:
            self.working_dir = pathlib.Path(self.args.temp_dir + "_" + self.name)
            self.results_file = self.working_dir / 'results.pickle'
        else:
            self.working_dir = pathlib.Path(pathlib.Path.cwd() / self.args.temp_dir)
            self.results_file = None

    @property
    def tag(self):
        #Base behavior: tag is the same as name.
        return self.name

    @property
    def name(self):
        raise Exception("No name defined.")

    @property
    def predicessor(self):
        #Base behavior; no predicessor.
        return frozenset()

    @property
    def valid(self):
        #Base behavior; always valid.
        return True

    @property
    def terminal(self):
        #Base behavior; always keep going.
        return False

    @property
    def preserve_old_working_dir(self):
        #Base behavior; always overwrite old working dirs.
        return False

    def start(self, results):
        self.results = results
        self.run()
        if self.results_file is not None:
            with open(self.results_file, 'wb') as f:
                pickle.dump(self.results, f)

    def run(self):
        raise Exception("Run is not implemented.")

    def cp(self, src, tgt, no_clobber=False):
        flags = '-r'
        if no_clobber:
            flags += 'n'

        rsrc = src.resolve()
         
        if not src.exists():
            raise Exception("Copy source {!s} doesn't exist.")

        self.exec("cp", flags, rsrc, tgt)

    def exec(self, *args, cwd=None, get_stdout=False, silence_stdout=False):
        buff = list()
        if get_stdout and silence_stdout:
            raise Exception("You're trying to record and silence stdout at the same time.  Pick one.")
        if silence_stdout:
            stdout = open('/dev/null', 'w')
        else:
            stdout = subprocess.PIPE

        self.l.trace("Executing {!s} at CWD {!s}".format(args, cwd))
        if cwd is None:
            p = subprocess.Popen(args, stdout=stdout)
        else:
            p = subprocess.Popen(args, stdout=stdout, cwd=cwd)

        if get_stdout:
            buff = [ l for l in p.stdout ]
        elif not silence_stdout:
            for l in p.stdout:
                self.l.info(l.decode('utf-8').rstrip())

        if p.wait() != 0:
            raise Exception("Exec of {!s} failed".format(args))
        return buff 
