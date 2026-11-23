#ifndef AST_H
#define AST_H
#include <memory>
#include <sstream>
#include <string>

#define OVERRIDE__TO_STRING__FUNCTION std::string ToString() const override

class Expression {
public:
	int line;
	int column;
	Expression(int l, int c) : line(l), column(c) {

	}
	virtual ~Expression() = default;
	virtual std::string ToString() const = 0;
};

using ExpressionPointer = std::unique_ptr<Expression>;

class IntExpr : public Expression {
public:
	long long value;
	IntExpr(long long v, int l, int c) : Expression(l, c), value(v) {}
	OVERRIDE__TO_STRING__FUNCTION{
		return std::to_string(value);
	}
};

class FloatExpr : public Expression {
public:
	double value;
	FloatExpr(double v, int l, int c) : Expression(l, c), value(v) {}
	OVERRIDE__TO_STRING__FUNCTION{
		std::ostringstream out;
	out << value;
	return out.str();
	}
};

class BoolExpr : public Expression {
public:
	bool value;
	BoolExpr(bool v, int l, int c) : Expression(l, c), value(v) {

	}
	OVERRIDE__TO_STRING__FUNCTION{
		return value ? "true" : "false";
	}
};

class StringExpr : public Expression {
public:
	std::string text;
	StringExpr(std::string t, int l, int c) : Expression(l, c), text(std::move(t)) {

	}
	OVERRIDE__TO_STRING__FUNCTION{
		return "\"" + text + "\""; 
	}
};

class IdentExpr : public Expression {
public:
	std::string name;
	IdentExpr(std::string n, int l, int c) : Expression(l, c), name(std::move(n)) {

	}
	OVERRIDE__TO_STRING__FUNCTION{
		return name; 
	}
};

class UnaryExpr : public Expression {
public:
	std::string op;
	ExpressionPointer operand;
	UnaryExpr(std::string o, ExpressionPointer e, int l, int c) :
		Expression(l, c), op(std::move(o)), operand(std::move(e)) {

	}
	OVERRIDE__TO_STRING__FUNCTION{
		return "(" + op + " " + operand->ToString() + ")";
	}
};

class BinaryExpr : public Expression {
public:
	std::string op;
	ExpressionPointer left;
	ExpressionPointer right;
	BinaryExpr(std::string o, ExpressionPointer a, ExpressionPointer b, int l, int c)
		: Expression(l, c), op(std::move(o)), left(std::move(a)), right(std::move(b)) {
	}
	OVERRIDE__TO_STRING__FUNCTION{
		return "(" + op + " " + left->ToString() + " " + right->ToString() + ")";
	}
};


#endif // !AST_H
