#include <nan.h>
#include <dlfcn.h>

using namespace v8;
using namespace node;

namespace flac_bindings {
    NAN_MODULE_INIT(initEncoder);
    NAN_MODULE_INIT(initDecoder);
    NAN_MODULE_INIT(initMetadata);
    NAN_MODULE_INIT(initFormat);

    bool isLibFlacLoaded = false;
    void* libFlacHandle;
    Nan::Persistent<Object> module;

    NAN_METHOD(loadLibFlac) {
        if(info[0]->IsUndefined()) {
            Nan::ThrowError("Needs the path to the flac library");
        } else if(!info[0]->IsString()) {
            Nan::ThrowError("String needed representing the path to flac library");
        } else {
            Nan::Utf8String Path(info[0]);
            libFlacHandle = dlopen(*Path, RTLD_LAZY | RTLD_LOCAL);
            if(libFlacHandle == nullptr) {
                Nan::ThrowError("Could not load flac library");
            } else {
                isLibFlacLoaded = true;
                Handle<Object> obj = Nan::New(module);
                initEncoder(obj);
                initDecoder(obj);
                initFormat(obj);
                Nan::Delete(obj, Nan::New("load").ToLocalChecked());
                info.GetReturnValue().Set(obj);
            }
        }
    }

    static void atExit(void*) {
        if(isLibFlacLoaded) dlclose(libFlacHandle);
    }

    NAN_MODULE_INIT(init) {
        module.Reset(target);
        libFlacHandle = dlopen("flac", RTLD_LOCAL | RTLD_LAZY);

        if(libFlacHandle == nullptr) {
            isLibFlacLoaded = false;
            Nan::Set(target, Nan::New("load").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(loadLibFlac)).ToLocalChecked());
            return;
        }

        isLibFlacLoaded = true;

        initEncoder(target);
        initDecoder(target);
        //initMetadata(target);
        initFormat(target);

        AtExit(atExit);
    }

};

NODE_MODULE(flac_bindings, flac_bindings::init)
