# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import pathlib
import re
from ..argdef import ArgDef
from ..stage import Stage

class CompileArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument('-D', '--use-dyn-models', action='store_true',
                            help="Switches weldr to dynamic modelling mode.  Required if a subprogram gets part of its functionality from shared libraries.")
        parser.add_argument('-X', '--link-static', action='store_true',
                            help="Switches weldr to link the entire binary statically.  This may not be possible if a 3rd-party library is unavailable statically.")


class CompileStage(Stage):
    def __init__(self, args):
        super().__init__(args)
        if self.args.use_dyn_models and self.args.link_static:
            raise Exception("You can't use dynamic models and link the binary statically at the same time.  It doesn't make sense.")
        self.obj_regex = re.compile('\t(\S+\.(o|a|la))')

    @property
    def name(self):
        return "compile"

    @property
    def predicessor(self):
        return frozenset(["generate_main"])

    def compile_main(self):
        # Update the file paths for the DWARF debug info.
        # Without this, DWARF only includes file names, meaning you can't
        # debug the binary outside its original compilation directory.
        abs_dot = '-fdebug-prefix-map=.={!s}'.format(self.working_dir)

        self.l.debug("Compiling main in {!s}".format(self.working_dir))

        # I include gcc as a requirement of weldr.  Once the subsidiary binaries are in an elf format,
        # I _think_ that it's safe to start using the known tools.

        # TODO: Test this.
        # When I looked at what the linker bit of GCC is actually doing, it spat out a monstrous amount of fedora-specific-looking stuff.
        # If we're trying to build outside of a GNU format, problematic.
        self.exec('gcc', '-c', '-g', abs_dot, './main.c', '-o', 'main.o', cwd=self.working_dir)

    def get_compile_cmd(self):
        if self.results.is_cpp:
            out = [ 'g++' ]
        else:
            out = [ 'gcc' ]

        if not self.args.use_dyn_models:
            out.append('--static')
        return out

    def get_global_link_dirs(self):
        lib_dir = pathlib.Path(self.working_dir, 'lib')
        return [ 
                 '-L{!s}'.format(lib_dir),
                 '-Wl,-rpath=$ORIGIN/lib'
               ]

    def get_inst_link_dirs(self):
        out = []
        for inst in self.results.inst_args:
            lib_dir = pathlib.Path(self.working_dir, inst, 'lib')
            rel_dir = pathlib.Path('$ORIGIN', inst, 'lib')
            # Tell the linker to search for libraries
            # where the libraries live.
            out.append("-L{!s}".format(lib_dir))
                
            # Because the weldr libraries are not stored in a
            # standard location, and I don't want to mess with the
            # global LD path, we bake the additional library search dirs
            # directly into the binary.  This means that the binary
            # must run from the working directory.
            out.append("-Wl,-rpath={!s}".format(rel_dir))
        return out

    def get_ext_link_dirs(self):
        return []

    def get_model_link_dirs(self):
        # For portability, we will move all model shared objects
        # into a model_lib directory in the working dir.

        # Tell the compiler and the runtime linker
        # where to find the new lib directory.
        lib_dir = pathlib.Path(self.working_dir, 'model_lib')
        rel_dir = pathlib.Path('$ORIGIN', 'model_lib')

        lib_dir.mkdir(parents=True, exist_ok=True)

        out = [
            '-L{!s}'.format(lib_dir),
            '-Wl,-rpath={!s}'.format(rel_dir)
        ]

        return out


    def get_wrapper_libs(self):
        out = []
        self.l.info("Adding wrapper libraries")
        lib_dir = pathlib.Path(self.working_dir, 'model_lib')
        for model in self.results.global_wrapper_ordering[::-1]:
            # Because we build both a .so and a .a, this works for both model modes.
            for p in model.rglob('libwrap*.so'):
                a = p.parent / p.name.replace('.so', '.a')

                # Copy the library into the model directory.  Pray to sokar this works.
                self.cp(p, lib_dir / p.name)
                self.cp(a, lib_dir / a.name)
                lib_name = p.name.replace("lib", "").replace(".so", "").replace('.a', '') 
                out.append('-l{!s}'.format(lib_name))
        return out


    def get_ext_libs(self):
        out = set()
        for cmd in self.results.insts:
            if cmd in self.results.ext_libs:
                out |= set(map(lambda x: "-l" + x, self.results.ext_libs[cmd]))
        return list(out)

    def get_wrap_cmds(self):
        out = []
        for model in self.results.global_wrapper_models:
            with open(pathlib.Path(model, "wrap.conf"), 'r') as f:
                for line in f:
                    out.append("-Wl,--wrap={:s}".format(line.replace('\n', '')))
        return out

    def get_inst_static_objs(self):
        out = []
        for inst in self.results.inst_args:
            inst_dir = pathlib.Path(self.working_dir, inst)
            inst_bins = set()
            with open(pathlib.Path(inst_dir, 'make.out'), 'r') as f:
                for line in f:
                    line = line.replace('\n', '')
                    m = self.obj_regex.match(line)
                    if m is not None:
                        obj_path = pathlib.Path(inst_dir, m.group(1))
                        obj_name = obj_path.name
                        if obj_name not in inst_bins:
                            obj_path = pathlib.Path(inst_dir, obj_name)
                            self.l.debug("Recovering file {:s} for {:s}".format(m.group(1), inst))
                            out.append("{!s}".format(obj_path))
                            inst_bins.add(obj_name)
            for p in inst_dir.glob("*.o"):
                if p.name not in inst_bins:
                    out.append("{!s}".format(p))
            for p in inst_dir.glob("*.a"):
                if p.name not in inst_bins:
                    out.append("{!s}".format(p))
            for p in inst_dir.glob("*.la"):
                lib_name = p.name.replace('.la', '.a')
                p = p.with_name(lib_name)
                if p.name not in inst_bins:
                    out.append("{!s}".format(p))
        return out

    def get_inst_stub_libs(self):
        out = []
        for inst in self.results.inst_args:
            inst_lib_dir = pathlib.Path(self.working_dir, inst, 'lib')
            for stub in inst_lib_dir.glob("lib*stub*.so"):
                out.append("-l{:s}".format(stub.name.replace("lib", "").replace(".so", "").replace(".a", "")))
        return out

    def run(self):
        
        self.l.info("Building the driver file.")
        self.compile_main()
        compile_cmd = self.get_compile_cmd()
        self.l.info("Adding global lib directory for linking.")
        compile_cmd += self.get_global_link_dirs()
        self.l.info("Adding instance lib directories for linking.")
        compile_cmd += self.get_inst_link_dirs()
        self.l.info("Adding external library link directories.")
        compile_cmd += self.get_ext_link_dirs()
        self.l.info("Adding wrapper model link directories.")
        compile_cmd += self.get_model_link_dirs()
        self.l.info("Adding symbol redefinition commands for wrapper libraries.")
        compile_cmd += self.get_wrap_cmds()
        self.l.info("Adding object files.")
        compile_cmd.append('{!s}'.format(pathlib.Path(self.working_dir, 'main.o')))
        compile_cmd.append('-Wl,--start-group')
        compile_cmd += self.get_inst_static_objs()
        compile_cmd.append('-Wl,--end-group')
        self.l.info("Adding external library link commands.")
        compile_cmd += self.get_ext_libs()
        self.l.info("Adding wrapper library link commands.")
        compile_cmd += self.get_wrapper_libs()
        self.l.info("Adding stub library link commands.")
        compile_cmd += self.get_inst_stub_libs()
        compile_cmd += ['-o', 'run_me']
        self.l.info("Attempting to compile the whole darn thing.")
        compile_record = pathlib.Path(self.working_dir, "compile_all.log")
        with open(compile_record, "w") as f:
            print(" ".join(compile_cmd), file=f)
        self.exec(*compile_cmd, cwd=self.working_dir)

        
