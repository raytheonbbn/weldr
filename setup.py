# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
from setuptools import setup, find_packages
setup(
    name="weldr",
    version="0.1.0",
    install_requires=[],
    packages=find_packages(),
    entry_points={
        'console_scripts': [
            'weldr = weldr:main'
        ]
    }
)
