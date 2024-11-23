import os
import sys

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test

set_test_leaks(False)

expected = """\
a          = (9.000000 + 4.000000i)
b          = (3.000000 + -2.000000i)
a + b      = (12.000000 + 2.000000i)
a - b      = (6.000000 + 6.000000i)
a * b      = (35.000000 + -6.000000i)
a / b      = (1.461538 + 2.307692i)
a.mod()    = 9.84886
a.arg()    = 0.418224
"""

assert test("correct output", [], 0, "", expected)
