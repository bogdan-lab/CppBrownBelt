#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;


class NumNode : public Expression{
public:
    explicit NumNode(int x): num(x){}

    int Evaluate() const override{
        return num;
    }

    string ToString() const override {
        stringstream ss;
        ss << num ;
        return ss.str();
    }
private:
    int num;
};

ExpressionPtr Value(int value){
    return make_unique<NumNode>(value);
}



class SumNode : public Expression{
public:

    SumNode(ExpressionPtr l, ExpressionPtr r): lhs(move(l)), rhs(move(r)) {}

    int Evaluate() const override {
        return lhs->Evaluate() + rhs->Evaluate();
    }

    string ToString() const override{
        stringstream ss;
        ss << '(' << lhs->ToString() << ")+(" << rhs->ToString() << ')';
        return ss.str();
    }

private:
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right){
    return make_unique<SumNode>(move(left), move(right));
}


class ProdNode : public Expression {
public:
    ProdNode(ExpressionPtr l, ExpressionPtr r): lhs(move(l)), rhs(move(r)) {}

    int Evaluate() const override {
        return lhs->Evaluate()*rhs->Evaluate();
    }

    string ToString() const override {
        stringstream ss;
        ss << '(' << lhs->ToString() << ")*("<< rhs->ToString() << ')';
        return ss.str();
    }
private:
    ExpressionPtr lhs;
    ExpressionPtr rhs;
};


ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right){
    return make_unique<ProdNode>(move(left), move(right));
}


string Print(const Expression* e) {
  if (!e) {
    return "Null expression provided";
  }
  stringstream output;
  output << e->ToString() << " = " << e->Evaluate();
  return output.str();
}

void Test() {
  ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
  ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

  ExpressionPtr e2 = Sum(move(e1), Value(5));
  ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

  ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, Test);
//    cout << Print(Value(5).get());
  return 0;
}
