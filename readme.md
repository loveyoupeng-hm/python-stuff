build 
```bash
python setup.py build_ext
```


stubgen
```bash
set MYPYPATH=./build
stubgen --package spam -m spam -o build
stubgen --package custom -m custom -o build
```