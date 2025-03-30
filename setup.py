from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import subprocess


class CmakeBuildExt(build_ext):
    def run(self):
        subprocess.check_call(
            [
                "cmake",
                "-G",
                "Ninja",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_C_COMPILER=clang-cl",
                "-DCMAKE_CXX_COMPILER=clang-cl",
                "-DCMAKE_LINKER=lld-link",
                "-B",
                "build",
            ]
        )
        subprocess.check_call(["ninja", "-C", "build"])


setup(
    name="spam",
    version="0.0.1",
    cmdclass={"build_ext": CmakeBuildExt},
    ext_modules=[Extension("spam", sources=[]), Extension("custom", sources=[])],
)
