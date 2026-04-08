
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include <string_view>
#include <array>

namespace L1
{

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
			R15 { "r15" },
			;

		static constexpr int LIBFUNC_CNT { 5 };
		static constexpr std::array< std::string_view, LIBFUNC_CNT > LIBFUNCS 
		{
			PRINT, INPUT, ALLOCATE,
			TUPLE_ERROR, TENSOR_ERROR,
		};

		static constexpr int W_REG_CNT { 9 };
		static constexpr std::array< std::string_view, W_REG_CNT > W_REGS
		{
			RAX, RBX, RBP,
			R10, R11, R12, R13, R14, R15,
		};

		static constexpr int A_REG_CNT { 5 };
		static constexpr std::array< std::string_view, A_REG_CNT > A_REGS
		{
			RDI, RSI, RDX, R8, R9,
		};
	}
}

#endif

