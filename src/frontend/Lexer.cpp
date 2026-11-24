#include "Lexer.h"
#include <iostream>
#include <unordered_map>

static bool IsAlpha(char c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_'));
}

static bool IsDigit(char c) {
	return ((c >= '0' && c <= '9'));
}

static bool IsPeriod(char c) {
	return (c == '.');
}

static const std::unordered_map<std::string_view, TokenType> KEYWORDS = {
	{ "let",      TokenType::KwLet },
	{ "var",      TokenType::KwVar },
	{ "fn",       TokenType::KwFn },
	{ "if",       TokenType::KwIf },
	{ "else",     TokenType::KwElse },
	{ "for",      TokenType::KwFor },
	{ "while",    TokenType::KwWhile },
	{ "in",       TokenType::KwIn },
	{ "break",    TokenType::KwBreak },
	{ "continue", TokenType::KwContinue },
	{ "return",   TokenType::KwReturn },
	{ "import",   TokenType::KwImport },
	{ "int",	  TokenType::KwInt },    
	{ "float",    TokenType::KwFloat },
	{ "bool",     TokenType::KwBool },   
	{"string",	  TokenType::KwString },
	{ "void",     TokenType::KwVoid },
	{ "true",     TokenType::KwTrue },   
	{ "false",    TokenType::KwFalse },
	{ "and",      TokenType::KwAnd },    
	{ "or",		  TokenType::KwOr },
	{ "not",      TokenType::KwNot },
};

Lexer::Lexer(std::string_view corpus) :
	mCorpus(corpus)
{

}

Lexer::~Lexer() {

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

void Lexer::RefreshVariables()
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

void Lexer::Advance() {
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

void Lexer::SkipTrivia()
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
		else if (mTmpChar == '/' && Peek() == '/') {
			while (!atEnd() && Current() != '\n') {
				Advance();
			}
		}
		else if (mTmpChar == '/' && Peek() == '*') {
			int cLine = mLine;
			int cCol = mColumn;
			Advance();
			Advance();

			bool closed = false;
			while (!atEnd()) {
				if (Current() == '*' && Peek() == '/') {
					Advance();
					Advance();
					closed = true;
					break;
				}
				Advance();
			}
			if (!closed) {
				mErrors.push_back({ cLine, cCol, "unterminated block comment" });
			}
		}
		else {
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
	// Comment handling is moved to trivia, comment code below here is dead
	// for later reference
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
			Token tkn = MakeToken(TokenType::Error);
			mErrors.push_back({ mStartLine, mStartColumn, "unexpected '!' (use 'not' for logical negation)" });
			return tkn;
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

	case '"':
		return ScanStringText(false);

	case '{':
		if (!mInterpDepth.empty()) mInterpDepth.back()++;
		return MakeToken(TokenType::LBrace);

	case '}':
		if (!mInterpDepth.empty()) {
			if (mInterpDepth.back() > 0) {
				mInterpDepth.back()--;
				return MakeToken(TokenType::RBrace);
			}
			mInterpDepth.pop_back();
			return ScanStringText(true);
		}
		return MakeToken(TokenType::RBrace);


	default:

		if (IsAlpha(c)) {
			while (IsAlpha(Current()) || IsDigit(Current())) {
				Advance();
			}
			Token tkn = MakeToken(TokenType::Identifier);
			auto index = KEYWORDS.find(tkn.lexeme);
			if (index != KEYWORDS.end()) {
				tkn.type = index->second;
			}
			return tkn;
		}

		if (IsDigit(c)) {
			TokenType type = TokenType::IntLiteral;
			while (IsDigit(Current())) {
				Advance();
			}
			if (IsPeriod(Current()) && IsDigit(Peek())) {
				type = TokenType::FloatLiteral;
				Advance();
				while (IsDigit(Current())) {
					Advance();
				}
			}

			if (IsAlpha(Current())) {
				while (IsAlpha(Current()) || IsDigit(Current())) {
					Advance();
				}
				Token bad = MakeToken(TokenType::Error);
				mErrors.push_back({ mStartLine, mStartColumn, "invalid numeric literal" });
				return bad;
			}

			return MakeToken(type);
		}

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

Token Lexer::ScanStringText(bool fromBrace)
{
	while (!atEnd() && Current() != '\n' && Current() != '\r') {
		char ch = Current();

		if (ch == '"') {
			Advance();
			return MakeToken(fromBrace ? TokenType::StringEnd
							 : TokenType::StringLiteral);
		}
		if (ch == '\\') {
			int escLine = mLine, escCol = mColumn;
			Advance();
			if (atEnd() || Current() == '\n' || Current() == '\r') break;
			char e = Current();
			if (e != 'n' && e != 't' && e != 'r' && e != '0' &&
				e != '\\' && e != '"' && e != '$') {
				mErrors.push_back({ escLine, escCol, "invalid escape sequence" });
			}
			Advance();
		}
		else if (ch == '$' && Peek() == '{') {
			Advance();
			Advance();
			mInterpDepth.push_back(0);
			return MakeToken(fromBrace ? TokenType::StringMiddle
							 : TokenType::StringStart);
		}
		else {
			Advance();
		}
	}
	Token tkn = MakeToken(TokenType::Error);
	mErrors.push_back({ mStartLine, mStartColumn, "unterminated string" });
	return tkn;
}

std::vector<Token> Lexer::Tokenize()
{
	RefreshVariables();
	while (true)
	{
		SkipTrivia();
		mStart = mCurrent;
		mStartLine = mLine;
		mStartColumn = mColumn;
		if (atEnd()) {
			if (!mInterpDepth.empty()) {
				mErrors.push_back({ mLine, mColumn, "unterminated interpolation" });
				mInterpDepth.clear();
			}
			mTokens.push_back(MakeToken(TokenType::EndOfFile));
			break;
		}
		mTokens.push_back(scanToken());
	}
	return mTokens;
}