#pragma once

#include <string>

#include "syntax.h"

namespace klin {
	std::wstring to_wstring(const std::string& str);
	std::string read_file(const char* filepath);
	std::string get_line_file(const char* filepath, unsigned int lineNumber);

	struct Diagnostic {
		unsigned int pos;
		unsigned int line;
		unsigned int length;
		std::string message;

		Diagnostic(Token _token, std::string _message)
			: pos(_token.start.column), line(_token.start.line), length((unsigned int)_token.val.size()), message(_message) {
		}
		Diagnostic(unsigned int _line, unsigned int _pos, unsigned int _length, std::string _message)
			: pos(_pos), line(_line), length(_length), message(_message) {
		}
	};

	void print_diagnostic(const char* filepath, const klin::Diagnostic& diag);

	inline Diagnostic report_blank_char(Token token) {
		return { token, "Empty character literal." };
	}
	inline Diagnostic report_multi_char(Token token) {
		return { token, "Multi-character literal is not allowed." };
	}
	inline Diagnostic report_unterminated_char(Token token) {
		return { token, "Unterminated character literal." };
	}

	inline Diagnostic report_unterminated_string(Token token) {
		return { token, "Unterminated string literal." };
	}
	inline Diagnostic report_invalid_escape_sequence(unsigned int line, unsigned int pos, unsigned int length) {
		return { line, pos, length, "Invalid escape sequence in literal." };
	}

	inline Diagnostic report_hexnodigit(Token token) {
		return { token, "Invalid hexadecimal literal: expected digits after '0x'." };
	}
	inline Diagnostic report_binnodigit(Token token) {
		return { token, "Invalid binary literal: expected digits after '0b'." };
	}
	inline Diagnostic report_invalid_number(Token token) {
		return { token, "Invalid numeric literal format." };
	}

	inline Diagnostic report_unterminated_block_comment(Token token) {
		return { token, "Unterminated block comment '/*'." };
	}

	inline Diagnostic report_unexpected_char(unsigned int line, unsigned int pos, char c) {
		std::string msg = "Unexpected character '";
		msg += c;
		msg += "'.";
		return { line, pos, 1, msg };
	}
	inline Diagnostic report_unknown_token(Token token) {
		return { token, "Unrecognized token sequence." };
	}
}