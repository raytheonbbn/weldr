# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
import logging
import pickle
import subprocess
import sys
from collections import Counter
from functools import reduce
from importlib import import_module
from pathlib import Path
from pkgutil import iter_modules
from .stage import Stage
from .results import Results

class Driver:
    def __init__(self, args):
        self.args = args
        self.stages = []
        self.results = Results()
        self.working_dir = None

        # Configure logging level
        if self.args.very_verbose:
            level= "TRACE"
        elif self.args.verbose:
            level = "DEBUG"
        else:
            level = "INFO"

        # Add the "TRACE" logging level
        self.addLoggingLevel("TRACE", logging.DEBUG - 5)
        self.configureRootLogger(level)

        self.l = logging.getLogger("weldr.driver")
        self.l.trace("Trace logging enabled!")

    def addLoggingLevel(self, levelName, levelNum, methodName=None):
        if not methodName:
            methodName = levelName.lower()
        if hasattr(logging, levelName):
            raise AttributeError("Level {:s} already defined for logging.".format(levelName))

        if hasattr(logging, methodName):
            raise AttributeError("Method {:s} already defined for logging.".format(methodName))

        if hasattr(logging.getLoggerClass(), methodName):
            raise AttributeError("Method {:s} already defined for logger class.".format(methodName))

        def logForLevel(self, message, *args, **kwargs):
            if self.isEnabledFor(levelNum):
                self._log(levelNum, message, args, **kwargs)

        def logToRoot(message, *args, **kwargs):
            logging._log(levelNum, message, *args, **kwargs)

        logging.addLevelName(levelNum, levelName)
        setattr(logging, levelName, levelNum)
        setattr(logging.getLoggerClass(), methodName, logForLevel)
        setattr(logging, methodName, logToRoot)

        if not hasattr(logging, methodName):
            raise Exception("Logging didn't take the new method")
        if not hasattr(logging.getLoggerClass(), methodName):
            raise Exception("Logger class didn't take the new method.")

    def configureRootLogger(self, level):
        root_logger = logging.getLogger('weldr')
        root_logger.setLevel("DEBUG")

        formatter = logging.Formatter('%(levelname)s [%(name)s]: %(message)s')

        out_handler = logging.StreamHandler(stream=sys.stdout)
        out_handler.setFormatter(formatter)
        out_handler.setLevel(level)
        root_logger.addHandler(out_handler)

        if self.args.log_file is not None:
            file_handler = logging.FileHandler(self.args.log_file, mode='w')
            file_handler.setFormatter(formatter)
            file_handler.setLevel("DEBUG")
            root_logger.addHandler(file_handler)


    def load_stages(self):
        #Load all modules in .stages
        for x in iter_modules([Path(__file__).parent / 'stages']):
            module = import_module('.stages.' + x.name, package='weldr')
        #This will dynamically register the Stage subclasses with their parent classes.
        for x in Stage.__subclasses__():
            self.stages.append(x(self.args))

        #Filter out only stages valid given the arguments provided.
        #Make sure the plugin authors didn't allow for a namespace collision.
        self.stages = list(filter(lambda x: x.valid, self.stages))
        tags = set(map(lambda x: x.tag, self.stages))
        if len(tags) < len(self.stages):
            counter = Counter(map(lambda x: x.tag, self.stages))
            dups = [ k for k, v in counter.items() if v > 1 ]
            self.l.error("Name collision between valid stages: {!s}".format(dups))
            raise Exception("Name collision")

        #Check that all required predicessors are present.
        preds = set(reduce(lambda x, y: x | y.predicessor, self.stages, set()))
        if len(preds - tags) > 0:
            self.l.error("Missing required stages: {!s}".format(preds - tag))
            raise Exception("Missing required stages")

        #Compute a total ordering for the stages, based on each stage's predicessors.
        new_stages = []
        seen = set()
        while len(tags - seen) != 0:
            for stage in self.stages:
                if stage.tag not in seen and len(stage.predicessor & seen) == len(stage.predicessor):
                    new_stages.append(stage)
                    seen.add(stage.tag)

        self.stages = new_stages

        # If we specified a later starting stage, figure out where it is.
        if self.args.start_stage is not None:
            good = False
            for i in range(0, len(self.stages)):
                if self.stages[i].tag == self.args.start_stage:
                    self.working_dir = self.stages[i - 1].working_dir
                    if not self.working_dir.exists():
                        self.l.error("Missing an existing working dir for {:s}: {!s}".format(self.stages[i - 1].tag, self.working_dir))
                        raise Exception("Existing working dir not found")

                    results_file = self.working_dir / 'results.pickle'
                    if not results_file.exists():
                        self.l.error("Missing an existing results file for {:s}: {!s}".format(self.stages[i - 1].tag, results_file))
                        raise Exception("Results file not found")
                    with open(results_file, 'rb') as f:
                        self.results = pickle.load(f)

                    self.stages = self.stages[i:]
                    good = True
                    break
            if not good:
                self.l.error("No stage named {:s}".format(self.args.start_stage))
                raise Exception("Invalid start stage")

        # If we specified an earlier stopping stage, figure out where it is.
        if self.args.stop_stage is not None:
            good = False
            for i in range(0, len(self.stages)):
                if self.stages[i].tag == self.args.stop_stage:
                    self.stages = self.stages[:i+1]
                    good = True
                    break
            if not good:
                self.l.error("No stage named {:s}".format(self.args.stop_stage))
                raise Exception("Invalid stop stage")

    def update_working_dir(self, stage):
        if self.working_dir is None:
            self.working_dir = stage.working_dir
            if self.working_dir.exists():
                stage.exec("rm", "-rf", stage.working_dir)
            self.working_dir.mkdir(parents=True)
        elif self.args.safe_mode:
            if stage.working_dir.exists() and stage.preserve_old_working_dir:
                self.l.debug("Keeping working dir from old run.")
            else:
                self.l.debug("Copying working dir from {!s} to {!s}".format(self.working_dir, stage.working_dir))
                stage.exec("rm", "-rf", stage.working_dir)
                stage.cp(self.working_dir, stage.working_dir)
            self.working_dir = stage.working_dir

    def run(self):
        self.load_stages()
        for stage in self.stages:
            self.update_working_dir(stage)
            try:
                stage.start(self.results)
            except Exception as e:
                self.l.exception("Exception while running {:s}".format(stage.name))
                return 1
            if stage.terminal:
                break
        self.l.info("SUCCESS!")
        return 0
        
