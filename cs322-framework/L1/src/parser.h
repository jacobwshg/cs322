
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include <string_view>
#include <array>
#include <vector>
#include <iostream>

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
			COLON { ":" }

			MEM { "mem" },

			ADD { "+" },
			SUB { "-" },
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

			CJUMP  { "cjump" },
			GOTO   { "goto" },
			CALL   { "call" },
			RETURN { "return" },

			PRINT { "print" },
			INPUT { "input" },
			ALLOCATE { "allocate" },
			TUPLE_ERROR { "tuple-error" },
			TENSOR_ERROR { "tensor-error" },

			INCR { "++" },
			DECR { "--" },

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
			PRINT, INPUT, ALLOCATE,
			TUPLE_ERROR, TENSOR_ERROR,
		};

		static constexpr int W_REG_CNT { 9 };
		static constexpr std::array< std::string_view, W_REG_CNT >
			W_REGS
		{
			RAX, RBX, RBP,
			R10, R11, R12, R13, R14, R15,
		};

		static constexpr int A_REG_CNT { 5 };
		static constexpr std::array< std::string_view, A_REG_CNT >
			A_REGS
		{
			RDI, RSI, RDX, R8, R9,
		};
	}

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
	public:
		// In-memory buffer for L1 source code file being lexed;
		// null-terminators will be inserted after tokens
		std::string srcbuf {};
		// Token base indices in buffer with null-terminated tokens
		std::vector< int > tok_base_idxs {};

		Parser( void );

		void
		lex( std::istream &src_is );
	};


}

#endif

