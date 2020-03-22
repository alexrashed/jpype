# -*- coding: utf-8 -*-
import sys
import os
import subprocess
import distutils.cmd
import distutils.log
import glob
import re
import shlex

def getJavaVersion():
    # Find Java version
    out = subprocess.run(['javac', '-version'], capture_output=True)
    version_str = out.stdout.decode('utf-8')
    result = re.match(r'javac ([0-9]+)\.([0-9]+)\..*', version_str)
    if int(result[1]) > 1:
        return int(result[1])
    return int(result[2])


def compileJava():
    version = getJavaVersion()
    srcs = glob.glob('test/harness/jpype/**/*.java', recursive=True)
    exports = ""
    if version > 7:
        srcs.extend(glob.glob('test/harness/java8/**/*.java', recursive=True))
    if version > 8:
        srcs.extend(glob.glob('test/harness/java9/**/*.java', recursive=True))
        exports = "--add-exports java.base/jdk.internal.reflect=ALL-UNNAMED"
    cmd = shlex.split(
        'javac -d test/classes %s -g:lines,vars,source' % (exports))
    cmd.extend(srcs)
    return cmd


class TestJavaCommand(distutils.cmd.Command):
    """A custom command to create jar file during test."""

    description = 'run javac to make test harness'
    user_options = []

    def initialize_options(self):
        """Set default values for options."""
        pass

    def finalize_options(self):
        """Post-process options."""
        pass

    def run(self):
        """Run command."""
        if os.path.exists(os.path.join("test", "classes")):
            distutils.log.info("Skip building Java testbench")
            return
        cmdStr = compileJava()
        self.announce("  %s" % " ".join(cmdStr), level=distutils.log.INFO)
        subprocess.check_call(cmdStr)
