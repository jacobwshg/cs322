
#ifndef L2_CALLCONV_H
#define L2_CALLCONV_H

#include "ast.h"
#include "liv/var_vis.h"

#include <array>
#include <string_view>

namespace L2
{
	namespace CallConv
	{
		static constexpr std::array< std::string_view, 6 >
			ARG_REGS
		{
			KW::RDI, KW::RSI,
			KW::RDX, KW::RCX,
			KW::R8, KW::R9,
		};

		static constexpr std::array< std::string_view, 9 >
			CALLER_SAVE_REGS
		{
			KW::RAX, KW::RCX, KW::RDX,
			KW::RDI, KW::RSI,
			KW::R8,  KW::R9,  KW::R10, KW::R11, 
		};

		static constexpr std::array< std::string_view, 6 >
			CALLEE_SAVE_REGS
		{
			KW::RBX,
			KW::RBP,
			KW::R12, KW::R13, KW::R14, KW::R15,
		};

		using Liv::GPRId;

		static constexpr var_id_t RETVAL_REG_ID
		{
			GPRId::val< RaxNode >
		};

		static constexpr std::size_t ARG_REG_CNT { 6 };
		static constexpr std::array< var_id_t, ARG_REG_CNT >
			ARG_REG_IDS
		{
			GPRId::val< RdiNode >,
			GPRId::val< RsiNode >,
			GPRId::val< RdxNode >,
			GPRId::val< RcxNode >,
			GPRId::val< R8Node >,
			GPRId::val< R9Node >,
		};

		static constexpr std::size_t CALLER_SAVE_REG_CNT { 9 };
		static constexpr std::array< var_id_t, CALLER_SAVE_REG_CNT >
			CALLER_SAVE_REG_IDS
		{
			GPRId::val< RaxNode >,
			GPRId::val< RcxNode >,
			GPRId::val< RdxNode >,

			GPRId::val< RdiNode >,
			GPRId::val< RsiNode >,

			GPRId::val< R8Node >,
			GPRId::val< R9Node >,
			GPRId::val< R10Node >,
			GPRId::val< R11Node >,
		};

		static constexpr std::size_t CALLEE_SAVE_REG_CNT { 6 };
		static constexpr std::array< var_id_t, CALLEE_SAVE_REG_CNT >
			CALLEE_SAVE_REG_IDS
		{
			GPRId::val< RbxNode >,

			GPRId::val< RbpNode >,

			GPRId::val< R12Node >,
			GPRId::val< R13Node >,
			GPRId::val< R14Node >,
			GPRId::val< R15Node >,
		};

	}
}

#endif

