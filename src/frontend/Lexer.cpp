#include "lexer.h"
#include <iostream>

Lexer::Lexer(std::string_view corpus) :
	mCorpus(corpus)
{

}


Lexer::~Lexer()
{

}

void Lexer::SetCorpus(std::string_view corpus)
{
	mCorpus = corpus;
}

std::string_view Lexer::GetCorpus()
{
	return mCorpus;
}

std::vector<LexError>& Lexer::errors()
{
	return mErrors;
}

void Lexer::refreshVariables()
{
	mStart = 0;
	mCurrent = 0;
	mLine = 1;
	mColumn = 1;
	mStartLine = 1;
	mStartColumn = 1;
	mErrors.clear();
	mTokens.clear();
}

char Lexer::Current()
{
	if (atEnd()) return '\0';
	return mCorpus[mCurrent];
}

void Lexer::Advance()
{
	if (atEnd()) return;
	char c = mCorpus[mCurrent];
	mCurrent += 1;
	if (c == '\n') {
		mLine++;
		mColumn = 1;
	}
	else {
		mColumn++;
	}
}

char Lexer::Peek()
{
	if (atEnd() || mCurrent + 1 >= mCorpus.size()) return '\0';
	return mCorpus[mCurrent + 1];
}

bool Lexer::atEnd()
{
	return mCurrent >= mCorpus.size();
}

void Lexer::SkipWhitespace()
{
	while (!atEnd()) {
		mTmpChar = Current();
		if (mTmpChar == ' ' ||
			mTmpChar == '\t' ||
			mTmpChar == '\r' ||
			mTmpChar == '\n')
		{
			Advance();
		}
		else
		{
			break;
		}
	}
}

Token Lexer::scanToken() {
	char c = Current();
	char nxt = Peek();
	Advance();

	switch (c) {
	case '+': {
		if (nxt == '+') {
			Advance();
			return MakeToken(TokenType::PlusPlus);
		}
		else if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::PlusEqual);
		}
		else {
			return MakeToken(TokenType::Plus);
		}
	}
	case '-': {
		if (nxt == '-') {
			Advance();
			return MakeToken(TokenType::MinusMinus);
		}
		else if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::MinusEqual);
		}
		else if (nxt == '>') {
			Advance();
			return MakeToken(TokenType::Arrow);
		}
		else {
			return MakeToken(TokenType::Minus);
		}
	}
	case '*': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::StarEqual);
		}
		else {
			return MakeToken(TokenType::Star);
		}
	}
	case '/': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::SlashEqual);
		}
		else if (nxt == '/') {
			Advance();

			while (!atEnd() && Current() != '\n') {
				Advance();
			}

			return MakeToken(TokenType::LineComment);
		}
		else if (nxt == '*') {
			Advance();

			while (!atEnd()) {
				char current = Current();
				Advance();

				if (current == '*' && !atEnd() && Current() == '/') {
					Advance();
					return MakeToken(TokenType::BlockComment);
				}
			}

			Token tkn = MakeToken(TokenType::Error);
			mErrors.push_back({
				mStartLine,
				mStartColumn,
				"unterminated block comment"
							  });
			return tkn;
		}
		else {
			return MakeToken(TokenType::Slash);
		}
	}
	case '%': return MakeToken(TokenType::Percent);

	case '=': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::EqualEqual);
		}
		else {
			return MakeToken(TokenType::Equal);
		}
	}
	case '!': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::BangEqual);
		}
		else {
			return MakeToken(TokenType::Bang);
		}
	}
	case '<': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::LessEqual);
		}
		else {
			return MakeToken(TokenType::Less);
		}
	}
	case '>': {
		if (nxt == '=') {
			Advance();
			return MakeToken(TokenType::GreaterEqual);
		}
		else {
			return MakeToken(TokenType::Greater);
		}
	}

	case '(': return MakeToken(TokenType::LParen);
	case ')': return MakeToken(TokenType::RParen);
	case '{': return MakeToken(TokenType::LBrace);
	case '}': return MakeToken(TokenType::RBrace);
	case '[': return MakeToken(TokenType::LBracket);
	case ']': return MakeToken(TokenType::RBracket);

	case ',': return MakeToken(TokenType::Comma);
	case ':': return MakeToken(TokenType::Colon);
	case ';': return MakeToken(TokenType::Semicolon);

	case '.': {
		if (nxt == '.') {
			Advance();
			return MakeToken(TokenType::DotDot);
		}
		else {
			return MakeToken(TokenType::Dot);
		}
	}

	default:
		Token tkn = MakeToken(TokenType::Error);
		std::string errMsg = "unexpected character \"";
		errMsg += tkn.lexeme;
		errMsg += "\"";

		mErrors.push_back({
			mStartLine,
			mStartColumn,
			errMsg
						  });

		return tkn;
	}
}

Token Lexer::MakeToken(TokenType type) const
{
	Token token;
	token.type = type;
	token.line = mStartLine;
	token.column = mStartColumn;
	token.lexeme = mCorpus.substr(mStart, mCurrent - mStart);
	return token;
}


std::vector<Token> Lexer::Tokenize()
{
	refreshVariables();
	while (true)
	{
		SkipWhitespace();
		mStart = mCurrent;
		mStartLine = mLine;
		mStartColumn = mColumn;
		if (atEnd()) {
			mTokens.push_back(MakeToken(TokenType::EndOfFile));
			break;
		}
		mTokens.push_back(scanToken());
	}
	return mTokens;
}