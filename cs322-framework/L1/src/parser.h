
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include <string_view>
#include <array>
#include <vector>
#include <iostream>
#include <regex>

namespace L1
{
	/*
	 * Reserved words
	 */
	namespace RW
	{
		static constexpr std::string_view
			LPAR { "(" },
			RPAR { ")" },
			AT    { "@" },
			COLON { ":" },

			ADD { "+" },
			SUB { "-" },
			INCR { "++" },
			DECR { "--" },
			ASSIGN  { "<-" },
			ADD_EQ  { "+=" },
			SUB_EQ  { "-=" },
			MUL_EQ  { "*=" },
			BAND_EQ { "&=" },
			LSH_EQ  { "<<=" },
			RSH_EQ  { ">>=" },
			LT      { "<" },
			LEQ     { "<=" },
			EQ      { "=" },

			MEM { "mem" },

			CJUMP  { "cjump" },
			GOTO   { "goto" },
			CALL   { "call" },
			RETURN { "return" },

			PRINT { "print" },
			INPUT { "input" },
			ALLOCATE { "allocate" },
			TUPLE_ERROR { "tuple-error" },
			TENSOR_ERROR { "tensor-error" },

			RAX { "rax" },
			RBX { "rbx" },
			RCX { "rcx" },
			RDX { "rdx" },
			RDI { "rdi" },
			RSI { "rsi" },
			RBP { "rbp" },
			RSP { "rsp" },
			R8  { "r8" },
			R9  { "r9" },
			R10 { "r10" },
			R11 { "r11" },
			R12 { "r12" },
			R13 { "r13" },
			R14 { "r14" },
			R15 { "r15" }
			;

		static constexpr int LIBFUNC_CNT { 5 };
		static constexpr std::array< std::string_view, LIBFUNC_CNT >
			LIBFUNCS 
		{
			RW::PRINT, RW::INPUT, RW::ALLOCATE,
			RW::TUPLE_ERROR, RW::TENSOR_ERROR,
		};

		static constexpr int W_REG_CNT { 9 };
		static constexpr std::array< std::string_view, W_REG_CNT >
			W_REGS
		{
			RW::RAX, RW::RBX, RW::RBP,
			RW::R10, RW::R11, RW::R12, RW::R13, RW::R14, RW::R15,
		};

		static constexpr int A_REG_CNT { 5 };
		static constexpr std::array< std::string_view, A_REG_CNT >
			A_REGS
		{
			RW::RDI, RW::RSI, RW::RDX, RW::R8, RW::R9,
		};
	}

	template< typename Handlers... >
	struct NodeVisitor : Handlers...
	{
		using Handlers::operator()...;
	}

	struct LParNode {};
	struct RParNode {};
	struct AtNode {};
	struct ColonNode {};

	struct AddNode {};
	struct SubNode {};
	struct IncrNode {};
	struct DecrNode {};
	struct AssignNode {};
	struct AddEqNode {};
	struct SubEqNode {};
	struct MulEqNode {};
	struct BAndEqNode {};
	struct LShEqNode {};
	struct RShEqNode {};
	struct LTNode {};
	struct LEqNode {};
	struct EqNode {};

	struct MemNode {};
	struct CJumpNode {};
	struct GotoNode {};

	struct CallNode {};
	struct AllocateNode {};
	struct TupleErrorNode {};
	struct TensorErrorNode {};

	struct RaxNode {};
	struct RbxNode {};
	struct RcxNode {};
	struct RdxNode {};
	struct RdiNode {};
	struct RsiNode {};
	struct RbpNode {};
	struct RspNode {};	
	struct R8Node  {};
	struct R9Node  {};
	struct R10Node {};
	struct R11Node {};
	struct R12Node {};
	struct R13Node {};
	struct R14Node {};
	struct R15Node {};

	struct _0Node {};
	struct _1Node {};
	struct _2Node {};
	struct _3Node {};
	struct _4Node {};
	struct _8Node {};

	using ANode = std::variant<
		RdiNode, RsiNode, RdxNode, SxNode, R8Node, R9Node
	>;

	struct IAssignNode { WNode w; AssignNode assign; SNode s; };
	struct ILoadNode { WNode w; AssignNode assign; MemNode mem; XNode x; MNode M; };
	struct IStoreNode { MemNode mem; XNode x; MNode M; AssignNode assign; SNode s; };
	struct IAOpNode { WNode w; AOpNode aop; TNode t; };
	struct ISOpSxNode { WNode w; SOpNode sop; SxNode sx; };
	struct ISOpNNode { WNode w; SOpNode sop; NNode N; };

 	/* 
 	 * @brief
 	 *   Returns whether a character can be used in an identifier (name).
	 */
	bool
	isident( const char c );

 	/* 
 	 * @brief
 	 *   Returns whether a character is a parenthesis.
	 */
	bool
	isparen( const char c );

	class Parser
	{
	private:
		// In-memory buffer for L1 source code file being lexed;
		// null-terminators will be inserted after tokens
		std::string srcbuf {};
		// Token base indices in buffer with null-terminated tokens
		std::vector< int > tok_base_idxs {};

	public:
		Parser( void );

		void
		lex( std::istream &src_is );

		void
		print_toks( void ) const;
	};

}

#endif

