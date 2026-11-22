#include <iostream>
#include <string_view>
#include "BuildInfo.h"
#include <filesystem>
#include <fstream>
#include "frontend/Lexer.h"
#include <sstream>
#include <vector>

namespace {
	using namespace bytelet::build;
	constexpr int EXITOK = 0;
	constexpr int EXITUSAGE = 64;

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
bool DumpTokens(std::string path) {
	std::filesystem::path _path(path);
	if (!std::filesystem::exists(_path)) {
		std::cerr << "error: path doesn't exist\n\n";
		return false;
	}
	std::ifstream f(_path);
	if (!f) {
		std::cerr << "error: cannot open file\n\n";
		return false;
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
		std::cout << "[" << err.line << ":" << err.column << "] "
			<< "error: " << err.msg << "\n";
	}

	return true;
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

	if (arg.starts_with("-")) {
		std::cerr << "error: unknown option '" << arg << "'\n";
		return EXITUSAGE;
	}

	int iCmd = 2;
	std::string path(arg);
	if (arg == "build") path = std::string(argv[iCmd++]);
	if (argc >= iCmd && std::string_view(argv[iCmd]) == "--tokens") {
		if (DumpTokens(path)) {
			return EXITOK;
		}
		else {
			return EXITUSAGE;
		}
	}

	std::cerr << "error: running scripts is not implemented yet (got '" << arg << ";)\n";
	return EXITUSAGE;
}