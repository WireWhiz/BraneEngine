#include "virtualSystemCompiler.h"
#include <core/virtualType.h>
using namespace asmjit;

typedef int (*Func)(void);
int VirtualSystemCompiler::testFunction()
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
	if (err) return -1;                // Handle a possible error returned by AsmJit.
	// ----> CodeHolder is no longer needed from here and can be destroyed <----

	int result = fn();

	rt.release(fn);

	return result;                // Execute the generated code.

}

int VirtualSystemCompiler::testFunction2()
{
	JitRuntime rt;
	CodeHolder code;

	code.init(rt.environment());
	x86::Compiler cc(&code);

	cc.addFunc(FuncSignatureT<int, VirtualSystemCompiler*>(CallConv::kIdHost));// Begin a function of `int fn(void)` signature.


	x86::Gp ret = cc.newInt32("ret");
	x86::Gp thisPtr = cc.newIntPtr("thisPtr");
	x86::Gp a = cc.newInt32("a");
	x86::Gp b = cc.newInt32("b");
	cc.setArg(0, thisPtr);
	cc.mov(a, 2);
	cc.mov(b, 1);

	InvokeNode* testCall;
	cc.invoke(&testCall,
			  imm((void*)callMember),
			  FuncSignatureT<int, VirtualSystemCompiler*, int, int>(CallConv::kIdHost));
	testCall->setArg(0, thisPtr);
	testCall->setArg(1, a);
	testCall->setArg(2, b);
	testCall->setRet(0, ret);

	cc.ret(ret);                     // Return `vReg` from the function.

	cc.endFunc();                     // End of the function body.
	cc.finalize();                    // Translate and assemble the whole 'cc' content.
	// ----> x86::Compiler is no longer needed from here and can be destroyed <----

	int (*fn)(VirtualSystemCompiler*);
	Error err = rt.add(&fn, &code);   // Add the generated code to the runtime.
	if (err) return -1;                // Handle a possible error returned by AsmJit.
	// ----> CodeHolder is no longer needed from here and can be destroyed <----

	int result = fn(this);

	rt.release(fn);

	return result;                // Execute the generated code.
}

