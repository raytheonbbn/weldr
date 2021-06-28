# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
class Results:
    # Global to detect infinite recursion.
    # If a 'get' is in operation, 
    prev_name=None
    vals_name='_vals'
    def __init__(self):
        if Results.vals_name not in dir(self):
            # Avoid overwriting _vals if we've already set it.
            print('Assigning _vals in the initializer of {!s}'.format(id(self)))
            self._vals = dict()
            print('Initializer assignment complete')

    def __getattr__(self, name):
        if name == Results.vals_name and name not in dir(self):
            raise AttributeError('Missing value dict')

        if name == Results.prev_name:
            # We detected infinite recursion.  Raise an exception.
            Results.prev_name = None
            raise AttributeError("Recursion on name '{:s}'".format(name))
        
        # Start tracking recursion.        
        Results.prev_name = name

        if name not in self._vals:
            # No field of the name given.  Reset recursion tracking and raise.
            Results.prev_name = None
            raise AttributeError("Unknown result field: {:s}".format(name))
        
        # We have a field matching name.
        # Get it first, _then_ reset recursion tracking,
        # just in case the 'get' goes wrong.
        out = self._vals[name]
        Results.prev_name = None
        return out

    def __setattr__(self, name, val): 
        if name == Results.vals_name and name not in dir(self):
            # Pass-through case: we need to be able to assign '_vals'
            super().__setattr__(name, val)
        elif name in dir(self):
            # Don't allow overrides of existing object fields.
            raise AttributeError("Assign to an illegal attribute{:s}".format(name))
        else:
            # Anything else goes in the dict.
            self._vals[name] = val

    def __delattr__(self, name, val):
        if name in dir(self):
            # Don't allow deleting object fields.
            raise AttributeError("Delete of an illegal attribute: {:s}".format(name))
        if name not in self._vals:
            # If it's not in the dict, we can't delete it.
            raise AttributeError("Unknown result field: {:s}".format(name))
        # Otherwise, delete it.
        del self._vals[name]

