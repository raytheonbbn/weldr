# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import pathlib
import re
from ..stage import Stage

class DeconflictSymbolsStage(Stage):
    def __init__(self, args):
        super().__init__(args)
        self.defined_syms_to_objs = dict()
        self.undefined_syms_to_objs = dict()
        self.torch_paths = dict()
        self.objdump_sym_regex = re.compile("([0-9a-f]{16})\s+([lg! ])([w ])([C ])([W ])([Ii ])([dD ])([fFO ])\s+(\S+)\s+([0-9a-f]{16})\s+(\S+)")
        self.cpp_namespace_regex = re.compile("([a-zA-Z0-9_]+)::")
        self.cpp_special_name_regex = re.compile("for ([a-zA-Z0-9_]+)")
        self.cpp_bare_func_regex = re.compile("([a-zA-Z0-9_]+)(\[abi:.+\])?\(")

    
    @property
    def name(self):
        return "deconflict_syms"

    @property
    def predicessor(self):
        return frozenset(["create_insts"])

    def demangle(self, symbol):
        for l in self.exec('c++filt', symbol, get_stdout=True):
            return l.decode('utf-8').replace('\n', '')

    def check_cxx(self, symbol):
        #Symbol is a C++ symbol if it demangles to something different.
        return symbol != self.demangle(symbol)

    def rename_cxx(self, symbol):
        # Mangled C++ symbols aren't straightforward to rename.
        # I forget why, but just clobbering them breaks things.
        # So, we tease apart just enough of the mangled name to add a namespace
        # to the beginning of the symbol.

        # Fastest way I can think to do this is to demangle the symbol, try and find
        # it's first namespace component (e.g.: "foo::..."), find that name
        # in the mangled symbol, and then put the instance name immediately
        # to the left.

        name = None
        demang = self.demangle(symbol)

        # Get rid of template markers.
        # They gunk up name recognition, and we preserve them when we rewrite the name.
        # Unfortunately, removing them intelligently is beyond regexes.
        
        # Replace operators with carrots with illegal symbols.
        demang = demang.replace('operator<<', '^_^') \
            .replace('operator>>', '@_@') \
            .replace('operator<', '^.^') \
            .replace('operator>', '@.@') \
            .replace('operator->', '*.*')

        orig_demang = demang

        # Scan for carrots
        open_br = 0
        curr_start = -1
        demang_len = len(demang)
        for i in range(0, demang_len):
            if demang[i] == '<':
                if open_br == 0:
                    curr_start = i
                open_br += 1
            if demang[i] == '>':
                open_br -= 1
                if open_br == 0:
                    size = i - curr_start + 1
                    demang = demang[:curr_start] + ('#' * size) + demang[i+1:]

        if '<' in demang or '>' in demang:
            raise Exception('There are still carrots in demang! {:s}'.format(demang))

        # Reverse illegal symbol masking.
        demang = demang.replace('^_^', 'operator<<') \
            .replace('@_@', 'operator>') \
            .replace('^.^', 'operator<') \
            .replace('@.@', 'operator>') \
            .replace('*.*', 'operator->') \
            .replace('#', '')

        # Find the outermost namespace
        # Nicely enough, this will also capture the class name
        # if the symbol is a property of something in the default namespace
        m = self.cpp_namespace_regex.search(demang)
        if m is not None:
            namespace = m.group(1)
            # Ignore most things in std
            # TODO: I don't want to ignore them if they're overridden by an instance model.
            if namespace != 'std':
                name = namespace
            else:
                return None
        
        # Some artifacts in the default namespace won't have a '::'
        # A big group of these are language machinery, which is of the form <whatever this is> for <name>.
        m = self.cpp_special_name_regex.search(demang)
        if m is not None:
            name = m.group(1)

        # Some artifacts are just functions.
        # Give the poor thing a namespace.
        m = self.cpp_bare_func_regex.match(demang)
        if m is not None:
            name = m.group(1)

        # Some artifacts are operators in the default namespace.
        # I seriously hope none of these are stateful.
        if demang.startswith('operator'):
            return None

        if name is None:
            self.l.error("\tSymbol: {:s}".format(symbol))
            self.l.error("\tDemangled Name: {:s}".format(demang))
            self.l.error("\tThis thing doesn't have a namespace or recognizable name...?")
            return None
        else:
            old_token = '{:d}{:s}'.format(len(name), name)
            new_token = '{{0:d}}{{1:s}}{:s}'.format(old_token)
            new_symbol = symbol.replace(old_token, new_token)
            if symbol == new_symbol:
                self.l.error('\tNo instance of name token {:s} in {:s}'.format(old_token, symbol))
                raise Exception("Token mismatch")
            else:
                return new_symbol
        
    def rename_normal(self, symbol):
        # We rename symbols as inst_symbol.  
        # As stated in run_objdump, for the program to compile correctly,
        # the kinds of symbols we deduplicate must already be consistent
        # if the instance is a valid ELF program.  Thus,
        # we just need to rename by instance.
        return '{{1:s}}_{:s}'.format(symbol)

    def replace_symbol(self, obj, symbol, new_symbol, is_defined):
        self.l.debug("\tReplacing {:s} with {:s} in {!s}".format(symbol, new_symbol, obj))
        if is_defined:
            version = 1
        else:
            version = 0
        header = False
        obj = obj.relative_to(self.working_dir)
        if obj not in self.torch_paths:
            torch_path =obj.parent / "{:s}.tcf".format(obj.name)
            self.torch_paths[obj] = torch_path
            self.l.debug("Creating torch file {:s}".format(self.torch_paths[obj].as_posix()))
            header = True
        with open(self.working_dir / self.torch_paths[obj], 'a') as f:
            if header:
                f.write('LOAD,ELF,{:s}\n'.format(obj.as_posix()))
            f.write('RENAME_SYMBOL,{:s},{:s},BOTH,PERMISSIVE\n'.format(symbol, new_symbol))
            f.write('SET_SYMBOL_VERSION,{:s},{:d},PERMISSIVE\n'.format(new_symbol, version))

    def add_lib_dependencies(self, obj):
        with open(self.working_dir / self.torch_paths[obj], 'a') as f:
            rpath = []
            for model in self.results.global_wrapper_ordering:
                rpath.append((model / 'lib').as_posix())
                for p in model.rglob('libwrap*.so'):
                    self.l.debug('\tAdding a dependency between {!s} and {!s}'.format(obj, p))
                    f.write('MAKE_DYN_TAG,DT_NEEDED,{:s}\n'.format(p.name))
            rpath = ':'.join(rpath)
            #f.write('MAKE_DYN_TAG,DT_RPATH,{:s}\n'.format(rpath))

    def objdump_all(self, inst_dir):
        for obj in inst_dir.rglob("*.o"):
            self.run_objdump(obj)
        for obj in inst_dir.rglob("*.so"):
            self.run_objdump(obj)

    def run_objdump(self, obj):
            self.l.debug("Symbols for {!s}".format(obj))
            for l in self.exec("objdump", "-t", obj, get_stdout=True):
                line = l.decode('utf-8').replace('\n', '')
                m = self.objdump_sym_regex.search(line)
                if m is not None:
                    #Recover the name and the section of each symbol
                    scope = m.group(2)
                    weakness = m.group(3)
                    section = m.group(9)
                    name = m.group(11)
                    self.l.debug("\t{!s} ({!s})".format(name, section))
                    if scope == 'l':
                        # This is a local symbol.
                        # According to the docs, these can exist in multiple files
                        # at once without conflicting.  The linker knows what to do.
                        #
                        # Annecdotally, I've only ever seen them used for
                        # introspective definitions, like actual sections in the object.
                        #
                        # There are three other possible values for this field:
                        # - 'g': This is a global symbol, and needs to be deconflicted.
                        # - ' ': This is neither global nor local; it's probaly undefined.
                        # - '!': This is both.  Docs say this is a bug.
                        self.l.debug("\t\tLocal symbol; ignoring")
                        continue
                    elif weakness == 'w':
                        # This is a weak symbol.
                        # These are like globals in that they make it into the final
                        # binary, but a global with the same name will override it.
                        # 
                        # Unfortunately, all the docs I've found so far say
                        # nothing about how multiple colliding weak symbols get handled.
                        # These were originally a discouraged way to make the compiler
                        # clean up your weird C code architecture, and the basic
                        # C linker docs stop there.  However, I've noticed that all 
                        # C++ class artifacts are defined as weak symbols.
                        #
                        # Experimentally, the compiler takes the first definition of a weak
                        # symbol it finds.  This results in weird behavior if you have two
                        # different versions of a class with the same name.  The feaures of
                        # the first class defined will win out, but this is decided
                        # on a method-by-method and field-by-field basis;
                        # you can get mixed implementations if you compile two or more objects
                        # with classes named the same thing, but different methods and fields.
                        # I assume there's a valid object-oriented reason for this, but
                        # I'm coming at the problem from the wrong end.
                        #
                        # Without completely reverse-engineering the C++ compiler,
                        # I think it's safe to say that we should treat weak symbols
                        # as shared across all objects in an instance.  If you get duplicates
                        # within an instance, assume the code handles it correctly.
                        # If you get duplicates across instances, deconflict them by instance,
                        # as you would with globals.
                        #
                        # Cute thing; if you just always deconflict by instance,
                        # it has the same effect.  More work for python, but easier to implement.
                        self.defined_syms_to_objs.setdefault(name, set()).add(obj)
                        self.l.debug("\t\tWeak symbol; tracking")
                    elif section == "*UND*":
                        # This is an undefined symbol.  This means that it's either
                        # meant to be filled by dynamic linking, or by a defined symbol
                        # in another relocatable object.  When we deconflict the defined version,
                        # we need to redefine these so they reflect the new name.
                        self.undefined_syms_to_objs.setdefault(name, set()).add(obj)
                        self.l.debug("\t\tUndefined symbol; tracking");
                    else:
                        # This is a defined, global symbol.
                        # These aren't allowed to occur more than once per binary,
                        # and the linker will honk at you if more than one object
                        # defines globals with the same name.
                        # 
                        # If a global appears in more than one object, deconflict by
                        # naming each copy according to the containing instance.
                        # Since globals can only appear once per valid ELF binary,
                        # and weldr assumes the inputted projects are valid,
                        # this is sufficent.
                        self.defined_syms_to_objs.setdefault(name, set()).add(obj)
                        self.l.debug("\t\tGlobal symbol; tracking");

    def recover_symbols(self):
        for cmd in self.results.insts:
            for inst in self.results.insts[cmd]:
                self.l.debug("Recovering symbols for {:s}".format(inst))
                inst_dir = pathlib.Path(self.working_dir, inst)
                self.objdump_all(inst_dir)

    def deconflict_symbols(self):
        for symbol in self.defined_syms_to_objs.keys():
            self.l.debug("Considering symbol {:s}".format(symbol)) 
            is_blacklist = symbol.startswith("__cxa")
            is_main = symbol == "main"
            is_lib_init = symbol.startswith('fuse_init')
            is_lib_fini = symbol.startswith('fuse_fini')
            conflict = symbol in self.defined_syms_to_objs and len(self.defined_syms_to_objs[symbol]) > 1
            if is_blacklist:
                continue
            elif is_main or is_lib_init or is_lib_fini or conflict:
                self.l.debug("Deconflicting symbol {:s}".format(symbol))
                if self.check_cxx(symbol):
                    fmt = self.rename_cxx(symbol)
                else:
                    fmt = self.rename_normal(symbol)
                if fmt is None:
                    # We decided not to override the symbol.
                    continue
                self.l.debug("New symbol pattern: {:s}".format(fmt))

                for obj in self.defined_syms_to_objs.get(symbol, set()):
                    inst = obj.parent.name
                    if inst == 'lib':
                        inst = obj.parent.parent.name
                    new_symbol = fmt.format(len(inst), inst)
                    self.replace_symbol(obj, symbol, new_symbol, True)

                for obj in self.undefined_syms_to_objs.get(symbol, set()):
                    inst = obj.parent.name
                    if inst == 'lib':
                        inst = obj.parent.parent.name
                    new_symbol = fmt.format(len(inst), inst)
                    self.replace_symbol(obj, symbol, new_symbol, False)

    def wrap_dyn_libs(self):
        for (lib_path, syms) in self.results.bin_wrap_syms.items():
            # Normalize lib path.
            self.l.debug("Considering library {!s}".format(lib_path))
            root_name = lib_path.parts[0]
            if root_name in self.results.insts:
                self.l.error('\tLibrary {!s} is instance specific: {!s}'.format(lib_path, self.results.insts[root_name]))
                lib_path = lib_path.relative_to(root_name)
                lib_path = pathlib.Path(self.results.insts[root_name][0]) / lib_path
                self.l.error('\tRedirecting to {!s}'.format(lib_path))
            # Record the symbols in the torch command file.
            for sym in syms:
                self.replace_symbol(self.working_dir / lib_path, sym, "__wrap_{:s}".format(sym), False)
            # Record all wrapper models as library dependencies in the torch command file.
            self.add_lib_dependencies(lib_path)

        for (lib_path, syms) in self.results.bin_stub_syms.items():
            # Normalize lib path.
            self.l.debug("Considering library {!s}".format(lib_path))
            root_name = lib_path.parts[0]
            if root_name != 'lib' and root_name not in self.results.insts:
                self.l.debug("Library {!s} is part of a program we're not using.".format(lib_path))
                continue
                
            if root_name in self.results.insts:
                self.l.error('Library {!s} is instance specific: {!s}'.format(lib_path, self.results.insts[root_name]))
                lib_path = lib_path.relative_to(root_name)
                lib_path = pathlib.Path(self.results.insts[root_name][0]) / lib_path
                self.l.error('\tRedirecting to {!s}'.format(lib_path))
            # Record the symbols in the torch command file.
            for sym in syms:
                self.replace_symbol(self.working_dir / lib_path, sym, "__wrap_{:s}".format(sym), False)

    def run(self):
        
        self.l.info("Recovering symbols from object files.")
        self.recover_symbols()
        self.l.info("Renaming conflicting symbols.")
        self.deconflict_symbols()
        self.l.info("Renaming model overrides")
        self.wrap_dyn_libs()
        self.results.torch_paths = self.torch_paths

