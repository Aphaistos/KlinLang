#include "parser.h"

#define current m_Tokens[m_Pos]

namespace klin {

	Parser::Parser(Lexer lexer)
		: m_Lexer(lexer), m_Pos(0) {
		Token token;
		while ((token = m_Lexer.read_token()).kind != TokenKind::E_O_F)
			m_Tokens.push_back(token);
		m_Tokens.push_back(token); // For the EOF or Bad token

		m_Diagnostics.clear();
		for (Diagnostic diagnostic : m_Lexer.diagnostics())
			m_Diagnostics.push_back(diagnostic);
	}

	std::unique_ptr<ProgramNode> Parser::parse_program() {
		SourceLoc start = current.start;

		std::vector<std::unique_ptr<ImportNode>> imports;
		std::vector<std::unique_ptr<SyntaxNode>> declarations;

		if (current.kind == TokenKind::IMPKW) {
			while (current.kind != TokenKind::E_O_F && current.kind == TokenKind::IMPKW) {
				sd
			}
		}
		while (current.kind != TokenKind::E_O_F) {

		}
		std::make_unique<ProgramNode>(imports, declarations);
	}
}