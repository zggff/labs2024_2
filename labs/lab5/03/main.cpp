#include <cstddef>
#include <iomanip>
#include <iostream>
typedef unsigned int uint;

class logical_values_array {
  private:
    uint value;

  public:
    logical_values_array(uint value = 0) : value(value) {
    }
    uint get_value() const {
        return this->value;
    }
    int get_bit(size_t i) const {
        size_t n = sizeof(value) * 8;
        if (i >= n)
            return -1;
        return (this->value & (1 << (31 - i))) != 0;
    }
    void write(char *ptr) const {
        for (size_t i = 0; i < 32; i++) {
            ptr[i] = get_bit(i) + '0';
        }
        ptr[32] = 0;
    }
    logical_values_array inverse() const {
        return logical_values_array(~this->value);
    }
    logical_values_array conjunction(logical_values_array b) const {
        return logical_values_array(this->value & b.value);
    }
    logical_values_array disjunction(logical_values_array b) const {
        return this->inverse().conjunction(b.inverse()).inverse();
    }
    logical_values_array implication(logical_values_array b) const {
        return this->inverse().disjunction(b);
    }
    logical_values_array coimplication(logical_values_array b) const {
        return this->implication(b).inverse();
    }
    logical_values_array exclusive(logical_values_array b) const {
        logical_values_array left = this->inverse().conjunction(b);
        logical_values_array right = this->conjunction(b.inverse());
        return left.disjunction(right);
    }
    logical_values_array equal(logical_values_array b) const {
        return this->exclusive(b).inverse();
    }
    logical_values_array pierce(logical_values_array b) const {
        return this->disjunction(b).inverse();
    }
    logical_values_array sheffer(logical_values_array b) const {
        return this->conjunction(b).inverse();
    }

    static bool equal(logical_values_array a, logical_values_array b) {
        return a.equal(b).inverse().value == 0;
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const logical_values_array &a) {
        char buffer[64];
        a.write(buffer);
        os << buffer;
        return os;
    }
};

#define PRINT_EXPR(str, expr)                                                  \
    {                                                                          \
        using std::cout, std::endl, std::setw, std::left, std::right;          \
        auto _c = (expr);                                                      \
        cout << setw(18) << left << str << " = " << setw(34) << right << _c    \
             << endl;                                                          \
    }

int main(void) {
    logical_values_array a(0b100010011010);
    logical_values_array b(0b010010110011);
    PRINT_EXPR("a", a);
    PRINT_EXPR("b", b);
    PRINT_EXPR("!a", a.inverse());
    PRINT_EXPR("a | b", a.disjunction(b));
    PRINT_EXPR("a & b", a.conjunction(b));
    PRINT_EXPR("a -> b", a.implication(b));
    PRINT_EXPR("a !-> b", a.coimplication(b));
    PRINT_EXPR("a = b", a.equal(b))
    PRINT_EXPR("a ^ b", a.exclusive(b))
    PRINT_EXPR("a pierce b", a.pierce(b))
    PRINT_EXPR("a sheffer b", a.sheffer(b))
    PRINT_EXPR("a == b", logical_values_array::equal(a, b))
    PRINT_EXPR("a == a", logical_values_array::equal(a, a))
    return 0;
}
