
#include "color.h"
#include "../liveness.h"
#include <cstddef>
#include <vector>
#include <algorithm>

std::size_t
L2::Color::
degree(
	const L2::var_id_t max_var_id,
	const L2::Liv::VarIdSet &nei_id_set
)
{
	std::size_t deg { 0 };

	for (
		L2::var_id_t nei_id { MIN_GPR_ID };
		nei_id < max_var_id;
		++nei_id
	)
	{
		if ( nei_id_set.has( nei_id ) )
		{
			++deg;
		}
	}

	return deg;
}

std::vector< std::size_t >
L2::Color::count_degrees(
	const L2::var_id_t max_var_id,
	const std::vector< L2::Liv::VarIdSet > &nei_id_sets
)
{
	std::vector< std::size_t > degrees {};
	degrees.reserve( nei_id_sets.size() );

	//
	// count and append degrees based on each set of neighbor IDs
	//
	std::for_each(
		nei_id_sets.begin(), nei_id_sets.end(),
		[ max_var_id, &degrees ]( const L2::Liv::VarIdSet &nei_id_set )
		{
			degrees.emplace_back( degree( max_var_id, nei_id_set ) );
		}
	);

	return std::move( degrees );

}

/*
 *
 * Briggs' criterion
 *
 * Nodes a and b can be coalesced if the resulting node ab
 * will have fewer than K neighbors of degree >= K
 * - K = Number of general purpose registers
 *
 * This coalescing is guaranteed not to turn a K-colorable graph
 * into a non-K-colorable graph
 *
 */
bool
L2::Color::
can_coalesce_briggs(
	const L2::var_id_t max_var_id,
	const std::vector< std::size_t > &degrees,
	const L2::Liv::VarIdSet &nei_id_set_1,
	const L2::Liv::VarIdSet &nei_id_set_2
)
{
	//
	// number of neighbors with degree >= GPR_CNT
	//
	std::size_t high_deg_nei_cnt { 0UL };

	//
	// hypothetical neighbor ID set after coalescing
	//
	const L2::Liv::VarIdSet coal_nei_id_set { nei_id_set_1 | nei_id_set_2 };

	//
	// loop through all neighbors IDs in the coalesced set
	// and check their degrees
	//
	for (
		L2::var_id_t id { MIN_GPR_ID };
		id < max_var_id;
		++id
	)
	{
		// 
		// skip IDs that are not neighbors of the coalesced node
		//
		if ( !coal_nei_id_set.has( id ) ) { continue; }
		// 
		// check neighbor degree
		//
		if ( degrees[ id ] >= GPR_CNT ) { ++high_deg_nei_cnt; }
		//
		// fail if too many neighbors have high degree
		//
		if ( high_deg_nei_cnt >= GPR_CNT ) { return false; }
	}

	return true;

}

/*
 * George's criterion
 *
 * Nodes a and b can be coalesced if
 * for every adjacent node t of a, either
 * - (t, b) already exists or
 * - Degree(t) < K
 *
 */
bool
L2::Color::
can_coalesce_george(
	const L2::var_id_t max_var_id,
	const std::vector< std::size_t > &degrees,
	const L2::Liv::VarIdSet &nei_id_set_1,
	const L2::Liv::VarIdSet &nei_id_set_2
)
{
	for (
		L2::var_id_t id { MIN_GPR_ID };
		id <= max_var_id;
		++id
	)
	{
		//
		// skip IDs that are not neighbors of node 1
		//
		if ( !nei_id_set_1.has( id ) ) { continue; }
		//
		// if node 1's neighbor is node 2's neighbor too, it's safe
		//
		if ( nei_id_set_2.has( id ) ) { continue; }
		//
		// if node 1's neighbor has low degree, it's safe
		//
		if ( degrees[ id ] < GPR_CNT ) { continue; }
		//
		// can't coalesce
		//
		return false;
	}
	return true;
}


