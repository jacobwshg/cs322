
#ifndef L2_COLOR_H
#define L2_COLOR_H

#include "../liveness.h"

namespace L2
{
	namespace Color
	{
		static constexpr std::size_t GPR_CNT { 15 };
		static constexpr var_id_t
			MIN_GPR_ID { L2::Liv::GPRId::val< RaxNode > }, 
			MAX_GPR_ID { L2::Liv::GPRId::val< R15Node > };

		//std::vector< std::size_t > node_neighbor_cnts {};

		struct CoalescedNode
		{
			std::vector< var_id_t > var_ids;
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

