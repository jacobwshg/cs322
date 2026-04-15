
#ifndef L2_LIVENESS_H
#define L2_LIVENESS_H

#include "ast.h"
#include "svutil.h"
#include <variant>
#include <vector>
#include <unordered_map>

#include <string_view>
#include <concepts>

namespace L2
{

	using var_id_t = int;
	using instr_id_t = int;

	static constexpr var_id_t VAR_ID_INVAL { -1 };

	struct LivenessGPRId
	{
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
	};

	template< typename Node >
	concept IsGPR = ( ( LivenessGPRId::val< Node > ) > 0 );

	struct LivenessVisitor
	{
		// smallest id for named, non-GPR variables
		static constexpr var_id_t BASE_VAR_ID { 16 };
		var_id_t next_var_id { BASE_VAR_ID };

		std::vector< std::string > id_var_mp {};
		std::unordered_map<
			std::string, var_id_t,
			//std::hash< std::string_view >,
			//std::equal_to< std::string_view >
			SVUtil::TransparentHash, SVUtil::TransparentEq
		> var_id_mp {};

		LivenessVisitor( void );

		template< typename GPRNode > requires IsGPR< GPRNode >
		var_id_t operator()( const GPRNode &gpr_n )
		{
			return LivenessGPRId::val< GPRNode >;
		}

		var_id_t operator()( const nameNode &name_n );

		var_id_t operator()( const varNode &var_n );

		std::string_view var_by_id( const var_id_t var_id );

	};
}

#endif

