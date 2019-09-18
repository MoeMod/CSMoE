// Keep this file compiled so that libc++ will be linked.

#include <iostream>

namespace dummy
{
	int main()
	{
		std::cout << "Hello world" << std::endl;
	}
}
