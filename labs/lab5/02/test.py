import os
import sys
import tempfile

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test

set_test_leaks(False)

assert test("no key", [], 1, "", "ERROR: key not provided")
assert test("invalid key", ["["], 1, "", "ERROR: invalid key")
assert test("input file not provided", ["f"], 1, "", "ERROR: input file not provided")
assert test(
    "output file not provided", ["f", "a"], 1, "", "ERROR: output file not provided"
)
assert test(
    "non existent file",
    ["f", "non existent", "b"],
    1,
    "",
    "ERROR: failed to open file [non existent]",
)

fp1 = tempfile.NamedTemporaryFile()
fp2 = tempfile.NamedTemporaryFile()

text = "The quick brown fox jumps over the lazy dog."

fp1.writelines([text.encode()])
fp1.seek(0)
assert test(
    "encode file",
    ["ff12fafcfe", fp1.name, fp2.name],
    0,
    "",
    "",
    fp2.name,
    bytes.fromhex(
        "470724aa26f1f7417a11717eb48b1120a16f028fb38a672857e3acd2bbbe7cb362c31d70686cf93af80dcbcd"
    ),
)
fp1.seek(0)
fp2.seek(0)

assert test(
    "decode file", ["ff12fafcfe", fp2.name, fp1.name], 0, "", "", fp1.name, text
)

fp1.close()
fp2.close()
