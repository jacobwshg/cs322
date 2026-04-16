
#ifndef L2_LIV_VARIDSET_H
#define L2_LIV_VARIDSET_H

#include <vector>
#include <cstdint>
#include <concepts> 

namespace L2
{
	namespace Liv
	{

		struct VarIdSet
		{
			std::vector< std::uint64_t > data {};

			VarIdSet &operator|=( const VarIdSet & );
			VarIdSet &operator&=( const VarIdSet & );
			VarIdSet &operator-=( const VarIdSet & );

			VarIdSet( void );

			void display( void ) const;

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
				this->data[ i / 64 ] |= ( 0x1ULL << ( i % 64 ) );
				return *this;
			}

			friend VarIdSet operator|( const VarIdSet &, const VarIdSet & );
			friend VarIdSet operator&( const VarIdSet &, const VarIdSet & );
			friend VarIdSet operator-( const VarIdSet &, const VarIdSet & );

		};

		VarIdSet operator|( const VarIdSet &, const VarIdSet & );

		VarIdSet operator&( const VarIdSet &, const VarIdSet & );

		VarIdSet operator-( const VarIdSet &, const VarIdSet & );

	}

}


#endif

