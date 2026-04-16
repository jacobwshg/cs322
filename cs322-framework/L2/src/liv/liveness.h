
#ifndef L2_LIVENESS_H
#define L2_LIVENESS_H

#include "../ast.h"
#include "../svutil.h"
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

		struct LivenessGPRId;

		struct LabelVisitor;

		struct InstrVisitor;

		struct FuncVisitor;
	}


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

