#include <iostream>
#include <string_view>
#include "BuildInfo.h"
#include <filesystem>
#include <fstream>
#include "frontend/Lexer.h"
#include <sstream>
#include <vector>
#include "frontend/Parser.h"

namespace {
	using namespace bytelet::build;
	constexpr int EXITOK = 0;
	constexpr int EXITUSAGE = 64;
	constexpr int EXITDATAERR = 65;
	constexpr int EXITNOINPUT = 66;

	void printVersion() {
		std::cout << CLINAME << ' ' << VERSION << "\n";
	}

	void printHelp() {
		std::cout
			<< LANGUAGENAME << ' ' << VERSION << " - a small scripting language\n\n"
			<< "Usage:\n"
			<< "  " << CLINAME << " <script" << FILEEXTENSION << ">                  Run a script (not implemented yet)\n"
			<< "  " << CLINAME << " build <script" << FILEEXTENSION << "> -o <exe>   Compile natively (not implemented yet)\n"
			<< "  " << CLINAME << " --version                     Print version\n"
			<< "  " << CLINAME << " --help                        Print this help\n";
	}
}

[[nodiscard]]
int DumpTokens(const std::string& path) {
	std::ifstream f(path, std::ios::binary);
	if (!f) {
		std::cerr << "error: cannot open '" << path << "'\n";
		return EXITNOINPUT;
	}
	std::stringstream buff;
	buff << f.rdbuf();
	std::string corpus = buff.str();

	Lexer lexer(corpus);
	std::vector<Token> tokens = lexer.Tokenize();

	for (const auto& token : tokens) {
		std::cout << "[" << token.line << ":" << token.column << "] "
			<< token.lexeme << " | " << TokenTypeName(token.type) << "\n";
	}
	for (const auto& err : lexer.errors()) {
		std::cerr << path << ":" << err.line << ":" << err.column
			<< ": error: " << err.msg << "\n";
	}
	return lexer.errors().empty() ? EXITOK : EXITDATAERR;
}

[[nodiscard]]
int PrintAst(const std::string& source) {
	Lexer lexer(source);
	std::vector<Token> tokens = lexer.Tokenize();

	if (!lexer.errors().empty()) {
		for (const auto& err : lexer.errors()) {
			std::cerr << "<expr>:" << err.line << ":" << err.column
				<< ": error: " << err.msg << "\n";
		}
		return EXITDATAERR;
	}

	Parser parser(tokens);
	ExpressionPointer tree = parser.ParseExpression();
	if (!tree) {
		for (const auto& err : parser.Errors()) {
			std::cerr << "<expr>:" << err.line << ":" << err.column
				<< ": error: " << err.msg << "\n";
		}
		return EXITDATAERR;
	}
	std::cout << tree->ToString() << "\n";
	return EXITOK;
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cerr << "error: no input given\n\n";
		printHelp();
		return EXITUSAGE;
	}

	const std::string_view arg = argv[1];
	if (arg == "--version" || arg == "-v") {
		printVersion();
		return EXITOK;
	}
	if (arg == "--help" || arg == "-h") {
		printHelp();
		return EXITOK;
	}
	if (arg == "--expr") {
		if (argc < 3) {
			std::cerr << "error: --expr needs an expression in quotes\n";
			return EXITUSAGE;
		}
		return PrintAst(argv[2]);
	}
	if (arg.starts_with("-")) {
		std::cerr << "error: unknown option '" << arg << "'\n";
		return EXITUSAGE;
	}

	int iCmd = 2;
	std::string path(arg);
	if (arg == "build") {
		if (argc < 3) {
			std::cerr << "error: 'build' needs a script path\n";
			return EXITUSAGE;
		}
		path = argv[2];
		iCmd = 3;
	}
	if (argc > iCmd && std::string_view(argv[iCmd]) == "--tokens") {
		return DumpTokens(path);
	}

	std::cerr << "error: running scripts is not implemented yet (got '" << arg << ";)\n";
	return EXITUSAGE;
}