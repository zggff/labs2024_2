import os
import sys
import tempfile

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test, run_process

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
fp.writelines([b"read"])
fp.seek(0)
assert test(
    "parse not enough tokens",
    [fp.name],
    2,
    "",
    ["ERROR: not enough tokens:", "\t{[read]}"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read(a, b, c);"])
fp.seek(0)
assert test(
    "parse too many tokens",
    [fp.name],
    2,
    "",
    ["ERROR: too many tokens:", "\t{[read] [(] [a] [,] [b] [,] [c] [)] [;]}"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read a, 12);"])
fp.seek(0)
assert test(
    "parse invalid format",
    [fp.name],
    2,
    "",
    ["ERROR: expected [(] got [a]"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read(a, 39);"])
fp.seek(0)
assert test(
    "parse invalid base",
    [fp.name],
    2,
    "",
    ["ERROR: base [39] not in range [2..36]"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read(a, 16);"])
fp.seek(0)
assert test(
    "read invalid",
    [fp.name],
    2,
    "fz",
    ["ERROR: failed to parse [fz] as number in base [16]"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read(a, 16);", b"write(a, 10);"])
fp.seek(0)
assert test(
    "read and write valid",
    [fp.name],
    0,
    "ff",
    ["A_10 = 255"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"read(a, 36);", b"b:=\\a;", b"write(a,2);", b"write(b, 2);"])
fp.seek(0)
assert test(
    "check bit negation",
    [fp.name],
    0,
    "z",
    [
        "A_2 = 100011",
        "B_2 = 11011100",
    ],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"read(a, 2);",
        b"read(b, 2);",
        b"c := a + b;",
        b"write(c, 2);",
        b"c := a & b;",
        b"write(c, 2);",
        b"c := a -> b;",
        b"write(c, 2);",
        b"c := b <- a;",
        b"write(c, 2);",
        b"c := a ~ b;",
        b"write(c, 2);",
        b"c := a <> b;",
        b"write(c, 2);",
        b"c := a +> b;",
        b"write(c, 2);",
        b"c := a ? b;",
        b"write(c, 2);",
        b"c := a ! b;",
        b"write(c, 2);",
        b"c := \\a;",
        b"write(c, 2);",
    ]
)
fp.seek(0)
assert test(
    "check all ops",
    [fp.name],
    0,
    ["1101", "101010"],
    [
        "C_2 = 101111",
        "C_2 = 1000",
        "C_2 = 11111010",
        "C_2 = 11111010",
        "C_2 = 11011000",
        "C_2 = 100111",
        "C_2 = 100010",
        "C_2 = 11110111",
        "C_2 = 11010000",
        "C_2 = 11110010",
    ],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
op = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"read(a, 2);",
        b"read(b, 2);",
        b"c := a + b;",
        b"c := \\a;",
    ]
)
fp.seek(0)
assert test(
    "check tracing",
    [fp.name, "/trace", op.name],
    0,
    ["1101", "101010"],
    [],
    op.name,
    [
        "read(A, 2);",
        "\tA = 00000000",
        "\t->",
        "\tA = 00001101",
        "read(B, 2);",
        "\tB = 00000000",
        "\t->",
        "\tB = 00101010",
        "C := A + B;",
        "\tC = 00000000",
        "\tA = 00001101",
        "\tB = 00101010",
        "\t->",
        "\tC = 00101111",
        "C := \\A;",
        "\tC = 00101111",
        "\tA = 00001101",
        "\t->",
        "\tC = 11110010",
    ],
)
fp.close()
op.close()
