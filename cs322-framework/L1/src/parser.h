
#ifndef L1_PARSER_H
#define L1_PARSER_H

#include <string_view>

namespace L1
{

	namespace RW
	{
		static constexpr std::string_view
			LPAR { "(" },
			RPAR { "(" },

			LARR { "<-" },
			MEM  { "mem" },
			ADD_EQ { "+=" },
			SUB_EQ { "-=" },
			CJUMP { "cjump" },
			GOTO { "goto" },
			RETURN { "return" },
			CALL { "call" },

			PRINT { "print" },
			INPUT { "input" },
			ALLOCATE { "allocate" },
			TUPLE_ERROR { "tuple-error" },
			TENSOR_ERROR { "tensor-error" },

			INCR { "++" },
			DECR { "--" },
			AT { "@" },

			RAX { "rax" },
			RBX { "rbx" },
			RBP { "rbp" },
			R10 { "r10" },
			R11 { "r11" },
			R12 { "r12" },
			R13 { "r13" },
			R14 { "r14" },
			R15 { "r15" },
			RDI { "rdi" },
			RSI { "rsi" },
			RDX { "rdx" },
			R8  { "r8" },
			R9  { "r9" }
			;
	}

}

#endif

