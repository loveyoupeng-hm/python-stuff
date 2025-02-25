import ast
import inspect
import textwrap
from typing import Any, Callable
import spam


cache: dict[ast.FunctionDef, ast.FunctionDef] = {}
context: dict[str, Any] = {}


def replace(dist: Callable[[int, int], int]):
    def do_replace(src: Callable[[int, int], int]):
        global cache
        src_source = textwrap.dedent(inspect.getsource(src))
        dist_source = textwrap.dedent(inspect.getsource(dist))
        src_ast = ast.parse(src_source)
        dist_ast = ast.parse(dist_source)
        cache[src_ast.body[-1]] = dist_ast.body[0]  # type:ignore[assignment, index]
        global context
        context[dist.__name__] = dist
        return src

    return do_replace


def process(func: Callable[[int, int], int]) -> Callable[[int, int], int]:
    """process"""
    source = textwrap.dedent(inspect.getsource(func))
    prefix = (
        "\n".join(["# nothing " for _ in range(func.__code__.co_firstlineno)]) + "\n"
    )
    source = "\n".join(
        [line for line in source.split("\n") if not line.strip().startswith("@process")]
    )
    body = ast.parse(prefix + source)
    func_def = body.body[0]
    add_func = func_def.body[1].body[0].value.func
    name = add_func.id
    global cache
    for src, dist in cache.items():
        if src.name == name:
            add_func.id = dist.name
            ast.fix_missing_locations(body)
            result = compile(body, inspect.getfile(func), mode="exec")
            global context
            names = dict[str, Any](context)
            exec(result, names)
            result = names[func.__name__]
            return result
    return func


def addm(a: int, b: int) -> int:
    """add modifyed"""
    print(f"{a} + {b} = {a+b}")
    return a + b


@replace(dist=addm)
def add(a: int, b: int) -> int:
    """test add"""
    return a + b


@process
def mul(a: int, b: int) -> int:
    """mul"""
    if a == 0:
        return add(a, b)
    else:
        return a - b


print(spam.__file__)

print(spam.system())


print(mul(0, 3))
print(mul(4, 4))
