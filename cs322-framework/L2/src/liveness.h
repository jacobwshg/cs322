
#ifndef L2_LIVENESS_H
#define L2_LIVENESS_H

#include "ast.h"
#include "svutil.h"
#include <variant>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <concepts>
#include <cstdint>

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

	struct VarVisitor
	{
		// smallest id for named, non-GPR variables
		static constexpr var_id_t BASE_VAR_ID { 16 };
		var_id_t next_var_id { BASE_VAR_ID };

		//
		// we don't initialize these tables with default mappings between GPRs and their IDs
		// because for the analysis of each new function, the visitor will:
		// - be cleared, in which case the default mappings will be lost;
		// - or be reinstantiated, in which case the overhead of instantiating default mappings
		//   is incurred again.
		// 
		std::vector< std::string > id_var_tbl {};
		std::unordered_map<
			std::string, var_id_t,
			// enable indexing by std::string_view
			SVUtil::TransparentHash, SVUtil::TransparentEq
		> var_id_tbl {};

		VarVisitor( void );

		// given ID, retrieve variable name 
		std::string_view var_by_id( const var_id_t var_id );

		// retrieve variable ID of GPR
		template< typename GPRNode > requires IsGPR< GPRNode >
		var_id_t operator()( const GPRNode &gpr_n )
		{
			return LivenessGPRId::val< GPRNode >;
		}

		// retrieve variable ID of named variable, creating one if necessary
		var_id_t operator()( const nameNode &name_n );

		// retrieve variable ID of the name wrapped in a varNode
		var_id_t operator()( const varNode &var_n );

		// recurse on variants ( such as aNode encountered when vising wNode )
		template< typename VariantNode > requires ::is_variant_v< VariantNode >
		var_id_t operator()( const VariantNode &variant_n )
		{
			return std::visit( *this, variant_n );
		}

		// default, for nodes that don't express variables
		template< typename Node >
		var_id_t operator()( const Node &n )
		{
			return VAR_ID_INVAL;
		}

	};

	struct VarIdSet
	{
		std::vector< std::uint64_t > data {};

		VarIdSet &operator|=( const VarIdSet &that );
		VarIdSet &operator&=( const VarIdSet &that );
		VarIdSet &operator-=( const VarIdSet &that );

		VarIdSet operator|( const VarIdSet &that ) const;
		VarIdSet operator&( const VarIdSet &that ) const;
		VarIdSet operator-( const VarIdSet &that ) const;

	};

}

#endif

