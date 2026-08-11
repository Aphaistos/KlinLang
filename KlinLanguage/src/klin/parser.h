#pragma once

#include "lexer.h"

namespace klin {
	class Parser {
	private:
		Lexer m_Lexer;
		std::vector<Token> m_Tokens;
		unsigned int m_Pos;

		std::vector<Diagnostic> m_Diagnostics;

        Token match_tokn(TokenKind);
        Token next_tokn();
        Token peek(int i);
	public:
        explicit Parser(Lexer lexer);
        explicit Parser(std::string source)
            : Parser(Lexer(source)) { }


		std::unique_ptr<ProgramNode> parse_program();

        /*
		std::unique_ptr<ImportNode> parse_imp();
		std::unique_ptr<ModuleNode> parse_mod();
		std::unique_ptr<SpaceBubbleNode> parse_spacebubble();
		
		STRUCT,             // struct Name { ... }
        FIELD,              // field_name: Type (ou avec bitfield)
        UNION,              // union Name { ... }
		
		std::unique_ptr<ParamNode> parse_param();
		std::unique_ptr<FuncNode> parse_func();
		std::unique_ptr<FuncNode> parse_extern_func();
		
		VAR,                // var x: Type = expr; ou const X: Type = expr;

        // Statements
        BLOCK,              // { ... }
        EXPR,               // ...;
        RETURN,             // -> expr ou ->;
        FOR,                // for i = start -> end { ... }
        WHILE,              // while cond { ... }
        ASM,                // asm { "..." : inputs : ouput }

        // Expressions
        BINEXPR,            // a + b, x == y, a & b, a << b
        UNAEXPR,            // -x, !cond, *ptr
        VOLACCEXPR,         // vol *ptr (accès mémoire non optimisé)
        CAST,               // expr as Type
        RANGE,              // start..end(utilisé pour les slices)
        ASSIGN,             // var = expr, *ptr = expr
        IDTF,
        MACCESS,            // obj.field ou namespace::symbol
        IACCESS,            // arr[i] ou ptr[i]
        CALL,               // func_name(arg1, arg2)

        // Types
        PRMTYPE,            // u8, i32, usize, char, string, bool
        PTRTYPE,            // *T ou *const T
        ARRTYPE,            // [T; N]
        SLCTYPE,            // []T ou []const T

        // Litterals
        INT,                // 42, 0xB8000
        CHRLITT,            // 'A', '\n'
        STRLITT,            // "Hello World"
        ARRLITT,            // [1, 2, 3] ou [0; 256]
        NULL_NODE,          // null
		*/
        std::unique_ptr<NullNode> parse_null();
        std::unique_ptr<NullNode> parse_null();


		inline std::vector<Diagnostic> diagnostics() { return m_Diagnostics; }
		inline void diagnose(const Diagnostic& diagnostic) { m_Diagnostics.push_back(diagnostic); }
	};
}