#include <node/node.h>
#include <iostream>
#include <node/uv.h>

using namespace v8;

namespace promiseExp {

	Persistent<Promise::Resolver> resolverGlobal;
	Isolate *commonIsolate;

	void worker(uv_work_t *req) {
		int k;
		for(int i = 0;i < 100000; i++) {
			for(int j = 0;j < 1000; j++) {
				k++;
			}
			k++;
		}
	}
	
	void afterEnd(uv_work_t *req, int status) {
		Isolate *isolate = commonIsolate;
		HandleScope scope(commonIsolate);
		Local<Promise::Resolver> resolver = Local<Promise::Resolver>::New(isolate, resolverGlobal);
		Local<String> str =  String::NewFromUtf8(isolate, "This is code from C++") ;
		resolver->Resolve(str);
	}

	void startPromise(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		commonIsolate = isolate;
		
		EscapableHandleScope scope(isolate);
		Local<Promise::Resolver> resolver = Promise::Resolver::New(isolate->GetCallingContext()).ToLocalChecked();
		resolverGlobal.Reset(isolate, resolver);
		Local<Promise> promise = resolver->GetPromise();
		Isolate::AllowJavascriptExecutionScope allowJSScope(isolate);

		uv_work_t *t = new uv_work_t();
		uv_queue_work(uv_default_loop(), t, worker, afterEnd);
		args.GetReturnValue().Set(promise);
	}

	void Init(Local<Object> exports) {
		NODE_SET_METHOD(exports, "startPromise", startPromise);
	}

	NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
}

