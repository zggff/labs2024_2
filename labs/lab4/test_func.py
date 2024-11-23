"""
usage:
import sys
import os

sys.path.append(os.getcwd() + '/..')
"""

import subprocess
import sys
from typing import List, Optional

cnt = 0
test_leaks = True


def set_test_leaks(val: bool) -> None:
    global test_leaks
    test_leaks = val


def run_process(
    args: List[str],
    inp: str | List[str],
    prog_name: str = "./main.out",
    test_leaks: bool = False,
    capture=True,
) -> tuple[int, str]:
    if isinstance(inp, list):
        inp = "\n".join(inp)
    prog = [prog_name] + args
    if test_leaks:
        prog = ["leaks", "--atExit", "--list", "--"] + prog
    if not capture:
        p = subprocess.run(prog, text=True, input=inp)
        return (p.returncode, "")
    try:
        p = subprocess.run(
            prog,
            text=True,
            input=inp,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            encoding=sys.stdout.encoding,
        )
        return (p.returncode, p.stdout)
    except UnicodeDecodeError:
        return (-1, "error: non unicode output")


def test(
    test_name: str,
    args: List[str],
    status: int,
    inp: str | List[str],
    out: str | List[str],
    ofile: Optional[str] = None,
    ofile_val: Optional[str] | Optional[List[str]] = None,
    prog_name="./main.out",
) -> bool:
    global cnt
    cnt += 1
    print(f"TEST {cnt:02}: {test_name}", end="\n\t")
    prog = [prog_name] + args
    p = None
    if isinstance(inp, list):
        inp = "\n".join(inp)
    if isinstance(out, list):
        out = "\n".join(out)

    try:
        p = subprocess.run(
            prog,
            text=True,
            input=inp,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            encoding=sys.stdout.encoding,
        )
    except UnicodeDecodeError:
        print(f"FAILURE: non utf8 symbol in output")
        return False

    if p.returncode != status or p.stdout.splitlines() != out.splitlines():
        print(f"FAILURE:")
        if p.returncode != status:
            print(f"\treturn code: expected {status}, got {p.returncode}")
        cmp_string(p.stdout, out)
        return False

    if ofile is not None and ofile_val is not None:
        s = open(ofile).read()
        if isinstance(ofile_val, list):
            ofile_val = "\n".join(ofile_val)
        if s.splitlines() != ofile_val.splitlines():
            print(f"FAILURE: wrong file output")
            cmp_string(s, ofile_val)
            return False

    global test_leaks
    if test_leaks:
        test_leaks = ["leaks", "--atExit", "--list", "--"]
        prog = test_leaks + prog
        p = subprocess.run(
            prog,
            text=True,
            input=inp,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        if p.returncode != 0:
            print(f"FAILURE: memory leaks")
            print(f"\n[{p.stdout}\n]")
            return False

    print(f"SUCCESS")
    return True


def cmp_string(s: str, s_exp: str):
    txt = s.splitlines()
    exp = s_exp.splitlines()
    mlen = max(len(txt), len(exp))
    txt += [None] * (mlen - len(txt))
    exp += [None] * (mlen - len(exp))
    for i, (a, b) in enumerate(zip(txt, exp)):
        if a != b:
            print(f"\tdifference in line {i + 1}: [{repr(a)}] < [{repr(b)}]")
            return
