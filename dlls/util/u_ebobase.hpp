//
// Created by 小白白 on 2019-02-09.
//

#ifndef PROJECT_U_EBOBASE_HPP
#define PROJECT_U_EBOBASE_HPP

// use EBO here to reduce sizeof(vector), etc...
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
