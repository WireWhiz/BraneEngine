#include "VirtualSystemCompiler.h"

using namespace asmjit;




typedef int (*Func)(void);
void VirtualSystemCompiler::testFunction()
{
	JitRuntime rt;
	CodeHolder code; 

	code.init(rt.environment());
	x86::Compiler cc(&code);

	cc.addFunc(FuncSignatureT<int, byte**>(CallConv::kIdHost));// Begin a function of `int fn(void)` signature.


	x86::Gp ret = cc.newInt32("ret");
	x86::Gp a = cc.newInt32("a");
	x86::Gp b = cc.newInt32("b");
	cc.mov(a, 2);
	cc.mov(b, 3);

	InvokeNode* testCall;
	cc.invoke(&testCall,
			  imm((void*)testCalledFunction),
			  FuncSignatureT<int, int, int>(CallConv::kIdHost));
	testCall->setArg(0, a);
	testCall->setArg(1, b);
	testCall->setRet(0, ret);

	cc.ret(ret);                     // Return `vReg` from the function.

	cc.endFunc();                     // End of the function body.
	cc.finalize();                    // Translate and assemble the whole 'cc' content.
	// ----> x86::Compiler is no longer needed from here and can be destroyed <----

	Func fn;
	Error err = rt.add(&fn, &code);   // Add the generated code to the runtime.
	if (err) return;                // Handle a possible error returned by AsmJit.
	// ----> CodeHolder is no longer needed from here and can be destroyed <----


	int result = fn();                // Execute the generated code.
	printf("%d\n", result);           // Print the resulting "1".

	rt.release(fn);
}

