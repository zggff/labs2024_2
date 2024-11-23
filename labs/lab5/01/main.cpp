#include <cstddef>
#include <iomanip>
#include <iostream>
#include <utility>

class binary_int {
  private:
    int val;

  public:
    binary_int() : val(0) {
    }

    binary_int(int a) : val(a) {
    }

    int get_val() const {
        return this->val;
    }

    binary_int operator++(int) {
        binary_int tmp = *this;

        size_t shift = 1;
        bool end = false;
        while (!end && shift < UINT32_MAX) {
            if ((val & shift) == 0)
                end = true;
            val ^= shift;
            shift <<= 1;
        }
        return tmp;
    }
    binary_int &operator++() {
        this->operator++(0);
        return *this;
    }
    binary_int &operator--() {
        this->operator--(0);
        return *this;
    }

    binary_int operator--(int) {
        binary_int tmp = *this;

        size_t shift = 1;
        bool end = false;
        while (!end && shift < UINT32_MAX) {
            if ((val & shift) != 0)
                end = true;
            val ^= shift;
            shift <<= 1;
        }
        return tmp;
    }
    binary_int operator-() const {
        binary_int r = *this;
        r.val = ~r.val;
        r++;
        return r;
    }
    binary_int operator+=(binary_int b) {
        if (b.val < 0)
            return *this -= (-b);
        while (b.val) {
            (*this)++;
            b--;
        }
        return *this;
    }
    binary_int operator-=(binary_int b) {
        if (b.val < 0)
            return *this += (-b);
        while (b.val) {
            (*this)--;
            b--;
        }
        return *this;
    }
    binary_int operator+(binary_int b) const {
        binary_int r = *this;
        return r += b;
    }
    binary_int operator-(binary_int b) const {
        binary_int r = *this;
        return r -= b;
    }
    binary_int operator*=(binary_int b) {
        bool neg = b.val < 0;
        if (neg)
            b = -b;
        binary_int t = *this;
        this->val = 0;
        while (b.val > 0) {
            (*this) += t;
            b--;
        }
        if (neg)
            (*this) = -(*this);
        return *this;
    }
    binary_int operator*(binary_int b) const {
        binary_int r = *this;
        return r *= b;
    }
    binary_int operator<<=(binary_int b) {
        return this->val <<= b.val;
    }
    binary_int operator>>=(binary_int b) {
        return this->val >>= b.val;
    }
    binary_int operator<<(binary_int b) const {
        binary_int r = *this;
        return r <<= b;
    }
    binary_int operator>>(binary_int b) const {
        binary_int r = *this;
        return r >>= b;
    }
    bool operator>=(binary_int b) const {
        return this->val >= b.val;
    }
    bool operator<(binary_int b) const {
        return this->val < b.val;
    }

    friend std::ostream &operator<<(std::ostream &os, const binary_int &a) {
        char buffer[64] = "0b";
        binary_int shift = 31;
        binary_int i = 2;
        bool print = false;
        while (shift >= 0) {
            bool bit = (a.val & (1 << shift.val)) != 0;
            if (bit)
                print = true;
            if (print) {
                buffer[i.val] = bit ? '1' : '0';
                i++;
            }
            shift--;
        }
        if (!print) {
            buffer[i.val] = '0';
            i++;
        }
        buffer[i.val] = 0;
        os << buffer;
        return os;
    }
    //   31 23 15 7   0
    // 0b0...0    0...0
    std::pair<binary_int, binary_int> get_low_high() {
        int mask = 0b11111111;
        binary_int high;
        binary_int low;
        high.val = (this->val >> 24) & mask;
        low.val = (this->val >> 8) & mask;
        this->val = 0;
        return std::make_pair(high, low);
    }
};

#define PRINT_EXPR(str, expr)                                                  \
    {                                                                          \
        using std::cout, std::endl, std::setw, std::left, std::right;          \
        auto _c = (expr);                                                      \
        cout << setw(8) << left << str << " = " << setw(34) << right << _c     \
             << " = " << _c.get_val() << endl;                                 \
    }

#define PRINT(expr) PRINT_EXPR(#expr, expr)

int main() {
    using std::cout, std::endl, std::setw, std::left, std::right;
    {
        cout << "increment/decrement" << endl;
        binary_int a(0xf);
        PRINT(a);
        PRINT(a++);
        PRINT(a);
        PRINT(++a);
        PRINT(a--);
        PRINT(a);
        PRINT(--a);
    }
    cout << endl;
    {
        cout << "negation" << endl;
        binary_int a(0xf);
        PRINT(a);
        PRINT(-a);
    }
    cout << endl;
    {
        cout << "addition/subtraction" << endl;
        binary_int a(0xf);
        PRINT(a);
        PRINT(a + 4);
        PRINT(a - 4);
        PRINT(a + -4);
        PRINT(a - -4);
        PRINT(a += 4);
        PRINT(a -= 4);
    }
    cout << endl;
    {
        cout << "multiplication" << endl;
        binary_int a(0xf);
        PRINT(a);
        PRINT(a *= 3);
        PRINT(a * 2);
        PRINT(a * -2);
    }
    cout << endl;
    {
        cout << "shifts" << endl;
        binary_int a(0xf);
        PRINT(a);
        PRINT(a <<= 2);
        PRINT(a >>= 2);
        PRINT(a << 3);
        PRINT(a >> 3);
    }
    cout << endl;
    {
        cout << "high/low" << endl;
        //             |       |       |       |       |
        binary_int a(0b11000101111111111010111010000000);
        PRINT(a);
        auto c = a.get_low_high();
        PRINT_EXPR("high", c.first);
        PRINT_EXPR("high", c.second);
        PRINT(a);
    }
}
