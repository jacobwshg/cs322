
#include "var_id_set.h"
#include "ints.h"
#include "gprid.h"

#include <cstdint>
#include <cstdio>
#include <vector>

L2::Liv::
VarIdSet::VarIdSet( void )
{
	this->data.resize( 1, 0x0UL );
}

//
// print data in hex format, one line per 64-bit block
//
void
L2::Liv::
VarIdSet::display( void ) const
{
	long long int blk_base { 0LL };
	for ( const std::uint64_t blk: this->data )
	{
		std::printf(
			"%0lld:%0lld\t0x%016lx\n",
			blk_base + 63, blk_base, blk
		);
		blk_base += 64;
	}
	//std::printf( "\n" );
}

std::vector< L2::var_id_t >
L2::Liv::
VarIdSet::to_vec( const L2::var_id_t max_var_id ) const
{
	std::vector< var_id_t > var_id_vec {};	
	var_id_vec.reserve( 1 + max_var_id );

	for (
		var_id_t var_id { L2::Liv::MIN_GPR_ID };
		var_id <= max_var_id;
		++var_id
	)
	{
		if ( this->has( var_id ) )
		{
			var_id_vec.emplace_back( var_id );
		}
	}
	return std::move( var_id_vec );
}

//
// compute var ID set union in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator|=( const VarIdSet &that )
{
	std::size_t
		this_sz { this->data.size() };
	const std::size_t 
		that_sz { that.data.size() };

	//
	// extend `this` to be as long as `that`, so `this` can
	// collect `that`'s extra set bits
	//
	if ( this_sz < that_sz )
	{
		this->data.resize( that_sz, 0x0UL );
		this_sz = that_sz;
	}

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk : this->data )
	{
		if ( blk_id >= that_sz ) { break; }
		blk |= that.data[ blk_id ];

		++blk_id;
	}

	return *this;
}

//
// compute var ID set intersection in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator&=( const VarIdSet &that )
{
	const std::size_t
		this_sz { this->data.size() },
		that_sz { that.data.size() };

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk : this->data )
	{
		if ( blk_id < that_sz )
		{
			//std::printf( "VarIdSet &=  shared blk %0ld\n", blk_id );
			blk &= that.data[ blk_id ];
		}
		//
		// if `this` has more blocks than `that`, the intersection
		// must be cleared of elements ( set bits ) exclusive to `this`
		//
		else
		{
			//std::printf( "VarIdSet &= this-only block %0ld = 0x%016lx\n", blk_id, blk );
			blk = 0x0UL;
		}

		++blk_id;
	}

	return *this;
}

//
// compute var ID set subtraction in place
//
L2::Liv::VarIdSet &
L2::Liv::
VarIdSet::operator-=( const VarIdSet &that )
{
	const std::size_t
		this_sz { this->data.size() },
		that_sz { that.data.size() };

	std::size_t blk_id { 0 };
	for ( std::uint64_t &blk: this->data )
	{
		// 
		// if `this` has more blocks than `that`, the blocks missing from `that`
		// have no impact on blocks exclusive to `this`
		//
		if ( blk_id >= that_sz ) { break; }

		//
		// suppose `this` has block `1111`,`that` has block `0101`
		// `that` block flipped is `1010`
		// `this` block becomes `1010` after AND with flipped `that` block
		// shared elements are cleared
		//
		blk &= ~( that.data[ blk_id ] );

		++blk_id;
	}

	return *this;
}


L2::Liv::VarIdSet
L2::Liv::
operator|( const VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet new_lhs { lhs }; // copy
	new_lhs |= rhs;
	return std::move( new_lhs );
}


L2::Liv::VarIdSet
L2::Liv::
operator&( const L2::Liv::VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet new_lhs { lhs }; 
	new_lhs &= rhs;
	return std::move( new_lhs );
}

L2::Liv::VarIdSet
L2::Liv::
operator-( const L2::Liv::VarIdSet &lhs, const L2::Liv::VarIdSet &rhs )
{
	L2::Liv::VarIdSet new_lhs { lhs };
	new_lhs -= rhs;
	return std::move( new_lhs );
}


bool
L2::Liv::
operator!=( const VarIdSet &lhs, const VarIdSet &rhs )
{
	const std::size_t
		lsz { lhs.data.size() }, rsz { rhs.data.size() };

	// ensure lsz>=rsz to simplify logic
	if ( rsz > lsz )
	{
		return rhs != lhs;
	}

	std::size_t blk_id { 0 };
	for ( const std::uint64_t blk : lhs.data )
	{
		if ( blk_id >= rsz )
		{
			// unequal if lhs has any set bit 
			// in its exclusive blocks
			if ( blk != 0x0UL ) { return true; }
		}
		else
		{
			// unequal if any shared block differs
			if ( blk != rhs.data[ blk_id ] ) { return true; }
		}

		++blk_id;
	}
	return false;
}

bool
L2::Liv::
operator==( const VarIdSet &lhs, const VarIdSet &rhs )
{
	return !( lhs != rhs );
}

L2::Liv::
FnVarIdSets::FnVarIdSets( const std::size_t instr_cnt )
{
	const std::size_t safe_cnt { instr_cnt + 1 };
	this->GENs .resize( safe_cnt, {} );
	this->KILLs.resize( safe_cnt, {} );
	this->INs  .resize( safe_cnt, {} );
	this->OUTs .resize( safe_cnt, {} );
}

void
L2::Liv::
FnVarIdSets::display( void ) const
{
	const std::size_t sz { this->GENs.size() };

	instr_id_t instr_id { -1 };

	std::printf( "function var id sets\n" );

	for ( const VarIdSet &gen_st : this->GENs )
	{
		++instr_id;
		std::printf( "instruction ID %0d\n", instr_id );

		std::printf( "GEN\n" );
		gen_st.display();

		std::printf( "KILLs\n" );
		this->KILLs[ instr_id ].display();

		std::printf( "IN\n" );
		this->INs[ instr_id ].display();

		std::printf( "OUT\n" );
		this->OUTs[ instr_id ].display();

	}

}



