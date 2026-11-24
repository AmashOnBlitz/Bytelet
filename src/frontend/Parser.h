#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "Token.h"
#include "Ast.h"
#include <memory>

struct ParseError
{
	int line; 
	int column;
	std::string msg;
};

class Parser {
public:
	Parser(const std::vector<Token>& Tokens);
	ExpressionPointer ParseExpression();
	std::vector<StatementPointer> ParseProgram();
	const std::vector<ParseError>& Errors();

private:
	const Token& Current() const;
	const Token& Previous() const;
	bool AtEnd() const;
	const Token& Advance();
	bool MatchAny(const std::vector<TokenType>& types);
	bool Check(TokenType type) const;
	bool IsPlace(const Expression& e);
	const Token& Expect(TokenType type, const std::string& msg);
	[[noreturn]]
	void Fail(const Token& at, const std::string& msg);

	ExpressionPointer Expression();
	ExpressionPointer ParseBinary(std::size_t level);
	ExpressionPointer Unary();
	ExpressionPointer Postfix();
	ExpressionPointer Primary();

	StatementPointer Declaration();
	StatementPointer Statement();
	StatementPointer SimpleStatement();

private:
	const std::vector<Token>& mTokens;
	std::size_t mPos = 0;
	int mDepth = 0;
	std::vector<ParseError> mErrors;
};

#endif // !PARSER_H
