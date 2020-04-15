# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import itertools
import pathlib
import re
from ..argdef import ArgDef
from ..stage import Stage

class LibsArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument('-l', '--lib-model', action='append', default=[],
                            help='Manually specify a library model to use when welding.')
        parser.add_argument('-I', '--ignore-models-in-libs', action='store_true',
                            help='Do not duplicate external libraries if they override instance models.')
        parser.add_argument('--blacklist-lib', action='append', dest='blacklist_libs', default=[],
                            help='Add a library to blacklist when welding, if one gives you trouble.')




class DetectLibsStage(Stage):
    def __init__(self, args):
        super().__init__(args)
        self.is_cpp = False
        self.cmds = set()
        self.cmd_stub_models = dict()
        self.cmd_stub_orderings = dict()
        self.global_wrapper_models = set()
        self.global_wrapper_ordering = None
        self.ext_lib_paths = dict()
        self.ext_libs = dict()
        self.ext_lib_dirs = set()
        self.glibc_regex = re.compile("@\S+$")
        self.ext_lib_dir_regex = re.compile("\s-L(\S+)")
        self.ext_lib_regex = re.compile("\s-l(\S+)")
        self.ext_lib_version_regex = re.compile("\.so\..*")
        self.ext_lib_name_regex = re.compile("lib(\S+)\.so")
        self.ext_lib_blacklist = {
            'linux-vdso.so',    # Not actually a library, but something far more terrifying.
            'ld-linux-',        # The ELF dynamic linker.  REALLY don't try to weld.
            'ld.so',            # Obsolete dynamic linker, just in case.
            'libc.so',          # The bottom of the system.  Do not try to weld.
            'libstdc++.so',     # C++ standard library.  Weird ELF format; borks torch.
            'libgcc_s.so'       # GCC helper functions.  I'd be surprised if we needed this.
        }
        self.ext_lib_blacklist |= set(self.args.blacklist_libs)
        self.all_wrap_symbols = dict()
        self.all_stub_symbols = dict()
        self.wrap_symbols = dict()
        self.stub_symbols = dict()
        self.bin_wrap_syms = dict()
        self.bin_stub_syms = dict()
        self.USES_WRAP = "USES_WRAP"
        self.NEW_WRAP = "NEW_WRAP"
        self.OVERRIDE_WRAP = "OVERRIDE_WRAP"
        self.USES_STUB = "USES_STUB"
        self.NEW_STUB = "NEW_STUB"
        self.OVERRIDE_STUB = "OVERRIDE_STUB"

    @property
    def name(self):
        return "detect_libs"

    @property
    def predicessor(self):
        return frozenset(["make"])
    
    def load_wrap_symbols(self, all_models=False, lib_path=None):
        # Load symbols for wrapper models
        out = dict()

        if lib_path is None:
            model_path = pathlib.Path(self.args.model_dir)
        else:
            model_path = lib_path

        for p in model_path.rglob("wrap.conf"):
            if not all_models and p.parent not in self.global_wrapper_models:
                continue
            with open(p, 'r') as f:
                for l in f:
                    line = l.strip()
                    out[line] = p.parent

        if all_models:
            dest = self.all_wrap_symbols
        else:
            dest = self.wrap_symbols

        dest.update({ self.glibc_regex.sub("", k): v for (k, v) in out.items() })

    def load_stub_symbols(self, cmd, all_models=False, lib_path=None):
        # Load symbols for stub models
        out = dict()
        if lib_path is None:
            model_path = pathlib.Path(self.args.model_dir)
        else:
            model_path = lib_path

        for p in model_path.rglob("stub.conf"):
            if not all_models and p.parent not in self.cmd_stub_models[cmd]:
                continue
            with open(p, 'r') as f:
                for l in f:
                    line = l.strip()
                    out[line] = p.parent

        if all_models:
            dest = self.all_stub_symbols
        else:
            dest = self.stub_symbols.setdefault(cmd, dict())
        dest.update({ self.glibc_regex.sub("", k): v for (k, v) in out.items() })
    
    def symbols_from_file(self, path):
        # Read all symbols from an ELF file.
        undef = set()
        defined = set()
        def parse_line(l):
            line = l.decode('utf-8').strip()
            symbol_arr = line.split()
            if len(symbol_arr) < 2:
                return
            try:
                symbol = symbol_arr[-1]
            except:
                print(symbol_arr)
                exit(-1)
            if "*UND*" in line:
                undef.add(symbol)
            else:
                defined.add(symbol)

        for l in self.exec('objdump', '-t', path, get_stdout=True):
            parse_line(l)

        for l in self.exec('objdump', '-T', path, get_stdout=True):
            parse_line(l)

        defined = set(map(lambda x: self.glibc_regex.sub("", x).strip(), defined))
        undef = set(map(lambda x: self.glibc_regex.sub("", x).strip(), undef))
        
        return (defined, undef)

    def identify_cmds(self):
        # Identify the subprogram names based on the working directory structure.
        # Each command comes out of make as a directory with a make.out file
        for x in self.working_dir.rglob("make.out"):
            self.cmds.add(x.parent.name)

    def identify_ext_lib_paths(self):
        # Use the saved copy of the executable to identify dynamic library paths.
        # I saved off a.out, because ldd is way more precise than anything I could have done.
        
        # If we're not using dynamic models, this step is currently useless.
        # TODO Find if there's a static copy of the dyn lib. being imported.
        # TODO Find if the compile command includes any static libs itself.
        # TODO If we want to go static and the build used dynamic, problem.
        for x in self.cmds:
            cmd_path = pathlib.Path(self.working_dir, x, 'a.out')
            self.l.debug('Ext libs for {:s}:'.format(x))
            for l in self.exec('ldd', cmd_path, get_stdout=True):
                res = l.decode('utf-8').lstrip().replace("\n", "").split("=>")
                if len(res) == 2:
                    (lib, path) = res
                elif len(res) == 1:
                    lib = None
                    path = res[0]
                else:
                    raise Exception("Unexpected length: {:d}".format(len(res)))

                if 'not found' in path:
                    raise Exception("Could not find library {!s}".format(lib))

                path = pathlib.Path(path.lstrip().split()[0])
                if lib is None:
                    lib = path.name
                lib = lib.strip()
                lib = self.ext_lib_version_regex.sub(".so", lib)

                blacklist = list(filter(lambda x: lib.startswith(x), self.ext_lib_blacklist))
                if len(blacklist) > 0:
                    self.l.debug('\tBLACKLIST: {!s}'.format(lib))
                else:
                    m = self.ext_lib_name_regex.match(lib)
                    if m is not None:
                        lib = m.group(1)
                    
                    if not self.args.use_dyn_models:
                        old_path = path
                        name = path.name
                        name = self.ext_lib_version_regex.sub('.a', name)
                        path = path.parent / name
                        if not path.exists():
                            self.l.error("Using static models, but can't find a static version of {!s}".format(old_path))
                            raise Exception("Missing static library")

                    if lib in self.ext_lib_paths and self.ext_lib_paths[lib] != path:
                        self.l.error("Library path mismatch for {:s}:".format(lib))
                        self.l.error("{:s}'s path: {!s}".format(cmd, path))
                        self.l.error("Known location: {!s}".format(path))
                        raise Exception("Library path mismatch")

                    self.l.debug('\t{!s}'.format(lib)) 
                    self.ext_lib_paths[lib] = path
                    self.detect_lib_model(lib, x)


    def add_lib_model(self, lib_dir, cmd=None):
        #Copy all stub objects into the relevant instances. 
        if self.args.use_dyn_models:
            ext = 'so'
        else:
            ext = 'a'
        for x in lib_dir.rglob("libstub*.{:s}".format(ext)):
            #If we didn't specify a command, copy to all command dirs.
            if cmd == None:
                self.l.debug("Adding stub library {!s} to all programs".format(x))
                for cmd in self.cmds:
                    if cmd in self.cmd_stub_models and x in self.cmd_stub_models[cmd]:
                        continue

                    cmd_dir = pathlib.Path(self.working_dir, cmd)
                    cmd_lib_dir = pathlib.Path(self.working_dir, cmd, 'lib')
                    cmd_lib_dir.mkdir(parents=True, exist_ok=True)
                
                    cmd_lib_path = cmd_lib_dir / x.replace("lib", "lib{:s}".format(cmd), 1)

                    self.cp(x, cmd_lib_path, no_clobber=True)
                    self.exec("ar", "x", x.name, cwd=cmd_dir)
                    self.exec("rm", x.name, cwd=cmd_dir)
                        
                    self.cmd_stub_models.setdefault(cmd, set()).add(lib_dir)
                    self.load_stub_symbols(cmd, lib_path=lib_dir)
            else:
                self.l.debug("Adding stub library {!s} to {:s}".format(x, cmd))

                cmd_dir = pathlib.Path(self.working_dir, cmd)
                cmd_lib_dir = pathlib.Path(self.working_dir, cmd, 'lib')
                cmd_lib_dir.mkdir(parents=True, exist_ok=True)

                cmd_lib_path = cmd_lib_dir / x.name
                self.cp(x, cmd_lib_path, no_clobber=True)
                self.cmd_stub_models.setdefault(cmd, set()).add(lib_dir)
                self.load_stub_symbols(cmd, lib_path=lib_dir)

        for x in lib_dir.rglob("wrap.conf"):
            self.l.debug("Adding wrapper library {!s}".format(x))
            self.global_wrapper_models.add(lib_dir)
            self.load_wrap_symbols(lib_path=lib_dir)

    def scan_ext_libs(self):
        # This step gets hard. 
        move_to_instance = dict()
        move_to_global = set()

        global_lib_path = pathlib.Path(self.working_dir, "lib")
        global_lib_path.mkdir(parents=True, exist_ok=True)

        lib_wrap_syms = dict()
        lib_stub_syms = dict()

        for (cmd, libs) in self.ext_libs.items():
            for lib in libs:
                self.l.debug("Considering external library {!s}".format(lib))
                if lib not in self.ext_lib_paths:
                    self.l.debug("\t{!s} didn't match a known external lib.")
                    continue

                lib_path = self.ext_lib_paths[lib]
                self.l.debug("\tFound library {!s} at path {!s}".format(lib, lib_path))

                res = self.detect_modeled_symbols(lib_path, cmd)
                
                if len(res) == 0:
                    continue
                
                if self.USES_WRAP in res or self.NEW_WRAP in res:
                    move_to_global.add(lib_path)
                    wrap_syms = set()
                    if self.USES_WRAP in res:
                        wrap_syms |= res[self.USES_WRAP]
                    if self.NEW_WRAP in res:
                        wrap_syms |= res[self.NEW_WRAP]
                    if len(wrap_syms) != 0:
                        lib_wrap_syms[lib_path] = wrap_syms

                # Detect if the lib needs to be copied to an instance.
                if not self.args.ignore_models_in_libs and (self.USES_STUB in res or self.NEW_STUB in res or self.OVERRIDE_STUB in res):
                    if lib_path in move_to_global:
                        move_to_global.remove(lib_path)
                    move_to_instance.setdefault(cmd, set()).add(lib_path)
                    stub_syms = set()
                    if self.USES_STUB in res:
                        stub_syms |= res[self.USES_STUB]
                    if self.NEW_STUB in res:
                        stub_syms |= res[self.NEW_STUB]
                    if self.OVERRIDE_STUB in res:
                        stub_syms |= res[self.OVERRIDE_STUB]
                    if len(stub_syms) != 0:
                        lib_stub_syms[lib_path] = stub_syms


                # Add detected libs
                self.add_detected_models(lib_path, cmd, res)

        if len(move_to_global) != 0:
            self.l.debug("Libraries that need to be copied into the project:")
            for l in move_to_global:
                lib_name = self.ext_lib_version_regex.sub('.so', l.name)
                self.l.debug("\t{!s}".format(lib_name))
                self.cp(l, global_lib_path / lib_name)
                rel_lib_path = (global_lib_path / lib_name).relative_to(self.working_dir)
                self.bin_wrap_syms[rel_lib_path] = lib_wrap_syms[l]

        if len(move_to_instance) != 0:
            self.l.debug("Libraries that need to be copied to specific instances:")
            for (cmd, ls) in move_to_instance.items():
                cmd_lib_path = pathlib.Path(self.working_dir, cmd, "lib")
                cmd_lib_path.mkdir(parents=True, exist_ok=True)
                self.l.debug("\t{:s}".format(cmd))
                for l in ls:
                    lib_name = self.ext_lib_version_regex.sub('.so', l.name)
                    is_blacklist = len(list(filter(lambda x: lib_name.startswith(x), self.ext_lib_blacklist)))
                    if is_blacklist:
                        self.l.debug("\t\tBLACKLIST {!s}".format(lib_name))
                    else:
                        self.l.debug("\t\t{!s}".format(lib_name))
                        self.cp(l, cmd_lib_path / lib_name)
                        rel_lib_path = (cmd_lib_path / lib_name).relative_to(self.working_dir)
                        self.bin_stub_syms[rel_lib_path] = lib_stub_syms[l]


    def detect_modeled_symbols(self, path, cmd):
        out = dict()
        (defined, undef) = self.symbols_from_file(path)

        # Compute the sets of wrapper symbols used in the binary.
        wrap_used = undef & self.wrap_symbols.keys()
        all_wrap_used = undef & self.all_wrap_symbols.keys()
        new_wrap_used = all_wrap_used - wrap_used

        #Compute the set of stub symbols used in the binary.
        stub_used = undef & self.stub_symbols.get(cmd, dict()).keys()
        all_stub_used = undef & self.all_stub_symbols.keys()
        new_stub_used = all_stub_used - stub_used

        #Compute if this library defines any modeled symbols.
        wrap_override = defined & self.all_wrap_symbols.keys()
        stub_override = defined & self.all_stub_symbols.keys()

        if len(wrap_used) != 0:
            out[self.USES_WRAP] = wrap_used
        if len(new_wrap_used) != 0:
            out[self.NEW_WRAP] = new_wrap_used
        if len(wrap_override) != 0:
            self.l.warn("File {!s} defines symbols overridden by wrapper models:".format(path))
            for x in wrap_override:
                self.l.warn("\t{:s}".format(x))
            out[self.OVERRIDE_WRAP] = wrap_override
        if len(stub_used) != 0:
            out[self.USES_STUB] = stub_used
        if len(new_stub_used) != 0:
            out[self.NEW_STUB] = new_stub_used
        if len(stub_override) != 0:
            self.l.warn("File {!s} defines symbols overridden by stub models:".format(path))
            for x in stub_override:
                self.l.warn("\t{:s}".format(x))
            out[self.OVERRIDE_STUB] = stub_override
        return out

    def add_detected_models(self, path, cmd, res):
        new_lib_paths = set()

        if self.USES_WRAP in res:
            self.l.debug("Binary {!s} needs to wrap the following known symbols".format(path))
            for x in res[self.USES_WRAP]:
                self.l.debug('\t{:s} => {!s}'.format(x, self.all_wrap_symbols[x]))

        if self.NEW_WRAP in res:
            self.l.debug("Binary {!s} needs to wrap the following new symbols".format(path))
            for x in res[self.NEW_WRAP]:
                self.l.debug("\t{:s} => {!s}".format(x, self.all_wrap_symbols[x]))
                new_lib_paths.add(self.all_wrap_symbols[x])

        if self.NEW_STUB in res or self.OVERRIDE_STUB in res:
            self.l.debug("Binary {!s} needs to stub the following symbols for ".format(path, cmd))

        if self.NEW_STUB in res:
            for x in res[self.NEW_STUB]:
                self.l.debug("\t{:s} => {!s}".format(x, self.all_stub_symbols[x]))
                new_lib_paths.add(self.all_stub_symbols[x])

        if self.OVERRIDE_STUB in res:
            for x in res[self.OVERRIDE_STUB]:
                self.l.debug("\t{:s} => {!s}".format(x, self.all_stub_symbols[x]))
                new_lib_paths.add(self.all_stub_symbols[x])

        for x in new_lib_paths:
            self.add_lib_model(x, cmd=cmd)

    def add_required_lib_models(self):
        required_libs_conf = pathlib.Path(self.args.model_dir, "required_libs.conf")
        with open(required_libs_conf, "r") as f:
            for lib in f.readlines():
                lib = lib.replace("\n", "")
                self.l.debug("Required library: {:s}".format(lib))
                self.add_lib_model(pathlib.Path(self.args.model_dir, lib))

    def add_user_requested_lib_models(self):
        for lib in self.args.lib_model:
            self.l.debug("User-requested library: {:s}".format(lib))
            self.add_lib_model(pathlib.Path(self.args.model_dir, lib))

    def handle_libtool_lib(self, la_path):
        raise Exception("Libtool handling not implemented.")

    def detect_lib_model(self, library, cmd):
        lib_model_path = pathlib.Path(self.args.model_dir, "fake_{:s}".format(library))
        if lib_model_path.exists():
            self.add_lib_model(lib_model_path, cmd=cmd)
        else:
            self.ext_libs.setdefault(cmd, set()).add(library)

    def process_make_commands(self):
        #Find all make.out files
        # TODO: We won't always have make.out, since we won't always have make.
        # Need to think about replacing that with the dynamic linking directives.
        for x in self.working_dir.rglob("make.out"):
            self.l.debug("Make result file: {!s}".format(x))
            with open(x, "r") as f:
                #Extract the data from the file.
                data = f.read().split("\n")
                exe = pathlib.Path(data[0]).name
                cmd = data[1]
                self.l.debug("Command for {:s}:\n\t{:s}".format(exe, cmd))
                
                #Check for C++
                if "g++" in cmd or "CXX" in cmd or "c++" in cmd:
                    self.is_cpp = True

                #Check for external lib directories
                for m in self.ext_lib_dir_regex.finditer(cmd):
                    self.l.debug("External lib dir: {:s}".format(m.group(1)))
                    self.ext_lib_dirs.add(m.group(1))

                #Check for libtool libs
                for la in x.parent.rglob("*.la"):
                    self.l.debug("Libtool library file: {!s}".format(la))
                    self.handle_libtool_lib(la)

                #Check for modeled symbols
                for binary in x.parent.glob("a.out"):
                    res = self.detect_modeled_symbols(binary, exe)
                    self.add_detected_models(binary, exe, res)

    def compute_model_set_dependencies(self, models):
         # Load dependencies for all models.
         model_to_deps = dict()
         model_order = []
         seen = set()
         for model in models:
             dep_file = pathlib.Path(model, 'dependencies.conf')
             deps = set()
             if dep_file.exists():
                 with open(dep_file, 'r') as f:
                     for line in f:
                         line = line.replace('\n', '')
                         dep_path = pathlib.Path(self.args.model_dir, line)
                         if not dep_path.exists():
                             raise Exception("Couldn't find dependency {:s} of {:s} at path {!s}".format(line, model.name, dep_path))
                         if dep_path not in models | self.global_wrapper_models:
                             raise Exception("Missing dependency {:s} for {:s}".format(line, model.name))
                         deps.add(pathlib.Path(self.args.model_dir, line))
             model_to_deps[model] = deps
         while len(seen) != len(model_to_deps):
             for model in model_to_deps:
                 if model not in seen:
                     deps = model_to_deps[model]
                     if model not in self.global_wrapper_models:
                         deps -= self.global_wrapper_models
                     if deps <= seen:
                         seen.add(model)
                         model_order.append(model)
         return model_order

    def compute_model_dependencies(self):
        for cmd in self.cmd_stub_models:
            model_order = self.compute_model_set_dependencies(self.cmd_stub_models[cmd])
            self.cmd_stub_orderings[cmd] = model_order
        self.global_wrapper_ordering = self.compute_model_set_dependencies(self.global_wrapper_models)
            

    def run(self):
        self.l.info("Loading model configs.")
        self.load_wrap_symbols(all_models=True)
        self.load_stub_symbols(None, all_models=True)
        self.l.info("Finding command directories.")
        self.identify_cmds()
        self.l.info("Identifying dynamic imports from binary.")
        self.identify_ext_lib_paths()
        self.l.info("Adding required lib models.")
        self.add_required_lib_models()
        self.l.info("Adding command-line defined models.")
        self.add_user_requested_lib_models()
        self.l.info("Detecting required library models")
        self.process_make_commands()
        self.l.info("Detecting model-dependent shared libraries")   
        self.scan_ext_libs()
        self.l.info("Computing dependency order for models")
        self.compute_model_dependencies()
        self.results.is_cpp = self.is_cpp
        self.results.cmds = self.cmds
        self.results.cmd_stub_models = self.cmd_stub_models
        self.results.cmd_stub_orderings = self.cmd_stub_orderings
        self.results.global_wrapper_models = self.global_wrapper_models
        self.results.global_wrapper_ordering = self.global_wrapper_ordering
        self.results.ext_libs = self.ext_libs
        self.results.ext_lib_dirs = self.ext_lib_dirs
        self.results.bin_stub_syms = self.bin_stub_syms
        self.results.bin_wrap_syms = self.bin_wrap_syms
