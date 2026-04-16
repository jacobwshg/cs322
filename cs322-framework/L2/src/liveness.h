
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
#include <array>
#include <cstdio>

namespace L2
{

	using var_id_t = int;
	using instr_id_t = int;

	static constexpr var_id_t VAR_ID_INVAL { -1 };

	namespace Liv
	{
		template< typename Node >
		concept IsGPR = (
			( requires { typename Node::is_reg; } )
			&& !std::is_same_v< Node, RspNode >
		);

		struct LivenessGPRId;

		struct VarIdSet;
		VarIdSet operator|( const VarIdSet &, const VarIdSet & );
		VarIdSet operator&( const VarIdSet &, const VarIdSet & );
		VarIdSet operator-( const VarIdSet &, const VarIdSet & );

		struct FuncVarIdSets;

		struct VarVisitor;

		struct LabelVisitor;

		struct InstrVisitor;

		struct FuncVisitor;
	}

	struct Liv::LivenessGPRId
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

		static inline constexpr std::array< std::string_view, 16 >
			ID_GPR_TBL
		{
			L2::EMPTYTOK,
			KW::RAX, KW::RBX, KW::RCX, KW::RDX,
			KW::RDI, KW::RSI, KW::RBP,
			KW::R8,  KW::R9,  KW::R10, KW::R11,
			KW::R12, KW::R13, KW::R14, KW::R15,
		};

	};

	struct Liv::VarIdSet
	{
		std::vector< std::uint64_t > data {};

		VarIdSet &operator|=( const VarIdSet & );
		VarIdSet &operator&=( const VarIdSet & );
		VarIdSet &operator-=( const VarIdSet & );

		//
		// add a var ID to the set
		//
		template< typename I > requires std::integral< I >
		VarIdSet &operator+=( const I i )
		{
			// ignore invalid var IDs
			if ( i <= 0 ) { return *this; }

			// required size ( number of 64-bit blocks ) to reach and accommodate var ID i
			const std::size_t req_sz { static_cast< std::size_t > ( (i + 63) / 64 ) };
			if ( req_sz > this->data.size() )
			{
				this->data.resize( req_sz, 0x0ULL );
			}
			this->data[ i / 64 ] &= ( 0x1ULL << ( i % 64 ) );
			return *this;
		}

		friend VarIdSet operator|( const VarIdSet &, const VarIdSet & );
		friend VarIdSet operator&( const VarIdSet &, const VarIdSet & );
		friend VarIdSet operator-( const VarIdSet &, const VarIdSet & );
	};

	struct Liv::FuncVarIdSets
	{
		std::vector< VarIdSet > gen_sets  {};
		std::vector< VarIdSet > kill_sets {};
		std::vector< VarIdSet > in_sets   {};
		std::vector< VarIdSet > out_sets  {};

		FuncVarIdSets( void ) =default;

		FuncVarIdSets( const std::size_t instr_cnt );

	};

	// visits nodes representing variables ( GPRs or named variables ) in a function
	// and maintains IDs for them in the order of appearance
	struct Liv::VarVisitor
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

		var_id_t new_var_id( void );

		//
		// given ID, retrieve variable name 
		//
		std::string_view var_by_id( const var_id_t var_id ) const;

		//
		// retrieve variable ID of GPR
		//
		template< typename GPRNode > requires IsGPR< GPRNode >
		var_id_t operator()( const GPRNode &gpr_n ) const
		{
			std::printf( "VarVisitor: visiting GPR node\n" );
			return LivenessGPRId::val< GPRNode >;
		}

		// dbg
		var_id_t operator()( const RdxNode &rdx_n ) const
		{
			return LivenessGPRId::val< RdxNode >;
		}

		//
		// retrieve variable ID of named variable, creating one if necessary
		//
		var_id_t operator()( const nameNode &name_n );

		//
		// retrieve variable ID of the name wrapped in a varNode,
		// creating one if necessary
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
			std::printf( "VarVisitor: visiting non-variable node\n" );
			return VAR_ID_INVAL;
		}

	};

	//
	// visits nodes carrying labels and extracts them to help determine successor relations.
	//
	// LabelVisitor is much simpler and doesn't manage label state within a function, because
	// label state is closely coupled with instruction ID, and is thus more conveniently
	// managed by InstrVisitor. if we were to manage it wihtin LabelVisitor, InstrVisitor
	// will have to pass in a current instr ID and whether the instruction is a jump or a pure label.
	//
	struct Liv::LabelVisitor
	{
		std::string_view operator()( const labelNode &label_n );
	};

	// 
	struct Liv::InstrVisitor
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
		VarVisitor var_vis {};

		instr_id_t next_instr_id { 0 };

		FuncVarIdSets var_id_sets {};

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

		InstrVisitor( void ) =default;

		explicit InstrVisitor( const std::size_t );

		// allocate and return new instr ID
		instr_id_t new_instr_id( void );

		//
		// add the serial-execution successor 
		// ( the instr immediately after the current one )
		//
		void add_serial_succ( const instr_id_t );

		//
		// for cjump and goto instructions:
		// if label has been encountered and thus has instr ID, 
		// simply add to own successor IDs; 
		// if label has't been encountered yet,
		// register a request for successor instr ID with 
		// name of label to look for and current instr ID
		//
		void try_add_label_succ( const std::string_view, const instr_id_t );	

		//
		// for pure labels: if the name had been requested as a successor,
		// resolve the request with the current instr ID
		//
		void resolve_label_succ( const instr_id_t, const std::string_view );

		void operator()( const L2::iAssignNode & );
		void operator()( const L2::iLoadNode & );
		void operator()( const L2::iStoreNode & );
		void operator()( const L2::iStackArgNode & );

		void operator()( const L2::iAOpNode & );
		void operator()( const L2::iSxNode & );
		void operator()( const L2::iSOpNode & );

		void operator()( const L2::iAddStoreNode & );
		void operator()( const L2::iSubStoreNode & );
		void operator()( const L2::iLoadAddNode & );
		void operator()( const L2::iLoadSubNode & );

		void operator()( const L2::iCmpAssignNode & );
		void operator()( const L2::iCJumpNode & );

		void operator()( const L2::iLabelNode & );
		void operator()( const L2::iGotoNode & );
		void operator()( const L2::iReturnNode & );

		void operator()( const L2::iCallUNode & );
		void operator()( const L2::iCallPrintNode & );
		void operator()( const L2::iCallInputNode & );
		void operator()( const L2::iCallAllocateNode & );
		void operator()( const L2::iCallTupleErrorNode & );
		void operator()( const L2::iCallTensorErrorNode & );

		void operator()( const L2::iIncrNode & );
		void operator()( const L2::iDecrNode & );

		void operator()( const L2::iLEANode & );
	};
}

#endif

