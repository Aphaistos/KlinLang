#include <klin/lexer.h>
#include <iostream>

int main(int argc, char** argv) {
	std::string source = klin::read_file(argv[1]);
	if (source.empty()) {
		std::cerr << "Impossible d'ouvrir le fichier source : " << argv[1] << std::endl;
		return 1;
	}

	klin::Lexer lexer(std::move(source));
	klin::Token current = lexer.read_token();

	while (current.kind != klin::TokenKind::UNKNOWN &&
		current.kind != klin::TokenKind::E_O_F &&
		lexer.diagnostics().empty()) {

		std::cout << "(" << static_cast<int>(current.kind)
			<< ", " << current.start.line << ":" << current.start.column
			<< ", \"" << current.val << "\")\n";

		current = lexer.read_token();
	}

	for (const klin::Diagnostic& d : lexer.diagnostics()) {
		klin::print_diagnostic(argv[1], d);
	}

	return 0;
}