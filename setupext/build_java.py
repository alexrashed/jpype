# -*- coding: utf-8 -*-
import os
import sys
import subprocess
import distutils.cmd
import distutils.log
from distutils.errors import DistutilsPlatformError
from distutils.dir_util import copy_tree
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


def compileJava(self):
    target_version = "1.7"
    version = getJavaVersion()
    srcs = glob.glob('native/java/**/*.java', recursive=True)
    src1 = [ i for i in srcs if "JPypeClassLoader" in i]
    src2 = [ i for i in srcs if not "JPypeClassLoader" in i]
    cmd1 = shlex.split('javac -d build/lib -g:none -source %s -target %s'%(target_version, target_version))
    cmd1.extend(src1)
    cmd2 = shlex.split('javac -d build/classes -g:none -source %s -target %s -cp build/lib'%(target_version, target_version))
    cmd2.extend(src2)
    self.announce("  %s" % " ".join(cmd1), level=distutils.log.INFO)
    subprocess.check_call(cmd1)
    self.announce("  %s" % " ".join(cmd2), level=distutils.log.INFO)
    subprocess.check_call(cmd2)
    cmd3 = shlex.split('jar --create --file build/lib/org.jpype.jar -C build/classes/ .')
    self.announce("  %s" % " ".join(cmd3), level=distutils.log.INFO)
    subprocess.check_call(cmd3)


class BuildJavaCommand(distutils.cmd.Command):
    """A custom command to create jar file during build."""

    description = 'build jpype jar'
    user_options = []

    def initialize_options(self):
        """Set default values for options."""
        pass

    def finalize_options(self):
        """Post-process options."""
        pass

    def run(self):
        """Run command."""
        java = self.distribution.enable_build_jar

        # Try to use the cach if we are not requested build
        if not java:
            src = os.path.join('native', 'jars')
            dest = os.path.join('build', 'lib')
            if os.path.exists(src):
                distutils.log.info("Using Jar cache")
                copy_tree(src, dest)
                return

        distutils.log.info(
            "Jar cache is missing, using --enable-build-jar to recreate it.")

        # build the jar
        try:
            compileJava(self)
        except subprocess.CalledProcessError as exc:
            distutils.log.error(exc.output)
            raise DistutilsPlatformError("Error executing {}".format(exc.cmd))

        ## Disable for now.  Java coverage tool needs work
        # Coverage tool requires special placement of the source
        #if self.distribution.enable_coverage:
        #    import shutil
        #    shutil.copyfile(os.path.join("build", "lib", "org.jpype.jar"), os.path.join(
        #        "native", "org.jpype.jar"))
