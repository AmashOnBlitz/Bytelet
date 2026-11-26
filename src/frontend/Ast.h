#ifndef AST_H
#define AST_H
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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

class CallExpr : public Expression {
public:
	ExpressionPointer callee;
	std::vector<ExpressionPointer> args;
	CallExpr(ExpressionPointer c, std::vector<ExpressionPointer> a, int l, int col)
		: Expression(l, col), callee(std::move(c)), args(std::move(a)) {
	}
	OVERRIDE__TO_STRING__FUNCTION{
		std::string s = "(call " + callee->ToString();
		for (const auto& a : args) s += " " + a->ToString();
		return s + ")";
	}
};

class IndexExpr : public Expression {
public:
	ExpressionPointer object;
	ExpressionPointer index;
	IndexExpr(ExpressionPointer o, ExpressionPointer i, int l, int c)
		: Expression(l, c), object(std::move(o)), index(std::move(i)) {
	}
	OVERRIDE__TO_STRING__FUNCTION{
		return "(index " + object->ToString() + " " + index->ToString() + ")";
	}
};



class Statement {
public:
	int line;
	int column;
	Statement(int l, int c) : line(l), column(c) {}
	virtual ~Statement() = default;
	virtual std::string ToString() const = 0;
};
using StatementPointer = std::unique_ptr<Statement>;


class VarDeclStatement : public Statement {
public:
	bool isMutable;
	std::string name;
	std::string typeName;
	ExpressionPointer init;
	VarDeclStatement(bool m, std::string n, std::string t, ExpressionPointer i, int l, int c)
		: Statement(l, c), 
		isMutable(m),
		name(std::move(n)),
		typeName(std::move(t)),
		init(std::move(i)) 
	{
	}
	OVERRIDE__TO_STRING__FUNCTION{
		std::string s = std::string("(") + (isMutable ? "var" : "let") + " " + name;
		if (!typeName.empty()) s += ":" + typeName;
		if (init) s += " " + init->ToString();
		return s + ")";
	}
};

class ExprStatement : public Statement {
public:
	ExpressionPointer expr;
	ExprStatement(ExpressionPointer e, int l, int c) : Statement(l, c), expr(std::move(e)) {}
	OVERRIDE__TO_STRING__FUNCTION{
		return "(expr " + expr->ToString() + ")"; 
	}
};

class AssignStatement : public Statement {
public:
	std::string op;
	ExpressionPointer target;
	ExpressionPointer value;
	AssignStatement(std::string o, ExpressionPointer t, ExpressionPointer v, int l, int c)
		: Statement(l, c), op(std::move(o)), target(std::move(t)), value(std::move(v)) {
	}
	OVERRIDE__TO_STRING__FUNCTION {
		std::string s = "(" + op + " " + target->ToString();
		if (value) s += " " + value->ToString();
		return s + ")";
	}
};

class BlockStatement : public Statement {
public:
	std::vector<StatementPointer> statements;
	BlockStatement(std::vector<StatementPointer> s, int l, int c)
		: Statement(l, c), statements(std::move(s)) {

	}
	OVERRIDE__TO_STRING__FUNCTION{
		std::stringstream ss;
		ss << "(block";
		for (const auto& stmt : statements) {
			ss << " " << stmt->ToString();
		}
		ss << ")";
		return ss.str();
	}
};

class IfStatement : public Statement {
public:
	ExpressionPointer condition;
	StatementPointer thenBranch;
	StatementPointer elseBranch;
	IfStatement(
		ExpressionPointer cond,
		StatementPointer then,
		StatementPointer Else,
		int l,
		int c)
		: Statement(l, c), 
		condition(std::move(cond)),
		thenBranch(std::move(then)),
		elseBranch(std::move(Else)) 
	{

	}

	OVERRIDE__TO_STRING__FUNCTION{
	std::string s = "(if " + condition->ToString()
		+ " " + thenBranch->ToString();
	if (elseBranch) s += " " + elseBranch->ToString();
	return s + ")";
	}
};

class WhileStatement : public Statement {
public:
	ExpressionPointer condition;
	StatementPointer body;

	WhileStatement(ExpressionPointer cond, StatementPointer Body, int l, int c)
		: Statement(l, c), condition(std::move(cond)), body(std::move(Body)) {

	}

	OVERRIDE__TO_STRING__FUNCTION{
	return "(while " + condition->ToString()
		+ " " + body->ToString() + ")";
	}
};


#endif // !AST_H
