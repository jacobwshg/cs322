
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
	};

	template< typename Node >
	concept IsGPR = ( ( LivenessGPRId::val< Node > ) > 0 );

	// visits nodes representing variables ( GPRs or named variables ) in a function
	// and maintains IDs for them in the order of appearance
	struct VarVisitor
	{
		//
		// smallest id for named, non-GPR variables
		//
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

		//
		// given ID, retrieve variable name 
		//
		std::string_view var_by_id( const var_id_t var_id );

		//
		// retrieve variable ID of GPR
		//
		template< typename GPRNode > requires IsGPR< GPRNode >
		var_id_t operator()( const GPRNode &gpr_n )
		{
			return LivenessGPRId::val< GPRNode >;
		}

		//
		// retrieve variable ID of named variable, creating one if necessary
		//
		var_id_t operator()( const nameNode &name_n );

		//
		// retrieve variable ID of the name wrapped in a varNode
		//
		var_id_t operator()( const varNode &var_n );

		//
		// recurse on variants ( such as aNode encountered when vising wNode )
		//
		template< typename VariantNode > requires ::is_variant_v< VariantNode >
		var_id_t operator()( const VariantNode &variant_n )
		{
			return std::visit( *this, variant_n );
		}

		//
		// default, for nodes that don't express variables
		// ( for example, NNode may be encountered when descending into tNode )
		// return invalid ID and don't register the tables
		//
		template< typename Node >
		var_id_t operator()( const Node &n )
		{
			return VAR_ID_INVAL;
		}

	};

	//
	// visits nodes carrying labels and extracts them to help determine successor relations
	//
	struct LabelVisitor
	{
		std::string_view operator()( const labelNode &label_n );
	};

	// 
	struct InstrVisitor
	{
		// 
		// delegate variable ID assignment to variable visitor, because we are 
		// dealing with instrs of various shapes and thus various variable slots.
		// if we tried to manage variable IDs in InstrVisitor, the VarVisitor 
		// must be modified to return std::string_views for var names, then
		// for each instruction type, we'd have to manually handle each slot:
		// for w <-s : assign( w ), assign( s )
		// for cjump t1 cmp t2 label: assign( t1 ), assign( t2 )
		//
		//VarVisitor var_vis {};

		instr_id_t next_instr_id { 0 };
		var_id_t next_var_id     { 0 };

		//
		// [ i ] = successor instr IDs of instr with ID i
		//
		std::vector< std::vector< instr_id_t > >
			succs_tbl {};
		//
		// [ label ] = instr ID of label
		//
		std::unordered_map<
			std::string, instr_id_t,
			SVUtil::TransparentHash, SVUtil::TransparentEq
		> label_id_tbl {};
		//
		// [ label ] = IDs of instrs that have requests the label
		// as a successor
		//
		std::unordered_map<
			std::string, std::vector< instr_id_t >,
			SVUtil::TransparentHash, SVUtil::TransparentEq
		> requests_tbl {};

		//
		// for cjump and goto instructions:
		// if label has been encountered and thus has instr ID, 
		// simply add to own successor IDs; 
		// if label has't been encountered yet,
		// register a request for successor instr ID with 
		// name of label to look for and current instr ID
		//
		void try_request_succ( const std::string_view, const instr_id_t );	

		//
		// for pure labels: if the name had been requested as a successor,
		// resolve the request with the current instr ID
		//
		void try_resolve_succ( const instr_id_t, const std::string_view );

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

