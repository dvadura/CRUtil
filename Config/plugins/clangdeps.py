# --------------------------------------------------------------------------------------------------
# Sample plugin.
#
# To make a plugin, import PluginBase, and create a class that implements eval as below.
# Because Bif is compiled with nuitka, by default, you can import any python builtin package
# and any of the packages listed in the bootstrap.py file in Bif Source tree.
#
# Make sure that you check the Source for your version to see the latest list of included
# packages.
#
# The following non-builtin packages are guaranteed to be included:
#
#   - typingextensions
#   - pytest
#   - argparse
#   - pathlib
#   - pyjson5
#   - pyaml
#   - wcmatch
#   - nuitka
#   - lark
#   - rich
#
# --------------------------------------------------------------------------------------------------
from plugins import PluginBase
import pathlib
import re

# This plugin reads the CLang generated dependencies file and and returns the list of dependencies
# for the specified target.
class ClangDeps(PluginBase):
    SPLIT_PATTERN = re.compile(r'[:\s]+')

    def __init__(self):
        pass

    # expects to be called with loc=pathlib.Path file=filename target=<targetid> [log=<DLog>].
    # needs to read filename @ loc, find targetid and return a list of strings representing dependencies
    def eval(self, *args, **kwargs):
        target = kwargs.get('target')
        loc = kwargs.get('loc', '.')
        file = kwargs.get('file')

        # all three must be supplied.
        if not (target and loc and file):
            return []

        f = open(pathlib.Path(loc) / file, "r")

        # for CLANG the deps file contains something similar to:
        #
        #    target.o: target.c target.h ....
        #
        # Note that some of the paths for the deps may be absolute.
        try:
            for line in f:
                names = re.split(ClangDeps.SPLIT_PATTERN, line.strip())
                if names[0] == target:
                    return names[1:]
        finally:
            f.close()

        return []
