
#ifndef L2_LIV_VARIDSET_H
#define L2_LIV_VARIDSET_H

#include "ints.h"
#include <vector>
#include <cstdint>
#include <concepts> 

namespace L2
{
	namespace Liv
	{
		//
		// stores IDs of variables in a single GEN/KILL/IN/OUT set
		// ( mappings are stored in a separate VarVisitor )
		//
		struct VarIdSet;

		//
		// stores the GEN/KILL/IN/OUT variable ID sets
		// for all instructions in a given scope
		//
		struct FnVarIdSets;

		struct VarIdSet
		{

			std::vector< std::uint64_t > data {};

			VarIdSet &operator|=( const VarIdSet & );
			VarIdSet &operator&=( const VarIdSet & );
			VarIdSet &operator-=( const VarIdSet & );

			VarIdSet( void );

			//
			// print var ID set raw data in hex format
			//
			void display( void ) const;

			//
			// expand the set's bit representation into a vector of variable IDs
			//
			std::vector< var_id_t > to_vec( const var_id_t ) const;

			//
			// test whether a var ID is present in the set
			//
			template< typename I > requires std::integral< I >
			bool has( const I i ) const
			{
				if ( i < 0 ) { return false; }
				if ( this->data.size() * 64 <= i ) { return false; }

				const std::uint64_t blk { this->data[ i / 64 ] };
				if ( 0x1UL & ( blk >> ( i % 64 ) ) )
				{
					return true;
				}
				return false;
				
			}

			//
			// add a var ID to the set
			//
			template< typename I > requires std::integral< I >
			VarIdSet &operator+=( const I i )
			{
				// ignore invalid var IDs
				if ( i < 0 ) { return *this; }

				// required size ( number of 64-bit blocks ) to reach and accommodate var ID i
				const std::size_t req_sz { static_cast< std::size_t > ( 1 + ( i / 64 ) ) };
				if ( req_sz > this->data.size() )
				{
					this->data.resize( req_sz, 0x0UL );
				}
				this->data[ i / 64 ] |= ( 0x1UL << ( i % 64 ) );
				return *this;
			}

			//
			// remove a var ID to the set
			//
			template< typename I > requires std::integral< I >
			VarIdSet &operator-=( const I i )
			{
				// ignore invalid var IDs
				if ( i < 0 ) { return *this; }

				const std::size_t blk_id { static_cast< std::size_t >( i ) / 64 };
				if ( blk_id + 1 > this->data.size() )
				{
					return *this;
				}
				this->data[ blk_id ] &= ~( 0x1UL << ( i % 64 ) );
				return *this;
			}


			friend VarIdSet operator|( const VarIdSet &, const VarIdSet & );
			friend VarIdSet operator&( const VarIdSet &, const VarIdSet & );
			friend VarIdSet operator-( const VarIdSet &, const VarIdSet & );

			friend bool operator==( const VarIdSet &, const VarIdSet & );
			friend bool operator!=( const VarIdSet &, const VarIdSet & );

		};

		VarIdSet operator|( const VarIdSet &, const VarIdSet & );
		VarIdSet operator&( const VarIdSet &, const VarIdSet & );
		VarIdSet operator-( const VarIdSet &, const VarIdSet & );

		bool operator==( const VarIdSet &, const VarIdSet & );
		bool operator!=( const VarIdSet &, const VarIdSet & );

		struct FnVarIdSets
		{
			std::vector< VarIdSet > GEN  {};
			std::vector< VarIdSet > KILL {};
			std::vector< VarIdSet > IN   {};
			std::vector< VarIdSet > OUT  {};

			FnVarIdSets( void ) =default;

			FnVarIdSets( const std::size_t instr_cnt );

			void display( void ) const;

		};

	}

}


#endif

