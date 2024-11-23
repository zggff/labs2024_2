#include <iostream>
#include <iomanip>

class complex {
  public:
    double real;
    double imag;
    complex(double real = 0, double imag = 0) : real(real), imag(imag) {
    }
    complex add(complex b) const {
        return complex(real + b.real, imag + b.imag);
    }
    complex sub(complex b) const {
        return complex(real - b.real, imag - b.imag);
    }
    complex mul(complex b) const {
        return complex(real * b.real - imag * b.imag,
                       imag * b.real + real * b.imag);
    }
    complex div(complex b) const {
        double den = b.real * b.real + b.imag * b.imag;
        return complex((real * b.real + imag * b.imag) / den,
                       (imag * b.real - real * b.imag) / den);
    }
    double mod() const {
        return sqrt(real * real + imag * imag);
    }
    double arg() const {
        return std::atan(imag / real);
    }
    complex operator+(complex b) const {
        return this->add(b);
    }
    complex operator-(complex b) const {
        return this->sub(b);
    }
    complex operator*(complex b) const {
        return this->mul(b);
    }
    complex operator/(complex b) const {
        return this->div(b);
    }
    friend std::ostream &operator<<(std::ostream &os, const complex &a) {
        std::string buf = "(";
        buf += std::to_string(a.real);
        buf += " + ";
        buf += std::to_string(a.imag);
        buf += "i)";
        os << buf;
        return os;
    }
};

#define PRINT_EXPR(str, expr)                                                  \
    {                                                                          \
        using std::cout, std::endl, std::setw, std::left, std::right;          \
        auto _c = (expr);                                                      \
        cout << setw(10) << left << str << " = " << _c << endl;                \
    }

#define PRINT(expr) PRINT_EXPR(#expr, expr)

int main(void) {
    complex a(9, 4);
    complex b(3, -2);
    PRINT(a);
    PRINT(b);
    PRINT(a + b);
    PRINT(a - b);
    PRINT(a * b);
    PRINT(a / b);
    PRINT(a.mod());
    PRINT(a.arg());

    return 0;
}
