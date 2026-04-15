
#ifndef L2_LIVENESS_H
#define L2_LIVENESS_H

#include "ast.h"
#include <variant>
#include <vector>
#include <unordered_map>

#include <string_view>
#include <concepts>

namespace L2
{

	using var_id_t = int;
	using instr_id_t = int;

	namespace LivenessRegId
	{
		template < typename Node > static inline constexpr var_id_t val { -1 };
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
	}

	template< typename Node >
	concept IsGPR = ( ( LivenessRegId::val< Node > ) > 0 );

	struct LivenessVisitor
	{
		std::vector< std::string > id_var_mp {};
		std::unordered_map< std::string, var_id_t > var_id_mp {};

		template< typename GPRNode > requires IsGPR< GPRNode >
		var_id_t operator()( const GPRNode &gpr_n )
		{
			return LivenessRegId::val< GPRNode >;
		}
	};
}

#endif

