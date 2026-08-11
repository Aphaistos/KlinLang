#pragma once

#include <vector>
#include <memory>
#include <string>
#include <utility>

/*
================================================================================
 KLIN PROGRAMMING LANGUAGE SYNTAX SPECIFICATION
================================================================================

Le langage Klin organise son code source au moyen d'importations simples et de blocs d'espaces de nommage explicites. 
L'instruction imp permet de charger un module en une seule passe, garantissant l'absence de rechargements cycliques sans avoir recours à des directives de préprocesseur. 
Les symboles, structures et fonctions sont ensuite regroupés au sein d'un espace de nommage encadré par des blocs à double deux-points.

imp sys::ports;
imp arch::x86::idt;

::vga {
    const BUFFER_ADDR: usize = 0xB8000;
}


La déclaration des fonctions prend en charge aussi bien les liaisons système externes que le code natif. 
Les fonctions externes vers l'assembleur ou le runtime bas niveau sont introduites par _func. 
Pour les fonctions classiques, la valeur de retour s'extrait au moyen de l'opérateur flèche ->, qui remplace le mot-clé de retour habituel pour alléger la syntaxe. 
Une forme courte en flèche double => est également disponible pour les expressions monolignes.

_func __sys_outb(port: u16, val: u8);

func add(a: i32, b: i32) -> i32 {
    -> a + b;
}

func hlt() => asm { "hlt" };


Le contrôle de flux repose sur des structures épurées. 
Les boucles for s'inspirent directement de la notation OCaml avec une flèche d'intervalle -> indiquant la borne supérieure, où le type de l'index peut être omis s'il est inféré par le compilateur. 
Les boucles d'attente ou de répétition conditionnelle s'écrivent simplement avec le mot-clé while suivi d'une expression booléenne sans parenthèses obligatoires.

func clear_screen() {
    for i = 0 -> 2000 {
        *(0xB8000 as *u16 + i) = 0;
    }

    while (vol *0x3DA as *u8 & 0x08) == 0 {
        asm { "nop" }
    }
}


La gestion des variables distingue les données mutables déclarées par var des constantes. 
La manipulation directe de la mémoire matérielle s'appuie sur des pointeurs bruts *T et sur l'utilisation du mot-clé d'accès vol. 
Placé directement devant une opération d'accès mémoire, vol garantit que le compilateur ne supprimera ni ne réordonnera les lectures ou écritures effectuées sur les registres matériels (MMIO).

var timer_ticks: u64 = 0;
const MMIO_REG: *u32 = 0xF0000000 as *u32;

func write_hardware_reg(valeur: u32) {
    vol *MMIO_REG = valeur;
    var status: u32 = vol *MMIO_REG;
}


La manipulation de séquences de données s'effectue via des tableaux de taille fixe statiquement alloués ou via des slices []T. 
Un slice agit comme une vue mémoire contiguë de taille dynamique représentée par un pointeur et une longueur. 
Il est possible de créer un slice à partir d'un tableau ou de convertir directement un pointeur brut suivi d'une plage d'éléments vers une vue sécurisée.

var stack_buffer: [u8; 256] = [0; 256];
var view: []u8 = stack_buffer[10..20];

func get_framebuffer() -> []u16 {
    var raw_ptr: *u16 = 0xB8000 as *u16;
    -> raw_ptr[0..2000];
}


Pour répondre aux exigences du développement bare-metal, la disposition mémoire des structures de données peut être ajustée sans attributs verbeux. 
Un préfixe sous forme de crochets vides [] indique une structure compactée (packed) sans rembourrage, tandis qu'une valeur numérique entre crochets [N] force l'alignement sur une frontière de $N$ octets. 
De plus, Klin intègre la déclaration native de champs de bits directement dans la définition des membres.

struct Point {
    x: i32,
    y: i32,
}

[] struct GdtPtr {
    limit: u16,
    base:  u32,
}

[4096] struct PageTable {
    entries: [u32; 1024],
}

[] struct IdtEntry {
    base_low:  u16,
    selector:  u16,
    zero:      u8,
    gate_type: u8 : 4,
    storage:   u8 : 1,
    dpl:       u8 : 2,
    present:   u8 : 1,
    base_high: u16,
}


*/

/*
Source Code
──> [Analysis / Frontend]
──> IR
──> [Synthesis / Backend]
──> Machine Code
*/


namespace klin {
    struct SourceLoc {
        size_t line;
        size_t column;
    };

    enum TokenKind {
        E_O_F = -1,           // \0

        // Ponctuations
        OBRACE,             // {
        CBRACE,             // }
        OBRACKET,           // [
        CBRACKET,           // ]
        OPARENTH,           // (
        CPARENTH,           // )
        COMMA,              // ,
        SCOLON,             // ;

        // Symbols
        THIN_ARROW,         // ->
        ARROW,              // =>
        COLON,              // :
        DCOLON,             // ::
        DOT,                // .
        DDOT,               // ..
        EQUALS,             // =

        // Operator (Arithmetic symbols)
        PLUS,               // +
        MINUS,              // -
        STAR,               // *
        SLASH,              // /
        PERCENT,            // %
        DEQUALS,            // ==
        IEQUALS,            // !=
        LEQUALS,            // <=
        MEQUALS,            // >=
        LESS,               // <
        MORE,               // >
        EXCLAMATION,        // !
        TILDE,              // ~
        AMPERSAND,          // &
        PIPE,               // |

        // Keywords
        ANDKW,              // and
        ORKW,               // or
        MODKW,              // mod
        IMPKW,              // imp    
        FUNCKW,             // func
        _FUNCKW,            // _func
        VARKW,              // var
        CONSTKW,            // const
        VOLKW,              // vol
        STRUCTKW,           // struct
        FORKW,              // for
        WHILEKW,            // while
        ASKW,               // as
        ASMKW,              // asm
        NULL_KW,            // null

        ID,
        UNKNOWN,

        // Litterals
        DECIMAL,            // ...
        HDECIMAL,           // 0x...
        CHAR,               // '.'
        STRING,             // ".."
    };
    enum SyntaxKind {
        // Declarations
        PROGRAM,
        IMP,                // imp path::to::mod;
        MOD,
        SPACEBUBBLE,        // ::name { ... }
        STRUCT,             // struct Name { ... }
        FIELD,              // field_name: Type (ou avec bitfield)
        UNION,              // union Name { ... }
        FUNC,               // func name(...) -> Type { ... }
        _FUNC,              // _func __name(...);
        PARAM,              // name: Type
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
    };

    struct Token {
        TokenKind kind = TokenKind::UNKNOWN;
        std::string val;
        SourceLoc start;
        SourceLoc end;
    };
    struct SyntaxNode {
        SyntaxKind kind;
        SourceLoc start;
        SourceLoc end;

        explicit SyntaxNode(SyntaxKind k, SourceLoc start, SourceLoc end) : kind(k), start(start), end(end) {}
        virtual ~SyntaxNode() = default;
    };
    struct NullNode : public SyntaxNode {
        Token literal;

        NullNode(Token literal, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::NULL_NODE, start, end),
            literal(literal) { }
    };
    struct ArrayNode : public SyntaxNode {
        std::vector<std::unique_ptr<SyntaxNode>> elements;
        std::unique_ptr<SyntaxNode> repeat_value; // Valeur d'initialisation pour [val; N]
        std::unique_ptr<SyntaxNode> repeat_count; // Taille N pour [val; N]

        ArrayNode(std::vector<std::unique_ptr<SyntaxNode>> elements, std::unique_ptr<SyntaxNode> repeat_value, std::unique_ptr<SyntaxNode> repeat_count, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::ARRLITT, start, end),
            elements(std::move(elements)),
            repeat_value(std::move(repeat_value)),
            repeat_count(std::move(repeat_count)) { }
    };
    struct StringNode : public SyntaxNode {
        Token literal;
        std::string value;

        StringNode(Token literal, std::string value, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::STRLITT, start, end),
            literal(literal),
            value(std::move(value)) { }
    };
    struct CharNode : public SyntaxNode {
        Token literal;
        char32_t value;

        CharNode(Token literal, char32_t value, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::CHRLITT, start, end),
            literal(literal),
            value(value) { }
    };
    struct IntNode : public SyntaxNode {
        Token literal;
        uint64_t value;

        IntNode(Token literal, uint64_t value, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::INT, start, end),
            literal(literal),
            value(value) { }
    };
    struct TypeNode : public SyntaxNode {
        std::string type;

        TypeNode(std::string type, SyntaxKind kind, SourceLoc start, SourceLoc end)
            : SyntaxNode(kind, start, end),
        type(type) { }
    };
    struct SliceTypeNode : TypeNode {
        bool is_const;

        SliceTypeNode(bool is_const, std::string type, SourceLoc start, SourceLoc end)
            : TypeNode(type, SyntaxKind::SLCTYPE, start, end),
            is_const(is_const) { }
    };
    struct ArrayTypeNode : TypeNode {
        size_t length;

        ArrayTypeNode(size_t length, std::string type, SourceLoc start, SourceLoc end)
            : TypeNode(type, SyntaxKind::ARRTYPE, start, end),
            length(length) { }
    };
    struct PointerTypeNode : TypeNode {
        bool is_const;

        PointerTypeNode(bool is_const, std::string type, SourceLoc start, SourceLoc end)
            : TypeNode(type, SyntaxKind::PTRTYPE, start, end),
        is_const(is_const) { }
    };
    struct PrimitiveTypeNode : TypeNode {
        PrimitiveTypeNode(std::string type, SourceLoc start, SourceLoc end)
            : TypeNode(type, SyntaxKind::PRMTYPE, start, end) { }
    };
    struct CallNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> called;
        std::vector<std::unique_ptr<SyntaxNode>> args;

        CallNode(std::unique_ptr<SyntaxNode> called, std::vector<std::unique_ptr<SyntaxNode>> args, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::CALL, start, end),
            called(std::move(called)),
            args(std::move(args)) { }
    };
    struct IndexAccessNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> var;
        Token op;
        std::unique_ptr<SyntaxNode> right;

        IndexAccessNode(std::unique_ptr<SyntaxNode> var, Token op, std::unique_ptr<SyntaxNode> right, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::IACCESS, start, end),
            var(std::move(var)),
            op(op),
            right(std::move(right)) { }
    };
    struct MemberAccessNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> target;
        Token op;
        std::string member;

        MemberAccessNode(std::unique_ptr<SyntaxNode> target, Token op, std::string member, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::MACCESS, start, end),
            target(std::move(target)),
            op(op),
            member(std::move(member)) { }

        bool is_namespace_access() const { return op.kind == TokenKind::DCOLON; }
    };
    struct IdentifierNode : public SyntaxNode {
        std::string id;

        IdentifierNode(std::string id, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::IDTF, start, end),
            id(std::move(id)) { }
    };
    struct AssignNode : public SyntaxNode {
        bool is_dereferenced;
        std::unique_ptr<SyntaxNode> var;
        Token equals;
        std::unique_ptr<SyntaxNode> expr;

        AssignNode(bool is_dereferenced, std::unique_ptr<SyntaxNode> var, Token equals, std::unique_ptr<SyntaxNode> expr, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::ASSIGN, start, end),
            is_dereferenced(is_dereferenced),
            var(std::move(var)),
            equals(equals),
            expr(std::move(expr)) {
        }
    };
    struct RangeNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> left;
        Token ddot;
        std::unique_ptr<SyntaxNode> right;

        RangeNode(std::unique_ptr<SyntaxNode> left, Token ddot, std::unique_ptr<SyntaxNode> right, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::RANGE, start, end),
            left(std::move(left)),
            ddot(ddot),
            right(std::move(right)) {
        }
    };
    struct CastNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> expr;
        Token as;
        std::unique_ptr<TypeNode> type;

        CastNode(std::unique_ptr<SyntaxNode> expr, Token as, std::unique_ptr<TypeNode> type, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::CAST, start, end),
            expr(std::move(expr)),
            as(as),
            type(std::move(type)) {}
    };
    struct VolAccessNode : public SyntaxNode {
        Token vol;
        Token op;
        std::unique_ptr<SyntaxNode> expr;

        VolAccessNode(Token vol, Token op, std::unique_ptr<SyntaxNode> expr, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::VOLACCEXPR, start, end),
            vol(vol),
            op(op),
            expr(std::move(expr)) { }
    };
    struct UnaryNode : public SyntaxNode {
        Token op;
        std::unique_ptr<SyntaxNode> expr;

        UnaryNode(Token op, std::unique_ptr<SyntaxNode> expr, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::UNAEXPR, start, end),
            op(op), 
            expr(std::move(expr)) { }
    };
    struct BinaryNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> left;
        Token op;
        std::unique_ptr<SyntaxNode> right;

        BinaryNode(std::unique_ptr<SyntaxNode> left, Token op, std::unique_ptr<SyntaxNode> right, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::BINEXPR, start, end),
            left(std::move(left)),
            op(op),
            right(std::move(right)) { }
    };
    struct AsmOperand {
        std::string constraint;
        std::unique_ptr<SyntaxNode> expr;
    };
    struct AsmNode : public SyntaxNode {
        Token kw;
        std::string instruction;
        std::vector<AsmOperand> outputs;
        std::vector<AsmOperand> inputs; 
        std::vector<std::string> clobbers;     // (ex: "memory", "ax")

        AsmNode(Token kw, std::string instruction, std::vector<AsmOperand> outputs, std::vector<AsmOperand> inputs, std::vector<std::string> clobbers, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::ASM, start, end),
            kw(kw),
            instruction(std::move(instruction)),
            outputs(std::move(outputs)),
            inputs(std::move(inputs)),
            clobbers(std::move(clobbers)) {}
    };
    struct BlockNode : public SyntaxNode {
        std::vector<std::unique_ptr<SyntaxNode>> statements;

        BlockNode(std::vector<std::unique_ptr<SyntaxNode>> statements, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::BLOCK, start, end),
            statements(std::move(statements)) {
        }
    };
    struct WhileNode : public SyntaxNode {
        Token kw;
        std::unique_ptr<SyntaxNode> expr;
        std::unique_ptr<BlockNode> block;

        WhileNode(Token kw, std::unique_ptr<SyntaxNode> expr, std::unique_ptr<BlockNode> block, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::WHILE, start, end),
            kw(kw),
            expr(std::move(expr)),
            block(std::move(block)) {
        }
    };
    struct ForNode : public SyntaxNode {
        Token kw;
        std::string id;
        std::unique_ptr<SyntaxNode> first_expr;
        Token arrow;
        std::unique_ptr<SyntaxNode> last_expr;
        std::unique_ptr<BlockNode> block;

        ForNode(Token kw, std::string id, std::unique_ptr<SyntaxNode> first_expr, Token arrow, std::unique_ptr<SyntaxNode> last_expr, std::unique_ptr<BlockNode> block, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::FOR, start, end),
            kw(kw),
            id(std::move(id)),
            first_expr(std::move(first_expr)),
            arrow(arrow),
            last_expr(std::move(last_expr)),
            block(std::move(block)) { }
    };
    struct ReturnNode : public SyntaxNode {
        Token arrow;
        std::unique_ptr<SyntaxNode> expression;     // nullptr if void return
        Token scolon;

        ReturnNode(Token arrow, std::unique_ptr<SyntaxNode> expression, Token scolon, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::RETURN, start, end),
            arrow(arrow),
            expression(std::move(expression)),
            scolon(scolon) { }

        bool is_void() const { return expression == nullptr; }
    };
    struct ExpressionNode : public SyntaxNode {
        std::unique_ptr<SyntaxNode> expr;
        Token scolon;

        ExpressionNode(std::unique_ptr<SyntaxNode> expr, Token scolon, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::EXPR, start, end),
            expr(std::move(expr)), 
            scolon(scolon) { }
    };
    struct VarNode : public SyntaxNode {
        Token kw;
        std::string name;
        std::unique_ptr<TypeNode> type;
        std::unique_ptr<SyntaxNode> value; // nullptr

        VarNode(Token kw, std::string name, std::unique_ptr<TypeNode> type, std::unique_ptr<SyntaxNode> value, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::VAR, start, end),
            kw(kw),
            name(std::move(name)),
            type(std::move(type)),
            value(std::move(value)) { }

        bool is_const() const { return kw.kind == TokenKind::CONSTKW; }
    };
    struct ParamNode : public SyntaxNode {
        std::string name;
        std::unique_ptr<TypeNode> type;

        ParamNode(std::string name, std::unique_ptr<TypeNode> type, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::PARAM, start, end), 
            name(std::move(name)), 
            type(std::move(type)) {}
    };
    struct FuncNode : public SyntaxNode {
        Token func;
        std::string name;
        std::vector<std::unique_ptr<ParamNode>> params;
        std::unique_ptr<TypeNode> return_type;
        std::unique_ptr<SyntaxNode> body;       // BlockNode or Expression (if =>)
        bool is_extern;
        bool is_expression_body;

        FuncNode(Token func, std::string name, std::vector<std::unique_ptr<ParamNode>> params, std::unique_ptr<TypeNode> return_type, std::unique_ptr<SyntaxNode> body, bool is_extern, bool is_expression_body, SourceLoc start, SourceLoc end)
            : SyntaxNode(is_extern ? SyntaxKind::_FUNC : SyntaxKind::FUNC, start, end),
            func(func),
            name(std::move(name)),
            params(std::move(params)),
            return_type(std::move(return_type)),
            body(std::move(body)),
            is_extern(is_extern),
            is_expression_body(is_expression_body) { }
    };
    struct FieldNode : public SyntaxNode {
        std::string name;
        std::unique_ptr<TypeNode> type;
        size_t bit_width; // 0 for standard fields, >0 if bitfield (ex: u8 : 4)

        FieldNode(std::string name, std::unique_ptr<TypeNode> type, size_t bit_width, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::FIELD, start, end),
            name(std::move(name)), 
            type(std::move(type)), 
            bit_width(bit_width) { }
    };
    struct UnionNode : public SyntaxNode {
        Token union_kw;
        std::string name;
        std::vector<std::unique_ptr<FieldNode>> fields;

        UnionNode(Token union_kw, std::string name, std::vector<std::unique_ptr<FieldNode>> fields, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::UNION, start, end), 
            union_kw(union_kw), 
            name(std::move(name)), 
            fields(std::move(fields)) {
        }
    };
    struct StructNode : public SyntaxNode {
        Token struct_kw;
        std::string name;
        std::vector<std::unique_ptr<FieldNode>> fields;
        bool is_packed;             // [] struct A {}
        size_t alignement;          // [N] struct A {}

        StructNode(Token struct_kw, std::string name, std::vector<std::unique_ptr<FieldNode>> fields, bool is_packed, size_t alignement, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::STRUCT, start, end), 
            struct_kw(struct_kw), 
            name(std::move(name)), 
            fields(std::move(fields)), 
            is_packed(is_packed), 
            alignement(alignement) { }
    };
    struct RouteNode {
        Token arrow;
        std::string path;
    };
    struct ModuleNode : public SyntaxNode {
        Token mod;
        Token dcolon;
        std::string name;
        std::vector<RouteNode> routes;
        std::vector<std::unique_ptr<SyntaxNode>> declarations;

        ModuleNode(Token mod, Token dcolon, std::string name, std::vector<RouteNode> routes, std::vector<std::unique_ptr<SyntaxNode>> declarations, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::MOD, start, end),
            mod(mod),
            dcolon(dcolon),
            name(std::move(name)),
            routes(std::move(routes)),
            declarations(std::move(declarations)) {}
    };
    struct SpaceBubbleNode : public SyntaxNode {
        Token dcolon;
        std::string name;
        std::vector<std::unique_ptr<SyntaxNode>> declarations;

        SpaceBubbleNode(Token dcolon, std::string name, std::vector<std::unique_ptr<SyntaxNode>> declarations, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::SPACEBUBBLE, start, end), 
            dcolon(dcolon), 
            name(std::move(name)), 
            declarations(std::move(declarations)) {}
    };
    struct ImportNode : public SyntaxNode {
        Token imp;
        std::vector<std::string> path;      // For std::io we have ["std", "io"]

        ImportNode(Token imp, std::vector<std::string> path, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::IMP, start, end), 
            imp(imp), 
            path(std::move(path)) {}
    };
    struct ProgramNode : public SyntaxNode {
        std::vector<std::unique_ptr<ImportNode>> imports;
        std::vector<std::unique_ptr<SyntaxNode>> declarations;

        ProgramNode(std::vector<std::unique_ptr<ImportNode>> imports, std::vector<std::unique_ptr<SyntaxNode>> declarations, SourceLoc start, SourceLoc end)
            : SyntaxNode(SyntaxKind::PROGRAM, start, end), 
            imports(std::move(imports)), 
            declarations(std::move(declarations)) {}
    };
}