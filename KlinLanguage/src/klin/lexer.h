#pragma once

#include "syntax.h"
#include "io.h"

namespace klin {
	class Lexer {
	private:
		std::string m_Source;
		size_t m_Pos;
		size_t m_Line;
		size_t m_Column;

		std::vector<Diagnostic> m_Diagnostics;

		char32_t read_escape_sequence();
	public:
		Lexer(std::string source)
			: m_Source(std::move(source)), m_Pos(0), m_Line(1), m_Column(1) { }

		unsigned int next(int times = 1);

		void skip_blanks_and_comments();

		Token read_identifier();
		Token read_string();
		Token read_char();
		Token read_number();
		Token read_token();

		inline const std::vector<Diagnostic>& diagnostics() const { return m_Diagnostics; }
		inline void diagnose(const Diagnostic& diagnostic) { m_Diagnostics.push_back(diagnostic); }
	};
}