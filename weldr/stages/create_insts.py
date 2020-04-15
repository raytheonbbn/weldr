# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import pathlib
import re
from ..argdef import ArgDef
from ..stage import Stage

class InstArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument('-s', '--system-file', action='store',
                            help='Specify a system description file.  This instructs weldr on which subprocesses to launch.')


class CreateInstancesStage(Stage):
    def __init__(self, args):
        super().__init__(args)
        self.insts = {}
        self.inst_args = {}
        self.primary_inst = None
        self.blank_regex = re.compile("^\s*$")
        self.comment_regex = re.compile("\s*#")

    @property
    def name(self):
        return "create_insts"

    @property
    def predicessor(self):
        return frozenset(["detect_libs"])

    def resolve_command(self, cmd):
        #Separate the commands into commands and args.
        cmd = list(filter(lambda x: self.blank_regex.match(x) is None, cmd.split()))
        self.l.debug("Found command {!s}".format(cmd)) 

        is_primary = False
        if cmd[0] == "PRIMARY":
            is_primary = True
            cmd = cmd[1:]

        #Handle args gracefully
        if len(cmd) > 1:
            args = cmd[1:]
        else:
            args = []
        cmd = cmd[0]

        #Try and find the command in the working directory.
        if cmd not in self.results.cmds:
            raise Exception("Unknown command in system definition: {:s}".format(cmd))

        #Assign the command to a new instance.
        self.insts.setdefault(cmd, [])
        id_number = len(self.insts[cmd])
        inst_id = "{:s}{:d}".format(cmd, id_number).replace(".", "_")
        self.insts[cmd].append(inst_id)
        self.inst_args[inst_id] = args

        #If this is the primary instance, register as such.
        if is_primary:
            if self.primary_inst is not None:
                self.l.error("More than one primary instance specified: {:s} found, but {:s} already specified.".format(inst_id, self.primary_inst))
                raise Exception("Too many primary instances")
            self.primary_inst = inst_id

    def parse_insts(self):
        #Read the sytem file.
        sys_def_path = pathlib.Path(self.args.system_file)
        with open(sys_def_path, "r") as f:
            for l in f.readlines():
                l = l.replace("\n", "")
                #If it's a comment, ditch it.
                if self.comment_regex.match(l) is not None:
                    continue
                self.l.debug("Parsing line {:s}".format(l))
                
                #TODO: Eventually, script parsing should go here.

                #Parse the command.
                self.resolve_command(l)

    def create_insts(self):
        for cmd in self.insts:
            for inst in self.insts[cmd]:
                # Copy the instances
                cmd_path = pathlib.Path(self.working_dir, cmd)
                inst_path = pathlib.Path(self.working_dir, inst)
                self.cp(cmd_path, inst_path)

                # Rename libraries
                for obj in inst_path.rglob("*.so"):
                    new_name = obj.name.replace("lib", "lib{:s}".format(inst), 1)
                    obj.rename(obj.parent / new_name)

                for obj in inst_path.rglob("*.a"):
                    new_name = obj.name.replace("lib", "lib{:s}".format(inst), 1)
                    obj.rename(obj.parent / new_name)


    def clean_proj(self):
        for cmd in self.results.cmds:
            cmd_dir = pathlib.Path(self.working_dir, cmd)
            self.l.debug("Removing {!s}".format(cmd_dir))
            self.exec("rm", "-rf", cmd_dir)
        
    def run(self):
        if self.args.system_file is None:
            raise Exception("Missing a system file.")
        
        self.l.info("Parsing system definition file at {:s}".format(self.args.system_file))
        self.parse_insts()
        self.l.info("Creating instances")
        self.create_insts()
        self.l.info("Deleting unneeded project directories")
        self.clean_proj()
        self.results.insts = self.insts
        self.results.inst_args = self.inst_args
        self.results.primary_inst = self.primary_inst
