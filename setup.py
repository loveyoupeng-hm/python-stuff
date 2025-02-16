from setuptools import Extension, setup

setup(
        ext_modules=[
            Extension(
                name="spam",
                sources=["simple.c"],
                library_dirs=[".venv/Scripts", ".venv/Lib/site-package"],
                include_dirs=[".venv/Include"],
                extra_compile_args=["/Zi", "/Od"],
                extra_link_args=["/DEBUG"]
            )
        ]
)
