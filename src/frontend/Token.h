#ifndef TOKEN_H
#define TOKEN_H
#include <string_view>

enum class TokenType {
	Plus,
	Minus,
	Star,
	Slash,
	Percent,

	PlusPlus,
	MinusMinus,

	PlusEqual,
	MinusEqual,
	StarEqual,
	SlashEqual,

	Equal,
	EqualEqual,
	//Bang,
	BangEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,

	LParen,
	RParen,
	LBrace,
	RBrace,
	LBracket,
	RBracket,

	Comma,
	Colon,
	Semicolon,
	Arrow,
	Dot,
	DotDot,

	LineComment,
	BlockComment,

	EndOfFile,
	Error,

	Identifier,

	IntLiteral,
	FloatLiteral,
	StringLiteral,

	KwLet,
	KwVar,
	KwFn,
	KwIf,
	KwElse,
	KwFor,
	KwWhile,
	KwIn,
	KwBreak,
	KwContinue,
	KwReturn,
	KwImport,

	KwInt,
	KwFloat,
	KwBool,
	KwString,
	KwVoid,

	KwTrue,
	KwFalse,

	KwAnd,
	KwOr,
	KwNot
};

struct Token
{
	TokenType type;
	std::string_view lexeme;
	int line;
	int column;
};

inline const char* TokenTypeName(TokenType type) {
	switch (type) {
	case TokenType::Plus:		   return "Plus";
	case TokenType::Minus:         return "Minus";
	case TokenType::Star:          return "Star";
	case TokenType::Slash:         return "Slash";
	case TokenType::Percent:       return "Percent";

	case TokenType::PlusPlus:      return "PlusPlus";
	case TokenType::MinusMinus:    return "MinusMinus";

	case TokenType::PlusEqual:     return "PlusEqual";
	case TokenType::MinusEqual:    return "MinusEqual";
	case TokenType::StarEqual:     return "StarEqual";
	case TokenType::SlashEqual:    return "SlashEqual";


	case TokenType::Equal:         return "Equal";
	case TokenType::EqualEqual:    return "EqualEqual";
	//case TokenType::Bang:          return "Bang";
	case TokenType::BangEqual:     return "BangEqual";
	case TokenType::Less:          return "Less";
	case TokenType::LessEqual:     return "LessEqual";
	case TokenType::Greater:       return "Greater";
	case TokenType::GreaterEqual:  return "GreaterEqual";

	case TokenType::LParen:        return "LParen";
	case TokenType::RParen:        return "RParen";
	case TokenType::LBrace:        return "LBrace";
	case TokenType::RBrace:        return "RBrace";
	case TokenType::LBracket:      return "LBracket";
	case TokenType::RBracket:      return "RBracket";

	case TokenType::Comma:         return "Comma";
	case TokenType::Colon:         return "Colon";
	case TokenType::Semicolon:     return "Semicolon";
	case TokenType::Arrow:         return "Arrow";
	case TokenType::Dot:           return "Dot";
	case TokenType::DotDot:        return "DotDot";

	case TokenType::LineComment:   return "LineComment";
	case TokenType::BlockComment:  return "BlockComment";

	case TokenType::EndOfFile:     return "EndOfFile";
	case TokenType::Error:         return "Error";

	case TokenType::Identifier:      return "Identifier";

	case TokenType::IntLiteral:      return "IntLiteral";
	case TokenType::FloatLiteral:    return "FloatLiteral";
	case TokenType::StringLiteral:   return "StringLiteral";

	case TokenType::KwLet:           return "KwLet";
	case TokenType::KwVar:           return "KwVar";
	case TokenType::KwFn:            return "KwFn";
	case TokenType::KwIf:            return "KwIf";
	case TokenType::KwElse:          return "KwElse";
	case TokenType::KwFor:           return "KwFor";
	case TokenType::KwWhile:         return "KwWhile";
	case TokenType::KwIn:            return "KwIn";
	case TokenType::KwBreak:         return "KwBreak";
	case TokenType::KwContinue:      return "KwContinue";
	case TokenType::KwReturn:        return "KwReturn";
	case TokenType::KwImport:        return "KwImport";

	case TokenType::KwInt:           return "KwInt";
	case TokenType::KwFloat:         return "KwFloat";
	case TokenType::KwBool:          return "KwBool";
	case TokenType::KwString:        return "KwString";
	case TokenType::KwVoid:          return "KwVoid";

	case TokenType::KwTrue:          return "KwTrue";
	case TokenType::KwFalse:         return "KwFalse";

	case TokenType::KwAnd:           return "KwAnd";
	case TokenType::KwOr:            return "KwOr";
	case TokenType::KwNot:           return "KwNot";
	}
	return "Unknown";
}
#endif // !TOKEN_H

