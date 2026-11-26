#include "Parser.h"
#include <charconv>

namespace {
	struct ParseFailure {};
	const int MAX_DEPTH = 100;
	const std::vector<std::vector<TokenType>> LEVELS = {
	{ TokenType::KwOr },
	{ TokenType::KwAnd },
	{ TokenType::EqualEqual, TokenType::BangEqual },
	{ TokenType::Less, TokenType::LessEqual, TokenType::Greater, TokenType::GreaterEqual },
	{ TokenType::Plus, TokenType::Minus },
	{ TokenType::Star, TokenType::Slash, TokenType::Percent },
	};
}

Parser::Parser(const std::vector<Token>& Tokens) : mTokens(Tokens)
{
}

ExpressionPointer Parser::ParseExpression()
{
	mErrors.clear();
	mPos = 0;
	mDepth = 0;
	try {
		ExpressionPointer expr = Expression();
		if (!AtEnd()) {
			Fail(Current(), "unexpected token after expression");
		}
		return expr;
	}
	catch (const ParseFailure&) {
		return nullptr;
	}
}

const std::vector<ParseError>& Parser::Errors()
{
	return this->mErrors;
}

const Token& Parser::Current() const
{
	return mTokens[mPos];
}

const Token& Parser::Previous() const
{
	return mTokens[mPos - 1];
}

bool Parser::AtEnd() const
{
	return Current().type == TokenType::EndOfFile;
}

const Token& Parser::Advance()
{
	if (!AtEnd()) mPos++;
	return Previous();
}

bool Parser::MatchAny(const std::vector<TokenType>& types)
{
	for (TokenType t : types) {
		if (Check(t)) { 
			Advance();
			return true;
		}
	}
	return false;
}

bool Parser::Check(TokenType type) const
{
	return Current().type == type;
}


const Token& Parser::Expect(TokenType type, const std::string& msg) {
	if (Check(type)) return Advance();
	Fail(Current(), msg);
}

void Parser::Fail(const Token& at, const std::string& msg) {
	mErrors.push_back({ at.line, at.column, msg });
	throw ParseFailure{};
}

ExpressionPointer Parser::Expression()
{
	return ParseBinary(0);
}

ExpressionPointer Parser::ParseBinary(std::size_t level) {
	if (level == LEVELS.size()) return Unary();

	ExpressionPointer left = ParseBinary(level + 1);
	while (MatchAny(LEVELS[level])) {
		const Token& op = Previous();
		ExpressionPointer right = ParseBinary(level + 1);
		left = std::make_unique<BinaryExpr>(
			std::string(op.lexeme), std::move(left), std::move(right), op.line, op.column);
	}
	return left;
}

ExpressionPointer Parser::Unary() {
	if (++mDepth > MAX_DEPTH) Fail(Current(), "expression is nested too deeply");

	ExpressionPointer result;
	if (MatchAny({ TokenType::Minus, TokenType::KwNot })) {
		const Token& op = Previous();
		ExpressionPointer operand = Unary();
		result = std::make_unique<UnaryExpr>(std::string(op.lexeme), std::move(operand), op.line, op.column);
	}
	else {
		//result = Primary();
		result = Postfix(); 
	}
	mDepth--;
	return result;
}

ExpressionPointer Parser::Postfix() {
	ExpressionPointer expr = Primary();

	while (true)
	{
		if (MatchAny({ TokenType::LParen })) {
			std::vector<ExpressionPointer> args;
			if (!Check(TokenType::RParen)) {
				do {
					args.push_back(Expression());
				} while (MatchAny({ TokenType::Comma }));
			}
			Expect(TokenType::RParen, "expected ')' after arguments");
			int line = expr->line, col = expr->column;
			expr = std::make_unique<CallExpr>(std::move(expr), std::move(args), line, col);
		}
		else if (MatchAny({ TokenType::LBracket })) {
			ExpressionPointer index = Expression();
			Expect(TokenType::RBracket, "expected ']' after index");
			int line = expr->line, col = expr->column;
			expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index), line, col);
		}
		else {
			break;
		}
	}
	return expr;
}

ExpressionPointer Parser::Primary() {
	const Token t = Current();

	switch (t.type) {
	case TokenType::IntLiteral: {
		Advance();
		long long value = 0;
		auto res = std::from_chars(t.lexeme.data(), t.lexeme.data() + t.lexeme.size(), value);
		if (res.ec != std::errc()) Fail(t, "integer literal is too large");
		return std::make_unique<IntExpr>(value, t.line, t.column);
	}
	case TokenType::FloatLiteral: {
		Advance();
		double value = 0;
		auto res = std::from_chars(
			t.lexeme.data(),
			t.lexeme.data() + t.lexeme.size(),
			value
		);
		if (res.ec != std::errc()) Fail(t, "float literal is out of range");
		return std::make_unique<FloatExpr>(value, t.line, t.column);
	}
	case TokenType::KwTrue:
		Advance();
		return std::make_unique<BoolExpr>(true, t.line, t.column);
	case TokenType::KwFalse:
		Advance();
		return std::make_unique<BoolExpr>(false, t.line, t.column);
	case TokenType::StringLiteral: {
		Advance();
		std::string inner(t.lexeme.substr(1, t.lexeme.size() - 2));
		return std::make_unique<StringExpr>(std::move(inner), t.line, t.column);
	}
	case TokenType::StringStart:
		Fail(t, "string interpolation is not supported by the parser yet");
	case TokenType::Identifier:
		Advance();
		return std::make_unique<IdentExpr>(std::string(t.lexeme), t.line, t.column);
	case TokenType::LParen: {
		Advance();
		if (++mDepth > MAX_DEPTH) Fail(t, "expression is nested too deeply");
		ExpressionPointer inner = Expression();
		mDepth--;
		Expect(TokenType::RParen, "expected ')' after expression");
		return inner;
	}
	default:
		Fail(t, "expected an expression");
	}
}

StatementPointer Parser::Declaration()
{
	const Token keyword = Advance();
	bool isMutable = (keyword.type == TokenType::KwVar);
	const Token name = Expect(
		TokenType::Identifier,
		"expected a variable name"
	);
	std::string typeName;
	if (MatchAny({ TokenType::Colon })) {
		if (MatchAny(
			{
				TokenType::KwInt,
				TokenType::KwFloat,
				TokenType::KwBool,
				TokenType::KwString
			}
		)) 
		{
			typeName = std::string(Previous().lexeme);
		}
		else {
			Fail(Current(), "expected a type name after ':'");
		}
	}

	ExpressionPointer equatedVal;
	if (MatchAny({ TokenType::Equal })) {
		equatedVal = Expression();
	}
	else if (!isMutable) {
		Fail(name, "immutable variable '" + std::string(name.lexeme) + "' must be initialized");
	}
	else if (typeName.empty()) {
		Fail(name, "cannot infer type of '" + std::string(name.lexeme) + "': add a type or an initializer");
	}
	ExpectSemicolon( "expected ';' after declaration");

	return std::make_unique<VarDeclStatement>(
		isMutable,
		std::string(name.lexeme),
		std::move(typeName),
		std::move(equatedVal),
		keyword.line,
		keyword.column
	);
}

StatementPointer Parser::Statement()
{
	if (++mDepth > MAX_DEPTH) Fail(Current(), "statements are nested too deeply");

	StatementPointer result;
	if (Check(TokenType::KwLet) || Check(TokenType::KwVar))
		result = Declaration();

	else if (Check(TokenType::KwIf))
		result = ParseIf();

	else if (Check(TokenType::KwWhile))
		result = ParseWhile();

	else if (Check(TokenType::LBrace))
		result = ParseBlock();

	else 
		result = SimpleStatement();

	mDepth--;
	return result;
}

StatementPointer Parser::SimpleStatement()
{
	ExpressionPointer target = Expression();

	if (MatchAny({
		TokenType::Equal,
		TokenType::PlusEqual,
		TokenType::MinusEqual,
		TokenType::StarEqual,
		TokenType::SlashEqual })
		) {
		const Token opr = Previous();
		if (!IsPlace(*target)) {
			Fail(opr, "invalid assignment target");
		}

		ExpressionPointer val = Expression();
		ExpectSemicolon( "expected ';' after assignment");
		return std::make_unique<AssignStatement>(
			std::string(opr.lexeme),
			std::move(target),
			std::move(val),
			opr.line,
			opr.column
		);
	}
	if (MatchAny({
		TokenType::PlusPlus,
		TokenType::MinusMinus })
		) {
		const Token opr = Previous();
		if (!IsPlace(*target)) {
			Fail(opr, "invalid increment/decrement target");
		}

		ExpectSemicolon( "expected ';' after increment/decrement");
		return std::make_unique<AssignStatement>(
			std::string(opr.lexeme),
			std::move(target),
			nullptr,
			opr.line,
			opr.column
		);
	}
	const int line = target->line;
	const int column = target->column;
	ExpectSemicolon( "expected ';' after expression");

	return std::make_unique<ExprStatement>(
		std::move(target),
		line,
		column
	);
}

StatementPointer Parser::ParseBlock()
{
	const Token openBrace = Advance();
	std::vector<StatementPointer> statements;

	while (!Check(TokenType::RBrace) && !AtEnd()) {
		const std::size_t startPos = mPos;
		const int startDepth = mDepth;
		try {
			statements.push_back(Statement());
		}
		catch (const ParseFailure&) {
			Recover(startPos, startDepth);
		}
	}
	Expect(TokenType::RBrace, "expected '}' to close block");
	return std::make_unique<BlockStatement>(
		std::move(statements),
		openBrace.line,
		openBrace.
		column
	);
}

StatementPointer Parser::ParseIf()
{
	const Token currentChar = Advance();
	Expect(TokenType::LParen, "expected '(' after 'if'");
	ExpressionPointer cond = Expression();
	Expect(TokenType::RParen, "expected ')' after condition");

	StatementPointer thenBranch = Body();
	StatementPointer elseBranch;
	if (MatchAny({ TokenType::KwElse })) {
		elseBranch = Body();
	}
	return std::make_unique<IfStatement>(
		std::move(cond),
		std::move(thenBranch),
		std::move(elseBranch),
		currentChar.line,
		currentChar.column
	);
}

StatementPointer Parser::ParseWhile()
{
	const Token currChar = Advance();
	Expect(TokenType::LParen, "expected '(' after 'while'");
	ExpressionPointer cond = Expression();
	Expect(TokenType::RParen, "expected ')' after condition");

	StatementPointer body = Body();
	return std::make_unique<WhileStatement>(
		std::move(cond),
		std::move(body),
		currChar.line,
		currChar.column
	);
}

StatementPointer Parser::Body()
{
	if (Check(TokenType::KwLet) || Check(TokenType::KwVar))
		Fail(Current(), "declaration is not allowed here, use braces");
	return Statement();
}


std::vector<StatementPointer> Parser::ParseProgram() {
	mErrors.clear();
	mPos = 0;
	mDepth = 0;
	std::vector<StatementPointer> program;
	while (!AtEnd()) {
		const std::size_t startPos = mPos;
		const int startDepth = mDepth;
		try {
			program.push_back(Statement());
		}
		catch (const ParseFailure&) {
			Recover(startPos, startDepth);
		}
	}
	return program;
}

bool Parser::IsPlace(const ::Expression& e) {
	return dynamic_cast<const IdentExpr*>(&e) != nullptr ||
		dynamic_cast<const IndexExpr*>(&e) != nullptr;
}

void Parser::Synchronize()
{
	while (!AtEnd())
	{
		if (mPos > 0 && Previous().type == TokenType::Semicolon) return;

		if (Check(TokenType::KwLet) ||
			Check(TokenType::KwVar) ||
			Check(TokenType::KwIf) ||
			Check(TokenType::KwWhile) ||
			Check(TokenType::KwFor) ||
			Check(TokenType::KwFn) ||
			Check(TokenType::KwReturn) ||
			Check(TokenType::KwBreak) ||
			Check(TokenType::KwContinue) ||
			Check(TokenType::RBrace)) 
		{
			return;
		}
		Advance();
	}
}

void Parser::ExpectSemicolon(const std::string& msg)
{
	if (Check(TokenType::Semicolon)) { Advance(); return; }
	const Token& last = Previous();
	mErrors.push_back({ last.line, last.column + static_cast<int>(last.lexeme.size()), msg });
	throw ParseFailure{};
}

void Parser::Recover(std::size_t startPos, int startDepth)
{
	mDepth = startDepth;
	if (mPos == startPos && !AtEnd()) Advance();
	Synchronize();
}
