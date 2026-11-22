#ifndef LEXER_H
#define LEXER_H
#include <string_view>
#include <vector>
#include "Token.h"
#include <string>

struct LexError {
	int line;
	int column;
	std::string msg;
};

class Lexer {
public:
	explicit Lexer(std::string_view corpus);
	~Lexer();
	std::vector<Token> Tokenize();

	void SetCorpus(std::string_view corpus);
	std::string_view GetCorpus();
	std::vector<LexError>& errors();

private:
	void refreshVariables();
	
	char Current();
	void Advance();
	char Peek();
	bool atEnd();
	void SkipTrivia();
	Token scanToken();
	Token MakeToken(TokenType type) const;
private:
	std::string_view mCorpus;
	std::size_t mStart= 0;
	std::size_t mCurrent = 0;
	int mLine = 1;
	int mColumn = 1;
	int mStartLine = 1;
	int mStartColumn = 1;
	std::vector<LexError> mErrors;
	std::vector<Token> mTokens;
	char mTmpChar;
};

#endif // !LEXER_H

