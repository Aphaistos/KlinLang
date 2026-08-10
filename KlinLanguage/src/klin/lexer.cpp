#include "lexer.h"

#include <unordered_map>
#include <iostream>

#define current m_Source[m_Pos]


namespace klin {
	static const std::unordered_map<std::string, TokenKind> KEYWORDS = {
	{"and", TokenKind::ANDKW},
	{"or", TokenKind::ORKW},
	{"mod", TokenKind::MODKW},
	{"imp", TokenKind::IMPKW},
	{"func", TokenKind::FUNCKW},
	{"_func", TokenKind::_FUNCKW},
	{"var", TokenKind::VARKW},
	{"const", TokenKind::CONSTKW},
	{"vol", TokenKind::VOLKW},
	{"struct", TokenKind::STRUCTKW},
	{"for", TokenKind::FORKW},
	{"while", TokenKind::WHILEKW},
	{"as", TokenKind::ASKW},
	{"asm", TokenKind::ASMKW},
	{"null", TokenKind::NULL_KW},
	};

	unsigned int Lexer::next(int times) { // Return pos in current line
		if (m_Pos + times > m_Source.size() || times < 1)
			return 0;

		int start = m_Column;

		if (current == '\n') {
			m_Line++;
			m_Pos += times;
			m_Column = 0;
			next(times - 1);
			return start;
		}
		else if (current == '\t') {
			m_Pos += times;
			m_Column += 4 * times;
			return start;
		}
		m_Pos += times;
		m_Column += times;
		return start;
	}
	void Lexer::skip_blanks_and_comments() {
		while (m_Pos < m_Source.size()) {
			if (current == ' ' || current == '\r' || current == '\t' || current == '\n') {
				next();
			}
			else if (current == '/') {
				if (m_Pos + 1 < m_Source.size()) {
					char next_char = m_Source[m_Pos + 1];

					if (next_char == '/') { // //
						next(2);
						while (m_Pos < m_Source.size() && current != '\n')
							next();
						continue;
					}
					else if (next_char == '*') { // /* ... */
						SourceLoc comment_start = { m_Line, m_Column };
						next(2);
						while (m_Pos + 1 < m_Source.size() && !(current == '*' && m_Source[m_Pos + 1] == '/'))
							next();
						if (m_Pos + 1 < m_Source.size())
							next(2);
						else {
							diagnose(report_unterminated_block_comment(Token(TokenKind::UNKNOWN, "/*", comment_start, { m_Line, m_Column })));
							m_Pos = m_Source.size();
						}
						continue;
					}
				}
				break;
			}
			else {
				break;
			}
		}
	}
	Token Lexer::read_identifier() {
		std::string str;
		unsigned int pos = m_Column;
		unsigned int line = m_Line;
		while ((std::isalnum(current) || current == '_') && current != '\0') {
			str += current;
			next();
		}

		unsigned int end_pos = m_Column;
		unsigned int end_line = m_Line;

		auto it = KEYWORDS.find(str);
		TokenKind kind = (it != KEYWORDS.end()) ? it->second : TokenKind::ID;

		return Token(kind, std::move(str), { line, pos }, { end_line, end_pos });
	}
	char32_t Lexer::read_escape_sequence() {
		if (current == '\0') {
			diagnose(report_invalid_escape_sequence(m_Line, m_Column, 1));
			return U'\0';
		}

		char ch = current;
		unsigned int start_pos = m_Column;
		next();
		switch (ch) {
		case 'n':  return U'\n';
		case 'r':  return U'\r';
		case 't':  return U'\t';
		case '\\': return U'\\';
		case '\'': return U'\'';
		case '"':  return U'"';
		case '0':  return U'\0';
		case 'x': { // Échappement hexadécimal à 2 octets (\x41)
			std::string hex_digits;
			for (int i = 0; i < 2; ++i) {
				if (current == '\0' || !std::isxdigit(current)) {
					diagnose(report_invalid_escape_sequence(m_Line, start_pos, m_Column - start_pos + 1));
					return U'\0';
				}
				hex_digits += current;
				next();
			}
			return static_cast<char32_t>(std::stoul(hex_digits, nullptr, 16));
		}
		default:
			diagnose(report_invalid_escape_sequence(m_Line, start_pos, 2));
			return U'\0';
		}
	}
	Token Lexer::read_string() {
		SourceLoc start_loc = { m_Line, m_Column };
		next();

		std::string value;
		while (current != '\0' && current != '"') {
			if (current == '\n') {
				diagnose(report_unterminated_string(Token(TokenKind::STRING, value, start_loc, { m_Line, m_Column })));
				break;
			}

			if (current == '\\') {
				next();
				char32_t escaped = read_escape_sequence();
				// Encodage manuel UTF-8 simple pour la valeur de chaîne
				if (escaped <= 0x7F) {
					value += static_cast<char>(escaped);
				}
				else {
					// Pour simplifier, insertion directe (étendre pour UTF-8 complet si besoin)
					value += static_cast<char>(escaped & 0xFF);
				}
			}
			else {
				value += current;
				next();
			}
		}

		if (current == '\0') {
			diagnose(report_unterminated_string(Token(TokenKind::STRING, value, start_loc, { m_Line, m_Column })));
		}
		else {
			next();
		}

		SourceLoc end_loc = { m_Line, m_Column };

		return Token(TokenKind::STRING, std::move(value), start_loc, end_loc);
	}
	Token Lexer::read_char() {
		SourceLoc start_loc = { m_Line, m_Column };
		next();

		if (current == '\0' || current == '\'') {
			diagnose(report_blank_char(Token(TokenKind::CHAR, "", start_loc, { m_Line, m_Column })));
		}

		char32_t char_value = 0;
		if (current == '\\') {
			next();
			char_value = read_escape_sequence();
		}
		else if (current != '\0' && current != '\'') {
			// Lecture simple d'un caractère (ASCII / UTF-8 direct)
			char_value = static_cast<unsigned char>(current);
			next();
		}

		if (current != '\'') {
			diagnose(report_unterminated_char(Token(TokenKind::CHAR, std::string(1, char_value), start_loc, { m_Line, m_Column })));
		}
		else {
			next();
		}
		SourceLoc end_loc = { m_Line, m_Column };

		return Token(TokenKind::CHAR, std::string(1, char_value), start_loc, end_loc);
	}
	Token Lexer::read_number() {
		SourceLoc start_loc = { m_Line, m_Column };
		int base = 10;
		TokenKind kind = TokenKind::DECIMAL;
		std::string raw_digits;

		if (current == '0') {
			raw_digits += current;
			next();
			if (m_Pos < m_Source.size() && (current == 'x' || current == 'X')) {
				base = 16;
				kind = TokenKind::HDECIMAL;
				raw_digits.clear();
				next();
			}
		}

		while (m_Pos < m_Source.size()) {
			char c = current;

			if (c == '_') {
				next();
				continue;
			}

			bool valid_digit = false;
			if (base == 10 && std::isdigit(c)) valid_digit = true;
			else if (base == 16 && std::isxdigit(c)) valid_digit = true;

			if (!valid_digit) break;

			raw_digits += current;
			next();
		}

		if (raw_digits.empty() && base == 16) {
			diagnose(report_hexnodigit(Token(kind, std::move(raw_digits), start_loc, { m_Line, m_Column })));
		}

		SourceLoc end_loc = { m_Line, m_Column };
		return Token(kind, std::move(raw_digits), start_loc, end_loc);
	}

	Token Lexer::read_token() {
		skip_blanks_and_comments();

		if (m_Pos >= m_Source.size()) {
			return Token(TokenKind::E_O_F, "\0", { m_Line, m_Column }, { m_Line, m_Column });
		}

		if (std::isalpha(current) || current == '_')
			return read_identifier();
		else if (std::isdigit(current))
			return read_number();
		else if (current == '"')
			return read_string();
		else if (current == '\'')
			return read_char();

		unsigned int line = m_Line;
		unsigned int pos = m_Column;

		switch (current) {
		case '\0': return Token(TokenKind::E_O_F, "\0", { m_Line, next() }, { m_Line, m_Column });
		case '{': return Token(TokenKind::OBRACE, "{", { m_Line, next() }, { m_Line, m_Column });
		case '}': return Token(TokenKind::CBRACE, "}", { m_Line, next() }, { m_Line, m_Column });
		case '[': return Token(TokenKind::OBRACKET, "[", { m_Line, next() }, { m_Line, m_Column });
		case ']': return Token(TokenKind::CBRACKET, "]", { m_Line, next() }, { m_Line, m_Column });
		case '(': return Token(TokenKind::OPARENTH, "(", { m_Line, next() }, { m_Line, m_Column });
		case ')': return Token(TokenKind::CPARENTH, ")", { m_Line, next() }, { m_Line, m_Column });
		case ',': return Token(TokenKind::COMMA, ",", { m_Line, next() }, { m_Line, m_Column });
		case ';': return Token(TokenKind::SCOLON, ";", { m_Line, next() }, { m_Line, m_Column });

		case ':': {
			next();
			if (current == ':') {
				next();
				return { TokenKind::DCOLON, "::", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::COLON, ":", { line, pos }, { m_Line, m_Column } };
		}
		case '.': {
			next();
			if (current == '.') {
				next();
				return { TokenKind::DDOT, "..", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::DOT, ".", { line, pos }, { m_Line, m_Column } };
		}
		case '+': next(); return Token(TokenKind::PLUS, "+", { line, pos }, { m_Line, m_Column });
		case '-': {
			next();
			if (current == '>') {
				next();
				return { TokenKind::ARROW, "->", { line, pos }, { m_Line, m_Column } }; // Corrigé : ARROW au lieu de DDOT
			}
			return { TokenKind::MINUS, "-", { line, pos }, { m_Line, m_Column } };
		}
		case '*': next(); return Token(TokenKind::STAR, "*", { line, pos }, { m_Line, m_Column });
		case '/': next(); return Token(TokenKind::SLASH, "/", { line, pos }, { m_Line, m_Column });
		case '%': next(); return Token(TokenKind::PERCENT, "%", { line, pos }, { m_Line, m_Column });
		case '~': return Token(TokenKind::TILDE, "~", { m_Line, next() }, { m_Line, m_Column });
		case '&': return Token(TokenKind::AMPERSAND, "&", { m_Line, next() }, { m_Line, m_Column });
		case '|': return Token(TokenKind::PIPE, "|", { m_Line, next() }, { m_Line, m_Column });

		case '=': {
			next();
			if (current == '>') {
				next();
				return { TokenKind::ARROW, "=>", { line, pos }, { m_Line, m_Column } };
			}
			else if (current == '=') {
				next();
				return { TokenKind::DEQUALS, "==", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::EQUALS, "=", { line, pos }, { m_Line, m_Column } };
		}
		case '!': {
			next();
			if (current == '=') {
				next();
				return { TokenKind::IEQUALS, "!=", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::EXCLAMATION, "!", { line, pos }, { m_Line, m_Column } };
		}
		case '<': {
			next();
			if (current == '=') {
				next();
				return { TokenKind::LEQUALS, "<=", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::LESS, "<", { line, pos }, { m_Line, m_Column } };
		}
		case '>': {
			next();
			if (current == '=') {
				next();
				return { TokenKind::MEQUALS, ">=", { line, pos }, { m_Line, m_Column } };
			}
			return { TokenKind::MORE, ">", { line, pos }, { m_Line, m_Column } };
		}

		default: {
			char err_c = current;
			diagnose(report_unexpected_char(line, pos, err_c));
			next();
			return Token(TokenKind::UNKNOWN, std::string(1, err_c), { line, pos }, { m_Line, m_Column });
		}
		}
	}
}