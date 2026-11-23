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
		if (Check(t)) { Advance(); return true; }
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
	if (level == LEVELS.size()) return Unary();      // bottom of the binary ladder

	ExpressionPointer left = ParseBinary(level + 1);           // ask the stickier level first
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
		result = Primary();
	}
	mDepth--;
	return result;
}

ExpressionPointer Parser::Primary() {
	const Token t = Current();       // copy: small struct, avoids reference surprises

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