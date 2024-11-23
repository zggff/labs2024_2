import os
import sys

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test

set_test_leaks(False)

assert test(
    "check invalid command",
    [],
    0,
    "test",
    [
        "ERROR: unknown operation [test]",
        "supported operations: {load, save, rand, concat, free, remove, copy, sort, shuffle, stats, print, }",
    ],
)

assert test(
    "check invalid command format",
    [],
    0,
    "free ",
    "ERROR: braces are expected as separator with free command",
)

assert test(
    "check invalid command format",
    [],
    0,
    "print(",
    "ERROR: commands must be followed by whitespace",
)

assert test(
    "check command arg parsing",
    [],
    0,
    ["Load 0, 1;", "Load a", "Load a;", "Load a,"],
    [
        "ERROR: failed to parse arguments from line [Load 0, 1;]",
        "ERROR: failed to parse arguments from line [Load a]",
        "ERROR: failed to parse arguments from line [Load a;]",
        "ERROR: failed to parse arguments from line [Load a,]",
    ],
)

assert test(
    "check not being able to open file",
    [],
    0,
    ["Load a, nums_non_existing.txt;", "print a, all;"],
    ["ERROR: failed to open file: [nums_non_existing.txt]", "A = []"],
)


assert test(
    "check incorrect file reading",
    [],
    0,
    ["Load a, nums_error.txt;", "print a, all;"],
    ["ERROR: failed to parse [1a] as number", "A = []"],
)

assert test(
    "check correct file reading",
    [],
    0,
    ["Load a, nums.txt;", "print a, all;"],
    "A = [1, 22, 14, 1, 24, 12, 41, 1, 22, 14, 1, 24, 12, 41]",
)

assert test(
    "check correct file writing",
    [],
    0,
    ["Load a, nums.txt;", "save a, nums_out.txt;"],
    "",
    "./nums_out.txt",
    "1 22 14 1 24 12 41 1 22 14 1 24 12 41 ",
)


assert test(
    "check correct working of the program",
    [],
    0,
    [
        "Load a, nums_2.txt;",
        "print A, all;",
        "print A, 5;",
        "print A, 2, 4;",
        "copy a, 2, 7, b;",
        "print b, all;",
        "remove a, 1, 3;",
        "print a, all;",
        "concat a, b;",
        "print a, all;",
        "sort a+;",
        "print a, all;",
        "sort a-;",
        "stats a;",
        "save a, nums_out.txt;",
    ],
    [
        "A = [21, 214, 1, 4512, 51, 14, 99, 0, 12, 41, 45, 41]",
        "A[5] = 14",
        "A[2..4] = [1, 4512, 51]",
        "B = [1, 4512, 51, 14, 99, 0]",
        "A = [21, 51, 14, 99, 51, 14, 99, 0, 12]",
        "A = [21, 51, 14, 99, 51, 14, 99, 0, 12, 1, 4512, 51, 14, 99, 0]",
        "A = [0, 0, 1, 12, 14, 14, 14, 21, 51, 51, 51, 99, 99, 99, 4512]",
        "A:",
        "\tlen=15",
        "\tmin=A[13]=0",
        "\tmax=A[0]=4512",
        "\tmost common=99",
        "\tmean=335.866669",
        "\tmax deviation=4176.133301",
    ],
    "./nums_out.txt",
    "4512 99 99 99 51 51 51 21 14 14 14 12 1 0 0 ",
)

assert test(
    "check file input non existing file",
    ["./non_existing.txt"],
    1,
    "",
    "ERROR: failed to open file: [./non_existing.txt]",
)

assert test(
    "check file input",
    ["./example_input2.txt"],
    0,
    "",
    [
        "A = [21, 214, 1, 4512, 51, 14, 99, 0, 12, 41, 45, 41]",
        "A[5] = 14",
        "A[2..4] = [1, 4512, 51]",
    ],
)
