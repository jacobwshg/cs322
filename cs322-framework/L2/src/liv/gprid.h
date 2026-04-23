
#ifndef L2_LIV_GPRID_H
#define L2_LIV_GPRID_H

#include "ints.h"
#include "../ast.h"
#include <array>
#include <string_view>

namespace L2
{
	namespace Liv
	{
		static constexpr var_id_t GPR_ID_DUMMY { 0 };

		struct GPRId
		{
			//
			// use these compile-time expanded values in lieu of a hashtable
			// for faster runtime access
			//
			template < typename Node > static inline constexpr var_id_t val { VAR_ID_INVAL };
			template<> inline constexpr var_id_t val< RaxNode > { 1 };
			template<> inline constexpr var_id_t val< RbxNode > { 2 };
			template<> inline constexpr var_id_t val< RcxNode > { 3 };
			template<> inline constexpr var_id_t val< RdxNode > { 4 };
			template<> inline constexpr var_id_t val< RdiNode > { 5 };
			template<> inline constexpr var_id_t val< RsiNode > { 6 };
			template<> inline constexpr var_id_t val< RbpNode > { 7 };
			// rsp does not participate in liveness
			template<> inline constexpr var_id_t val< R8Node >  { 8 };	
			template<> inline constexpr var_id_t val< R9Node >  { 9 };	
			template<> inline constexpr var_id_t val< R10Node > { 10 };	
			template<> inline constexpr var_id_t val< R11Node > { 11 };	
			template<> inline constexpr var_id_t val< R12Node > { 12 };	
			template<> inline constexpr var_id_t val< R13Node > { 13 };	
			template<> inline constexpr var_id_t val< R14Node > { 14 };	
			template<> inline constexpr var_id_t val< R15Node > { 15 };	

			/*
			static inline const std::unordered_map< std::string, var_id_t >
				GPR_ID_TBL
			{
				{ std::string { KW::RAX }, 1, },
				{ std::string { KW::RBX }, 2, },
				{ std::string { KW::RCX }, 3, },
				{ std::string { KW::RDX }, 4, },

				{ std::string { KW::RDI }, 5, },
				{ std::string { KW::RSI }, 6, },
				{ std::string { KW::RBP }, 7, },

				{ std::string { KW::R8  }, 8,  },
				{ std::string { KW::R9  }, 9,  },
				{ std::string { KW::R10 }, 10, },
				{ std::string { KW::R11 }, 11, },
				{ std::string { KW::R12 }, 12, },
				{ std::string { KW::R13 }, 13, },
				{ std::string { KW::R14 }, 14, },
				{ std::string { KW::R15 }, 15, },
			};
			*/

		};

		static constexpr var_id_t
			MIN_GPR_ID { GPRId::val< RaxNode > }, 
			MAX_GPR_ID { GPRId::val< R15Node > };

		static inline constexpr std::array< std::string_view, 16 >
			ID_GPR_TBL
		{
			L2::EMPTYTOK,
			KW::RAX, KW::RBX, KW::RCX, KW::RDX,
			KW::RDI, KW::RSI, KW::RBP,
			KW::R8,  KW::R9,  KW::R10, KW::R11,
			KW::R12, KW::R13, KW::R14, KW::R15,
		};
	}

}

#endif

