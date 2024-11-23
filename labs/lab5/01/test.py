import os
import sys

sys.path.append(os.getcwd() + "/..")

from test_func import set_test_leaks, test

set_test_leaks(False)

expected = """\
increment/decrement
a        =                             0b1111 = 15
a++      =                             0b1111 = 15
a        =                            0b10000 = 16
++a      =                            0b10001 = 17
a--      =                            0b10001 = 17
a        =                            0b10000 = 16
--a      =                             0b1111 = 15

negation
a        =                             0b1111 = 15
-a       = 0b11111111111111111111111111110001 = -15

addition/subtraction
a        =                             0b1111 = 15
a + 4    =                            0b10011 = 19
a - 4    =                             0b1011 = 11
a + -4   =                             0b1011 = 11
a - -4   =                            0b10011 = 19
a += 4   =                            0b10011 = 19
a -= 4   =                             0b1111 = 15

multiplication
a        =                             0b1111 = 15
a *= 3   =                           0b101101 = 45
a * 2    =                          0b1011010 = 90
a * -2   = 0b11111111111111111111111110100110 = -90

shifts
a        =                             0b1111 = 15
a <<= 2  =                           0b111100 = 60
a >>= 2  =                             0b1111 = 15
a << 3   =                          0b1111000 = 120
a >> 3   =                                0b1 = 1

high/low
a        = 0b11000101111111111010111010000000 = -973099392
high     =                         0b11000101 = 197
high     =                         0b10101110 = 174
a        =                                0b0 = 0
"""

assert test("correct output", [], 0, "", expected)
