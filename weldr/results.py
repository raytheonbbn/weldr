# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
class Results:
    def __init__(self):
        self.__vals = dict()

    def __getattr__(self, name):
        if name not in self.__vals:
            raise AttributeError("Unknown result field: {:s}".format(name))
        return self.__vals[name]

    def __setattr__(self, name, val):
        # TODO: I'm not real sure about this feature.
        # I'm not sure why this is using a different name...
        if '__vals' not in dir(self) and name == '_Results__vals':
            super().__setattr__(name, val)
        elif name in dir(self):
            raise AttributeError("Assign to an illegal attribute{:s}".format(name))
        else:
            self.__vals[name] = val

    def __delattr__(self, name, val):
        if name in dir(self):
            raise AttributeError("Delete of an illegal attribute: {:s}".format(name))
        if name not in self.__vals:
            raise AttributeError("Unknown result field: {:s}".format(name))
        del self.__vals[name]

