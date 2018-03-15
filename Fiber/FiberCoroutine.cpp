/*
Example proposed by Remi Padiolleau on 2018-03-14
Based on https://schneide.wordpress.com/2016/09/19/c-coroutines-on-windows-with-the-fiber-api/ by mariuselvert
*/

#include <windows.h> 
#include <iostream>
#include <memory>
#include <functional>
#include <assert.h>

using namespace std;

class FiberCoroutine
{
public:
	// The yielding function acknowledged by the client (but not its implementation)
	//		he can use it in its code to get the control back from the coroutine
	using yield_fun = std::function<void()>;

	// To define the running function on the fiber
	using run_fun = std::function<void(yield_fun)>;

private:
	// Pointer to the main fiber
	static void* main_fiber;

	// Function to execute
	run_fun exec_fun;

	// Fiber of the coroutine
	void* fiber;

	// Indicates if the fiber has still a job to do
	bool running;
public:
	FiberCoroutine(run_fun f)
	{
		// Transfert the main thread to a fiber, if needed, 
		// in order to create the fiber for this object
		if (!main_fiber)
		{
			main_fiber = ConvertThreadToFiber(NULL);
			assert(main_fiber != NULL);
		}
		// Get the function to execute by this coroutine
		exec_fun = move(f);

		// Indicates the fiber is running
		running = true;

		// Create the fiber executing the function
		fiber = CreateFiber(0, &FiberCoroutine::execution, this);
		assert(fiber != NULL);
	}

	~FiberCoroutine()
	{
		DeleteFiber(fiber);
	}

	// Function called by the client to let 
	// the coroutine use the fiber take control for its operations
	//		return true if it needs to repeat again 
	//		the operation when the clients wants it to
	bool step()
	{
		SwitchToFiber(fiber); // Fiber execution
		return running;
	}

	// Function called when, the client wants to get 
	// back the control from the fiber, 
	//		pause the fiber
	void yield()
	{
		SwitchToFiber(main_fiber);
	}

private:
	// Entry point of the fiber 
	//		Needs to be static in order to have access 
	//		to the address of this function at compile time
	static void __stdcall execution(void* fc)
	{
		reinterpret_cast<FiberCoroutine*>(fc)->run();
	}

	// Instance executing function for multiple times called coroutine
	void run() // multiple time
	{
		// We need to trap our fiber here in order to give 
		// back the control to the client to call again this coroutine
		// (or else our fiber will be lost in the darkness)
		while (true)
		{
			// Magic here, we inject the real yielding 
			// function to the function given by the client
			exec_fun([this] { yield(); });

			// Here we have ended executing the function
			running = false;
		
			yield(); // Give back the control to the client
		}
	}

	/*
	// Instance executing function for single called coroutine
	void run() // single time
	{
		exec_fun([this] { yield(); });
		running = false;

		// We need to trap our fiber here in order to give 
		// back the control to the client even if he is stupid enough
		// to call again this coroutine (not the purpose here)
		// (or else our fiber will be lost in the darkness)
		while (true)
		{
			yield(); // Give back the control to the client
		}
	}
	*/
};

void* FiberCoroutine::main_fiber = nullptr;

int main()
{
	auto coroutine = make_unique<FiberCoroutine>(
		[](FiberCoroutine::yield_fun yield)
		{
			for (int i = 0; i<3; ++i)
			{
				cout << "Coroutine "
					<< i << std::endl;
				yield(); // here we call a yielding function defined by the FiberCoroutine
			}
		}
	);
	int stepCount = 0;
	while (coroutine->step())
	{
		cout << "Main "
			<< stepCount++ << std::endl;
	}

	// Here we try to break the code

	cout << "Hello" << endl;
	coroutine->step();
	cout << "Goodbye" << endl;
	
	system("pause");
}

/*
RESULT :
---------------
Coroutine 0
Main 0
Coroutine 1
Main 1
Coroutine 2
Main 2
Hello
Coroutine 0
Goodbye
...
---------------
*/

