# from setuptools import Extension, setup

# setup(
#         ext_modules=[
#             Extension(
#                 name="spam",
#                 sources=["simple.c"],
#                 library_dirs=[".venv/Scripts", ".venv/Lib/site-package"],
#                 include_dirs=[".venv/Include"],
#                 extra_compile_args=["/Zi", "/Od"],
#                 extra_link_args=["/DEBUG"]
#             )
#         ]
# )

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import subprocess


class CmakeBuildExt(build_ext):
    def run(self):
        subprocess.check_call(
            ["cmake", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=RELEASE", "-B", "build"]
        )
        subprocess.check_call(["ninja", "-C", "build"])


setup(
    name="spam",
    version="0.0.1",
    cmdclass={"build_ext": CmakeBuildExt},
    ext_modules=[Extension("spam", sources=[])],
)
