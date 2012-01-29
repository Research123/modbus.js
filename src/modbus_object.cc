#include <v8.h>
#include <node.h>

#include <string>
#include <errno.h>
#include <modbus.h>

#include "modbus_object.h"
#include "unit-test.h"

using namespace modbus;

enum {
    TCP,
    TCP_PI,
    RTU
};

struct async_data {
    Persistent<Function> callback;
    ModbusObject* mObj;
    int addr;
    
    int result_code;
    uint8_t* result;
    
    virtual ~async_data() {
        callback.Dispose();
    }
};

struct bit_data : async_data {
    int nb;
};

ModbusObject::ModbusObject(modbus_t *context) {
    ctx = context; 
}

ModbusObject::~ModbusObject() {}

modbus_t* ModbusObject::GetContext() {
    return ctx;
}

void ModbusObject::Init(Handle<Object> target) {
    v8::HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "connect", Connect);
    NODE_SET_PROTOTYPE_METHOD(t, "write_bit", WriteBit);    
    NODE_SET_PROTOTYPE_METHOD(t, "read_bits", ReadBits);    
    target->Set(String::NewSymbol("ModbusObject"), t->GetFunction());
    
}

Handle<Value> ModbusObject::New(const Arguments& args) {
    HandleScope scope;
    bool return_buffers = false;

    v8::String::Utf8Value backend_type(args[0]);
    
    int use_backend = TCP;    
    modbus_t *ctx;
    
    if (strcmp(*backend_type, "tcp") == 0) {
        use_backend = TCP;
    } else if (strcmp(*backend_type, "tcppi") == 0) {
        use_backend = TCP_PI;
    } else if (strcmp(*backend_type, "rtu") == 0) {
        use_backend = RTU;
    } else {
        return ThrowException(Exception::Error(String::New("Unknown backend type.")));
    }        
    
    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi("::1", "1502");
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB1", 115200, 'N', 8, 1);
    }
    
    if (ctx == NULL) {
        return ThrowException(Exception::Error(String::New("Unable to allocate libmodbus context.")));
    }
    
    modbus_set_debug(ctx, false);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_PROTOCOL);
    
    if (use_backend == RTU) {
        modbus_set_slave(ctx, SERVER_ID);
    }
    
    ModbusObject* modbus_instance = new ModbusObject(ctx); 

    modbus_instance->Wrap(args.This());
    return scope.Close(args.This());
}

Handle<Value> ModbusObject::Connect(const Arguments& args) {
    HandleScope scope;
    
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    if (modbus_connect(mObj->GetContext()) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(mObj->GetContext());
        return Boolean::New(false);
    }
        
    return Boolean::New(true);
}

Handle<Value> ModbusObject::WriteBit(const Arguments& args) {
    HandleScope scope;
    
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int coil_addr = args[0]->Int32Value();
    const int status = args[1]->Int32Value();
        
    return Int32::New(modbus_write_bit(mObj->GetContext(), coil_addr, status));
}

void ReadBitsAsync(uv_work_t* req) {

    bit_data* data = static_cast<bit_data*>(req->data);
    
    uint8_t *dest = (uint8_t *) malloc(data->nb * sizeof(uint8_t));
    
    data->result_code = modbus_read_bits(data->mObj->GetContext(), data->addr, data->nb, dest);
    data->result = dest;    
}

void ReadBitsAsyncAfter(uv_work_t* req) {
    
    bit_data* data = static_cast<bit_data*>(req->data);
    
    Local<Array> result = Array::New(0);
    
    if (data->result_code == 1) {
        result = Array::New(data->nb);
    
        char coil [1];
        for (int i = 0; i < data->nb; i++) {
            sprintf(coil, "%i", data->result[i]);
            result->Set(Number::New(i), String::New(coil));        
        }        
    }
    
    free(data->result);
    
    Handle<Value> argv[2];
    
    argv[0] = Int32::New(data->result_code);
    argv[1] = result;
    
    data->callback->Call(Context::GetCurrent()->Global(), 2, argv);
    
    delete data;

}

Handle<Value> ModbusObject::ReadBits(const Arguments& args) {
    HandleScope scope;
    
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();    
    Local<Function> callback = Local<Function>::Cast(args[2]);
    
    bit_data* data = new bit_data();
    data->callback = Persistent<Function>::New(callback);
    data->addr = addr;
    data->nb = nb;
    data->mObj = mObj;
    
    uv_work_t* req = new uv_work_t;
    req->data = data;
    uv_queue_work(uv_default_loop(), req, ReadBitsAsync, ReadBitsAsyncAfter);
    
    return Undefined();
}