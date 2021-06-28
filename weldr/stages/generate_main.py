# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
import pathlib
from ..argdef import ArgDef
from ..stage import Stage

class MainGeneratorArgs(ArgDef):
    def add_args(self, parser):
        parser.add_argument('--use-posix-path-expansion', action='store_const', default='', 
                const='#define USE_POSIX_PATH_EXPANSION',
                help='Use helpful but difficult-to-model libraries to parse the basedir path.')


class MainGeneratorStage(Stage):
    def __init__(self, args):
        super().__init__(args)

    @property
    def name(self):
        return "generate_main"

    @property
    def predicessor(self):
        return frozenset(["resolve_syms"])

    def copy_static_sources(self):
        srcdir = pathlib.Path(self.args.model_dir, 'src')
        for src in srcdir.glob('*.c'):
            self.l.debug('Copying static source {!s} into project.'.format(src))
            self.cp(src, self.working_dir)

    def generate_main_header(self):
        with open(pathlib.Path(self.working_dir, "main.h"), "w") as header:
            header_txt = [
                '#ifndef WELDR_MAIN_H',
                '#define WELDR_MAIN_H',
                '/*',
                ' * Automatically-generated main header from weldr',
                ' */',
                '#include <pthread.h>',
                '#include <stdlib.h>',
                '#include <stdio.h>', 
                '#include <string.h>',
                '#include <sys/stat.h>',
                '#include <unistd.h>',
                '',
                '// If the following line is not blank, it activates',
                '// posix path expansion.',
                '{:s}'.format(self.args.use_posix_path_expansion),
                '',
                '// Extra import for posix path expansion.',
                '#ifdef USE_POSIX_PATH_EXPANSION',
                '#include <wordexp.h>',
                '#endif',
                '',
                '// Signature for argument parser',
                'int parse_args(int argc, char** argv, int num_instances, const char **basedir, const char const **instance_names, const char ***instance_args, const char const ***default_instance_args);',
                '',
                '// Signatures for stub library initializers'
            ]
            for cmd in filter(lambda x: x in self.results.insts, self.results.cmd_stub_models):
                for inst in self.results.insts[cmd]:
                    for model in map(lambda p: p.name, self.results.cmd_stub_models[cmd]):
                        self.l.debug("Define init of {:s} for {:s}".format(model, inst))
                        header_txt.append('int {:s}_weldr_init_stub_{:s}(const char* basedir, const char* instance);'.format(inst, model))

            header_txt.append('')
            header_txt.append('// Signatures for wrapper library initializers')
            for model in map(lambda p: p.name, self.results.global_wrapper_models):
                self.l.debug("Define init of {:s}".format(model))
                header_txt.append('int weldr_init_wrap_{:s}();'.format(model))

            header_txt.append('')
            header_txt.append('// Signatures for instance main functions')
            for cmd, insts in self.results.insts.items():
                for inst in insts:
                    self.l.debug("Define main for {:s}".format(inst))
                    header_txt.append('int {:s}_main(int argc, char* argv[]);'.format(inst))
            header_txt.append('')
            header_txt.append('#endif//WELDR_MAIN_H')
            header_txt = '\n'.join(header_txt)
            print(header_txt, file=header)

    
    def generate_main_source(self):
        with open(pathlib.Path(self.working_dir, "main.c"), "w") as source:
            source_txt = [
                '/*',
                ' * Automatically-generated main file from weldr',
                ' */',
                '#include "main.h"',
                '#define NUM_INSTANCES {:d}'.format(len(self.results.inst_args.keys())),
                '',
            ]
            inst_names_txt = []
            inst_argv_vars = []

            inst_idx = 0
            for inst, args in self.results.inst_args.items():
                
                source_txt.append('// Default configuration for instance {:s}'.format(inst))

                # Assign instance name
                inst_names_txt.append('"{:s}"'.format(inst))

                # Assign default argc from sytem definition
                argc_name = '{:s}_default_argc'.format(inst)
                source_txt.append('int {:s} = {:d};'.format(argc_name, len(args)))

                # Assign default argv from system definition
                args_txt = ['(const char *)&{:s}'.format(argc_name)]
                # Stringify all members of argv, and add them to the back of args_txt.
                args_txt.extend(map(lambda x: '"{:s}"'.format(x), args))
                

                # Write the default argv as a scalar variable.
                var_name = '{:s}_default_argv'.format(inst)
                source_txt.append('const char *{:s}[] = {{{:s}}};'
                        .format(var_name, ', '.join(args_txt)))
                # Save off the scalar variable so we can add it to DEFAULT_INSTANCE_ARGS.
                # Nested scalar arrays are a pain, so we need to initialize things separately.
                inst_argv_vars.append(var_name)
                source_txt.append('')

            source_txt.append('// Global configuration for subprograms.')
            source_txt.append('const char *INSTANCE_NAMES[] = {{{:s}}};'
                    .format(', '.join(inst_names_txt)))
            source_txt.append('const char **DEFAULT_INSTANCE_ARGS[] = {{{:s}}};'
                    .format(', '.join(inst_argv_vars)))
            source_txt.append('const char **INSTANCE_ARGS[NUM_INSTANCES];')
            source_txt.append('const char *basedir;')
            source_txt.append('')


            #Set up the thread entry method foreach instance.
            for inst in self.results.inst_args.keys():
                self.l.debug("Defining run function for {:s}".format(inst))
                run_txt = [
                    '',
                    '// Thread entry method for {:s}'.format(inst),
                    'void *run_{:s}(void *args) {{'.format(inst),
                    '    // Init the argv array',
                    '    int out;',
                    '    if(args != NULL) {',
                    '        // If we actually have args from the cmd line,',
                    '        // populate them.',
                    '        int argc = *(((int**)args)[0]);',
                    '        char *argv[argc + 1];',
                    '        argv[0] = "{:s}";'.format(inst),
                    '        for(int i = 1; i <= argc; i++) {',
                    '            argv[i] = ((char**)args)[i];',
                    '        }',
                    '        out = {:s}_main(argc+1, argv);'.format(inst),
                    '    } else {',
                    '        // If we did not receive args,',
                    '        // just fill in the command name.',
                    '        char *argv[1];',
                    '        argv[0] = "{:s}";'.format(inst),
                    '        out = {:s}_main(1, argv);'.format(inst),
                    '    }'
                ]
                if self.results.primary_inst is not None and  inst == self.results.primary_inst:
                    run_txt_2 = [
                        '   // Primary instance finished;',
                        '   // Terminate the program.',
                        '   exit(out);',
                        '}'
                    ]
                else:
                    run_txt_2 = [
                        '   // Return the return code',
                        '   return (void*)(long)out;',
                        '}'
                    ]
                source_txt = source_txt + run_txt + run_txt_2

            #Set up the main method.
            main_txt = [
                '',
                'int main(int argc, char *argv[]) {',
                '    // Init instance data'
            ]


            source_txt = source_txt + main_txt

            main_txt_2 = [
                '    // Parse arguments',
                '    printf("Parsing Arguments\\n");',
                '    parse_args(argc, argv,',
                '        NUM_INSTANCES,',
                '        &basedir,',
                '        INSTANCE_NAMES,', 
                '        INSTANCE_ARGS,',
                '        DEFAULT_INSTANCE_ARGS);',
                '',
                '    // Init stub libraries',
                '    printf("Starting stub libraries\\n");',
                '    int ret = 0;',
            ]

            source_txt = source_txt + main_txt_2

            for cmd in filter(lambda x: x in self.results.insts, self.results.cmd_stub_models):
                for inst in self.results.insts[cmd]:
                    for stub in map(lambda p: p.name, self.results.cmd_stub_orderings[cmd]):
                        self.l.debug('Calling init of {:s} for {:s}'.format(stub, inst))
                        stub_init_txt = [
                            '',
                            '    // Call the initializer of {:s} for {:s}'.format(stub, inst),
                            '    if(ret = {:s}_weldr_init_stub_{:s}(basedir, "{:s}")) {{'.format(inst, stub, inst),
                            '        fprintf(stderr, "Failed to init stub {:s} for {:s}(%d)\\n", ret);'.format(stub, inst),
                            '        exit(ret);',
                            '    }'
                        ]
                        source_txt = source_txt + stub_init_txt

            main_txt_3 = [
                '    // Init wrapper libraries',
                '    printf("Starting wrapper libraries\\n");'
            ]

            source_txt = source_txt + main_txt_3

            for model in map(lambda p: p.name, self.results.global_wrapper_ordering):
                self.l.debug('Calling init of {:s}'.format(model))
                wrap_init_txt = [
                    '',
                    '    // Call the initializer of {:s}'.format(model),
                    '    if(ret = weldr_init_wrap_{:s}()){{'.format(model),
                    '        fprintf(stderr, "Failed to init wrapper {:s}\\n");'.format(model),
                    '        exit(ret);',
                    '    }'
                ]
                
                source_txt = source_txt + wrap_init_txt

            main_txt_4 = [
                '    // Launch each program instance in a separate thread.',
                '    printf("Launching subprograms\\n");'
            ]

            source_txt = source_txt + main_txt_4

            i = 0
            for inst in self.results.inst_args:
                launch_txt = [
                    '',
                    '    // Launching {:s}'.format(inst),
                    '    pthread_t {:s}_thread;'.format(inst),
                    '    if(pthread_create(&{0:s}_thread, NULL, run_{0:s}, INSTANCE_ARGS[{1:d}])) {{'.format(inst, i),
                    '        fprintf(stderr, "Failed to launch thread for {:s}\\n");'.format(inst),
                    '        exit(1);',
                    '    }'
                ]

                source_txt = source_txt + launch_txt
                i += 1
            
            main_txt_5 = [
                '',
                '    // Join on all threads',
                '    printf("Joining all threads\\n");'
            ]

            source_txt = source_txt + main_txt_5

            for inst in self.results.inst_args:
                join_txt = [
                    '',
                    '    // Joining {:s}'.format(inst),
                    '    if(pthread_join({:s}_thread, NULL)) {{'.format(inst),
                    '        fprintf(stderr, "Failed to join thread for {:s}\\n");'.format(inst),
                    '        exit(1);',
                    '    }'
                ]

                source_txt = source_txt + join_txt


            source_txt.append('    exit(0);')
            source_txt.append('}')
            source_txt = '\n'.join(source_txt)
            print(source_txt, file=source)





    def run(self):
        self.l.info('Copying static sources...')
        self.copy_static_sources()
        self.l.info("Generating header file for driver code (prevents linker warnings.)")
        self.generate_main_header()
        self.l.info("Generating driver code.")
        self.generate_main_source()
