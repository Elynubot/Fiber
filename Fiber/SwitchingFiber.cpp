/*
	Example proposed by Remi Padiolleau on 2018-03-14
	Based on https://msdn.microsoft.com/en-us/library/windows/desktop/ms686919(v=vs.85).aspx
*/

#include <windows.h> // needed for Fibers functions
#include <iostream>

using namespace std;

#define NB_FIBER 3

#define PRIMARY_FIBER 0
#define FIRST_FIBER 1
#define SECOND_FIBER 2

// Fiber pool, it needs to stay a global variable in my example
void* fibers[NB_FIBER];

// Entry point of Fiber 1
void __stdcall first_fiber_func(void* param) // the use of __stdcall is needed
{
	// Debug message
	cout << "Fibre 1 " << endl;

	// Switch to the second fiber
	SwitchToFiber(fibers[SECOND_FIBER]);
}

// Entry point of Fiber 2
void __stdcall second_fiber_func(void* param)
{
	// Debug message
	cout << "Fibre 2 " << endl;

	// Switch to the main fiber
	SwitchToFiber(fibers[PRIMARY_FIBER]);
}

int main()
{
	// Convert the actual thread into a fiber
	//		Needed for creating and scheduling other fibers
	fibers[PRIMARY_FIBER] = ConvertThreadToFiber(nullptr); 
	if (fibers[PRIMARY_FIBER] == NULL)
	{
		cout << "ConvertThreadToFiber error" << endl;
		return -1;
	}


	// Creation of 2 other fibers
	fibers[FIRST_FIBER] = CreateFiber(0, first_fiber_func, nullptr);
	if (fibers[FIRST_FIBER] == NULL)
	{
		cout << "ConvertThreadToFiber error" << endl;
		return -1;
	}
	fibers[SECOND_FIBER] = CreateFiber(0, second_fiber_func, nullptr);
	if (fibers[SECOND_FIBER] == NULL)
	{
		cout << "ConvertThreadToFiber error" << endl;
		return -1;
	}

	// Debug message
	cout << "Hello" << endl;

	// Switch to the fiber 1
	SwitchToFiber(fibers[FIRST_FIBER]);

	// Debug message
	cout << "Goodbye" << endl;

	// Freeing the fibers
	DeleteFiber(fibers[FIRST_FIBER]);
	DeleteFiber(fibers[SECOND_FIBER]);

	return 0;
}


/*
RESULT :
---------------
Hello
Fibre 1
Fibre 2
Goodbye
...
---------------
*/
