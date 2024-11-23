import os
import sys
import tempfile

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test

set_test_leaks(False)

assert test("no input file provided", [], 1, "", "ERROR: input file not provided")

assert test(
    "no input file provided",
    ["not existing"],
    1,
    "",
    "ERROR: failed to open file: [not existing]",
)

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"print a;"])
fp.seek(0)
assert test(
    "not initialized variable",
    [fp.name],
    1,
    "",
    ["ERROR: variable [a] is not initialized"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"a = 1 / 0;"])
fp.seek(0)
assert test(
    "divide by zero 1",
    [fp.name],
    1,
    "",
    ["ERROR: can not divide by zero"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"a = 1 % 0;"])
fp.seek(0)
assert test(
    "divide by zero 2",
    [fp.name],
    1,
    "",
    ["ERROR: can not divide by zero"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"a = 10;",
        b"print a;",
        b"b = a + 12;",
        b"b = b * 5;",
        b"print b;",
        b"c = b - 5;",
        b"d = b / 4;",
        b"e = b % 7;",
        b"a = e;",
        b"print;"
    ]
)
fp.seek(0)
assert test(
    "correct work",
    [fp.name],
    0,
    "",
    [
        "[a] = 10",
        "",
        "[b] = 110",
        "",
        "[a] = 5",
        "[b] = 110",
        "[c] = 105",
        "[d] = 27",
        "[e] = 5",
        "\n"
    ],
)
fp.close()
