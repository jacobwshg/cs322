
#ifndef L2_COLOR_H
#define L2_COLOR_H

#include "../liveness.h"
#include "../ast.h"

namespace L2
{
	namespace Color
	{
		using L2::Liv::MIN_GPR_ID;
		using L2::Liv::MAX_GPR_ID;

		static constexpr std::size_t GPR_CNT { 15 };

		struct CoalescedNode
		{
			std::vector< var_id_t > var_ids;
		};

		//
		// helper visitor for find_moves(): return true if an instruction node
		// represents a move ( iAssignNode )
		//
		struct iAssignFinder
		{
			bool operator()( const iAssignNode &n ) { return true; }
			template< typename iNodeAlt > bool operator()( const iNodeAlt &alt ) { return false; }
		};

		//
		// return degree of a node whose neighbor's IDs are given in nei_id_set,
		// with the max variables ID to consider being max_var_id
		//
		std::size_t
		degree(
			const var_id_t max_var_id,
			const L2::Liv::VarIdSet &nei_id_set
		);

		//
		// return degrees of each node whose neighbor's IDs are given in nei_id_sets,
		// with the max variables ID to consider being max_var_id.
		//
		// called prior to attempting to coalesce
		//
		std::vector< std::size_t >
		count_degrees(
			const var_id_t max_var_id,
			const std::vector< L2::Liv::VarIdSet > &nei_id_sets
		);

		//
		// find pairs of variables related through move ( assign ) instructions
		//
		std::vector< L2::Liv::VarIdSet >
		find_moves(
			const L2::var_id_t max_var_id,
			const std::vector< L2::iNode > &i_ns,
			const std::vector< L2::Liv::VarIdSet > &gen_sets,
			const std::vector< L2::Liv::VarIdSet > &kill_sets
		);

		//
		// given degrees per node ( precomputed via count_degrees() ) 
		// and the neighbor ID sets of two nodes,
		// determine whether they can coalesce according to Briggs' criterion
		//
		bool can_coalesce_briggs(
			const var_id_t max_var_id,
			const std::vector< std::size_t > &degrees,
			const L2::Liv::VarIdSet &nei_id_set_1,
			const L2::Liv::VarIdSet &nei_id_set_2
		);

		//
		// given degrees per node ( precomputed via count_degrees() ) 
		// and the neighbor ID sets of two nodes,
		// determine whether they can coalesce according to George's criterion
		//
		bool can_coalesce_george(
			const var_id_t max_var_id,
			const std::vector< std::size_t > &degrees,
			const L2::Liv::VarIdSet &nei_id_set_1,
			const L2::Liv::VarIdSet
 &nei_id_set_2
		);

	}
}

#endif

