#include <node/node.h>
#include <iostream>
#include <node/uv.h>
#include <thread>

using namespace v8;
using namespace std;

namespace promiseExp {

	Persistent<Promise::Resolver> resolverGlobal;
	Isolate *commonIsolate;

	
	void afterEnd(uv_async_t *async) {
		Isolate *isolate = commonIsolate;
		HandleScope scope(commonIsolate);
		Local<Promise::Resolver> resolver = Local<Promise::Resolver>::New(isolate, resolverGlobal);
		Local<String> str =  String::NewFromUtf8(isolate, "This is code from C++") ;
		resolver->Resolve(str);
	}

	void worker() {
		int k;
		for(int i = 0;i < 100000; i++) {
			for(int j = 0;j < 1000; j++) {
				k++;
			}
			k++;
		}
		uv_async_t *init = new uv_async_t();
		uv_async_init(uv_default_loop(), init, afterEnd);
		uv_async_send(init);
	}
	
	void startPromise(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		commonIsolate = isolate;
		
		EscapableHandleScope scope(isolate);
		Local<Promise::Resolver> resolver = Promise::Resolver::New(isolate->GetCallingContext()).ToLocalChecked();
		resolverGlobal.Reset(isolate, resolver);
		Local<Promise> promise = resolver->GetPromise();

		std::thread *thr = new std::thread(worker);
		args.GetReturnValue().Set(promise);
	}

	void Init(Local<Object> exports) {
		NODE_SET_METHOD(exports, "startPromise", startPromise);
	}

	NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}

