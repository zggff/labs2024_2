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
fp.writelines([b"Add"])
fp.seek(0)
assert test(
    "parse not enough tokens",
    [fp.name],
    1,
    "",
    ["ERROR: expected [(] got none"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Add()"])
fp.seek(0)
assert test(
    "parse not enough2",
    [fp.name],
    1,
    "",
    ["ERROR: expected [;] got [)]"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Add(x^2*y:);"])
fp.seek(0)
assert test(
    "parsing invalid",
    [fp.name],
    1,
    "",
    ["ERROR: unexpected token [:]"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Add(x^2*x^-3);"])
fp.seek(0)
assert test(
    "parsing negative powers",
    [fp.name],
    1,
    "",
    ["ERROR: monom has powers < 0: {1.000000 * x^-1}"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Add(2.3*x^4*x^-2*y*3+4*y^2+z, y^2-z);", b"Add(-4*y^2);"])
fp.seek(0)
assert test(
    "addition",
    [fp.name],
    0,
    "",
    ["6.900000 * x^2 * y^1 + 5.000000 * y^2", "6.900000 * x^2 * y^1 + 1.000000 * y^2"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Sub(2.3*x^4*x^-2*y*3+4*y^2+z, -y^2+z);", b"Sub(4*y^2);"])
fp.seek(0)
assert test(
    "substraction",
    [fp.name],
    0,
    "",
    ["6.900000 * x^2 * y^1 + 5.000000 * y^2", "6.900000 * x^2 * y^1 + 1.000000 * y^2"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines([b"Mult(x^2+y+z^3, x+y);", b"Mult(z + 1);"])
fp.seek(0)
assert test(
    "multiplication",
    [fp.name],
    0,
    "",
    [
        "1.000000 * x^3 + 1.000000 * x^2 * y^1 + 1.000000 * x^1 * y^1 + 1.000000 * y^2 + 1.000000 * x^1 * z^3 + 1.000000 * y^1 * z^3",
        "1.000000 * x^3 * z^1 + 1.000000 * x^3 + 1.000000 * x^2 * y^1 * z^1 + 1.000000 * x^2 * y^1 + 1.000000 * x^1 * y^1 * z^1 + 1.000000 * x^1 * y^1 + 1.000000 * y^2 * z^1 + 1.000000 * y^2 + 1.000000 * x^1 * z^4 + 1.000000 * x^1 * z^3 + 1.000000 * y^1 * z^4 + 1.000000 * y^1 * z^3",
    ],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"Eval(x^2+y+z^3, {y:0.4, z:0.4});",
    ]
)
fp.seek(0)
assert test(
    "eval error",
    [fp.name],
    1,
    "",
    ["ERROR: value for [x] not provided"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"Add(x^2+y+z^3, 0);",
        b"Eval({x:1, y:0.4, z:0.4});",
        b"Eval(x^2+y+z^3, {x:1, y:0.4, z:0.4});",
    ]
)
fp.seek(0)
assert test(
    "eval",
    [fp.name],
    0,
    "",
    ["1.000000 * x^2 + 1.000000 * y^1 + 1.000000 * z^3", "1.464000", "1.464000"],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"Deriv(2*x^2*y*z+y*x+3*z^3*x, x);",
        b"Deriv(2*x^2*y*z+y*x+3*z^3*x, y);",
        b"Deriv(2*x^2*y*z+y*x+3*z^3*x, z);",
        b"Deriv(2*x^2*y*z+y*x+3*z^3*x, a);",
    ]
)
fp.seek(0)
assert test(
    "derivation",
    [fp.name],
    0,
    "",
    [
        "4.000000 * x^1 * y^1 * z^1 + 1.000000 * y^1 + 3.000000 * z^3",
        "2.000000 * x^2 * z^1 + 1.000000 * x^1",
        "2.000000 * x^2 * y^1 + 9.000000 * x^1 * z^2",
        "0.000000",
    ],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"Prim(2*x^2*y*z+y*x+3*z^3*x, x);",
        b"Prim(2*x^2*y*z+y*x+3*z^3*x, y);",
        b"Prim(2*x^2*y*z+y*x+3*z^3*x, z);",
        b"Prim(2*x^2*y*z+y*x+3*z^3*x, a);",
        b"Prim(24*x, x);",
        b"Prim(x);",
        b"Prim(x);",
    ]
)
fp.seek(0)
assert test(
    "antiderivation",
    [fp.name],
    0,
    "",
    [
        "0.666667 * x^3 * y^1 * z^1 + 0.500000 * x^2 * y^1 + 1.500000 * x^2 * z^3",
        "1.000000 * x^2 * y^2 * z^1 + 0.500000 * x^1 * y^2 + 3.000000 * x^1 * y^1 * z^3",
        "1.000000 * x^2 * y^1 * z^2 + 1.000000 * x^1 * y^1 * z^1 + 0.750000 * x^1 * z^4",
        "2.000000 * a^1 * x^2 * y^1 * z^1 + 1.000000 * a^1 * x^1 * y^1 + 3.000000 * a^1 * x^1 * z^3",
        "12.000000 * x^2",
        "4.000000 * x^3",
        "1.000000 * x^4",
    ],
)
fp.close()

fp = tempfile.NamedTemporaryFile()
fp.writelines(
    [
        b"Grad(x + 2*y + 3*z, e);",
        b"Grad(x*y*z, e);",
    ]
)
fp.seek(0)
assert test(
    "gradient",
    [fp.name],
    0,
    "",
    [
        "1.000000 * e0x^1 + 2.000000 * e0y^1 + 3.000000 * e0z^1",
        "1.000000 * e0x^1 * y^1 * z^1 + 1.000000 * e0y^1 * x^1 * z^1 + 1.000000 * e0z^1 * x^1 * y^1",
    ],
)
fp.close()
