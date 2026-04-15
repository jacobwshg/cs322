
#ifndef L2_SVUTIL_H
#define L2_SVUTIL_H

#include <string>
#include <string_view>

namespace SVUtil
{
	struct TransparentHash
	{
		using is_transparent = bool;

		std::size_t operator()( const std::string_view sv ) const
		{
			return std::hash< std::string_view >{}( sv );
		}
	};

	struct TransparentEq
	{
		using is_transparent = bool;

		bool operator()(
			const std::string_view sv1,
			const std::string_view sv2
		) const
		{
			return sv1 == sv2;
		}
	};
}

#endif

