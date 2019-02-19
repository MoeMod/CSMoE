/*
u_ebobase.hpp - Empty Base-class Optimization
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_U_EBOBASE_HPP
#define PROJECT_U_EBOBASE_HPP

// use EBO here to reduce sizeof(vector), etc...
// TODO : get rid of EBO-base using boost::compressed_pair
namespace moe
{
	template<class Empty>
	class EBOBase : private Empty
	{
	protected:
		EBOBase(const Empty &alloc) : Empty(alloc) {}
		Empty get()
		{
			return *this;
		}
	};
}

#endif //PROJECT_U_EBOBASE_HPP
