
#include "var_id_set.h"

#include <cstdio>

int main()
{
	using L2::Liv::VarIdSet;

	VarIdSet viset {}, viset2 {};

	VarIdSet viset3 {};

	viset += 0;
	viset += 1;

	for ( int i { 200 }; i<300; i+=3 )
	{
		viset2 += i;
	}

	for ( int i { 32 }; i<96; ++i )
	{
		viset += i;
	}

	std::printf( "set 1:\n" );
	viset.display();

	std::printf( "set 2:\n" );
	viset2.display();

	//std::printf( "set 2 &= set 1\n" );
	//viset2 &= viset;

	//std::printf( "set 2:\n" );
	//viset2.display();

	//std::printf( "set 3 = set 2 & set 1:\n" );
	//VarIdSet viset3 { std::move( viset2 & viset ) };

	//std::printf( "set 3:\n" );
	//viset3.display();


	std::printf( "set 1 |= set 2\n" );
	viset |= viset2;

	std::printf( "set 1:\n" );
	viset.display();

	for ( int i { 210 }; i<240; ++i )
	{	
		viset3 += i;
	}
	std::printf( "set 3:\n" );
	viset3.display();

	std::printf( "display set 3 - set 1:\n" );
	( viset3 - viset ).display();


	std::printf( "display set 1 - set 3:\n" );
	( viset - viset3 ).display();

}

