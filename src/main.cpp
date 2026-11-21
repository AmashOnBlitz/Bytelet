#include <iostream>
#include <string_view>
#include "BuildInfo.h"

namespace {
	using namespace bytelet::build;
	constexpr int iExitOK = 0;
	constexpr int iExitUsage = 64;

	void printVersion() {
		std::cout << kCliName << ' ' << kVersion << "\n";
	}

	void printHelp() {
		std::cout
			<< kLanguageName << ' ' << kVersion << " - a small scripting language\n\n"
			<< "Usage:\n"
			<< "  " << kCliName << " <script" << kFileExtension << ">                Run a script (not implemented yet)\n"
			<< "  " << kCliName << " build <script" << kFileExtension << "> -o <exe>   Compile natively (not implemented yet)\n"
			<< "  " << kCliName << " --version                     Print version\n"
			<< "  " << kCliName << " --help                        Print this help\n";
	}
}

int main(char argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "error: no input given\n\n";
		printHelp();
		return iExitUsage;
	}

	const std::string_view arg = argv[1];
	if (arg == "--version" || arg == "-v") {
		printVersion();
		return iExitOK;
	}
	if (arg == "--help" || arg == "-h") {
		printHelp();
		return iExitOK;
	}

	if (arg.starts_with("-")) {
		std::cerr << "error: unknown option '" << arg << "'\n";
		return iExitUsage;
	}

	std::cerr << "error: running scripts is not implemented yet (got '" << arg << ";)\n";
	return iExitUsage;
}