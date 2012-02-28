#include <v8.h>
#include <node.h>

#include <string>
#include <errno.h>
#include <modbus.h>

#include "modbus_object.h"
#include "async_helper.h"
#include "unit-test.h"

using namespace modbus;

ModbusObject::ModbusObject(modbus_t *context) {
    ctx = context; 
}

ModbusObject::~ModbusObject() {
    modbus_close(ctx);
    modbus_free(ctx);
}

modbus_t* ModbusObject::GetContext() {
    return ctx;
}

void ModbusObject::Init(Handle<Object> target) {
    v8::HandleScope scope;
    
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "connect", Connect);
    NODE_SET_PROTOTYPE_METHOD(t, "disconnect", Disconnect);
    
    NODE_SET_PROTOTYPE_METHOD(t, "write_bit", WriteBit);    
    NODE_SET_PROTOTYPE_METHOD(t, "read_bits", ReadBits);    
    NODE_SET_PROTOTYPE_METHOD(t, "write_bits", WriteBits);        
    
    NODE_SET_PROTOTYPE_METHOD(t, "read_input_bits", ReadInputBits);        
    
    NODE_SET_PROTOTYPE_METHOD(t, "write_register", WriteRegister);        
    NODE_SET_PROTOTYPE_METHOD(t, "read_registers", ReadRegisters);        
    NODE_SET_PROTOTYPE_METHOD(t, "write_registers", WriteRegisters);            
    
    NODE_SET_PROTOTYPE_METHOD(t, "read_input_registers", ReadInputRegisters);            
    
    NODE_SET_PROTOTYPE_METHOD(t, "set_float", SetFloat);                
    NODE_SET_PROTOTYPE_METHOD(t, "get_float", GetFloat);                    
    
    target->Set(String::NewSymbol("ModbusObject"), t->GetFunction());    
}

Handle<Value> ModbusObject::New(const Arguments& args) {
    HandleScope scope;

    String::Utf8Value backend_type(args[0]);
    String::Utf8Value object_type(args[1]);
    
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
        modbus_set_slave(ctx, SERVER_ID);
    }

    if (ctx == NULL) {
        return ThrowException(Exception::Error(String::New("Unable to allocate libmodbus context.")));
    } 

    modbus_set_debug(ctx, true);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_PROTOCOL);        

    ModbusObject* modbus_instance = new ModbusObject(ctx); 

    if (strcmp(*object_type, "server") == 0)
    {            
        modbus_mapping_t *mb_mapping;        
        int socket, rc;        

        mb_mapping = modbus_mapping_new(
            UT_BITS_ADDRESS + UT_BITS_NB,
            UT_INPUT_BITS_ADDRESS + UT_INPUT_BITS_NB,
            UT_REGISTERS_ADDRESS + UT_REGISTERS_NB,
            UT_INPUT_REGISTERS_ADDRESS + UT_INPUT_REGISTERS_NB);

        if (mb_mapping == NULL)             
            return ThrowException(Exception::Error(String::New("Failed to allocate the mapping.")));        

        /** INPUT STATUS **/
        modbus_set_bits_from_bytes(mb_mapping->tab_input_bits,
                                   UT_INPUT_BITS_ADDRESS, UT_INPUT_BITS_NB,
                                   UT_INPUT_BITS_TAB);

        /** INPUT REGISTERS **/
        for (int i = 0; i < UT_INPUT_REGISTERS_NB; i++) {
            mb_mapping->tab_input_registers[UT_INPUT_REGISTERS_ADDRESS+i] =
                UT_INPUT_REGISTERS_TAB[i];;
        }    

        server_data* data = new server_data();
        data->use_backend = use_backend;
        data->mObj = modbus_instance;
        data->mb_mapping = mb_mapping;

        uv_work_t* req = new uv_work_t;
        req->data = data;
        uv_queue_work(uv_default_loop(), req, AccepQueryAsync, NULL);
    }               

    modbus_instance->Wrap(args.This());
    return scope.Close(args.This());
}

Handle<Value> ModbusObject::Connect(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    if (modbus_connect(mObj->GetContext()) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(mObj->GetContext());
        return Boolean::New(false);
    }
        
    return Boolean::New(true);
}

Handle<Value> ModbusObject::Disconnect(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    modbus_close(mObj->GetContext());
    modbus_free(mObj->GetContext());
        
    return Boolean::New(true);
}

/** COIL BITS **/

Handle<Value> ModbusObject::WriteBit(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int coil_addr = args[0]->Int32Value();
    const int status = args[1]->Int32Value();
        
    return Int32::New(modbus_write_bit(mObj->GetContext(), coil_addr, status));
}

Handle<Value> ModbusObject::ReadBits(const Arguments& args) {        
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();    
    Local<Function> callback = Local<Function>::Cast(args[2]);
    
    bit_data* data = new bit_data();
    data->callback = Persistent<Function>::New(callback);
    data->addr = addr;
    data->nb = nb;
    data->mObj = mObj;    
    
    QueueWork(data, ReadBitsAsync, ReadBitsAsyncAfter);
    
    return Undefined();
}

Handle<Value> ModbusObject::WriteBits(const Arguments& args) {
    HandleScope scope;
    
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int coil_addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();   
    Local<Array> values = Local<Array>::Cast(args[2]);
    
    uint8_t tab_value[nb];   
    
    for (int i = 0; i < nb; i++) {
        String::Utf8Value val (values->Get(i));
        tab_value[i] = atoi(*val);
    } 
    
    return Int32::New(modbus_write_bits(mObj->GetContext(), coil_addr, nb, tab_value));
}

/** DISCRETE INPUTS **/

Handle<Value> ModbusObject::ReadInputBits(const Arguments& args) {
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
    
    QueueWork(data, ReadInputBitsAsync, ReadBitsAsyncAfter);
    
    return Undefined();
}

/** HOLDING REGISTERS **/

Handle<Value> ModbusObject::WriteRegister(const Arguments& args) {
    HandleScope scope;
    
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int addr = args[0]->Int32Value();
    const int value = args[1]->Int32Value();
    
    return Int32::New(modbus_write_register(mObj->GetContext(), addr, value));
}

Handle<Value> ModbusObject::ReadRegisters(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();    
    Local<Function> callback = Local<Function>::Cast(args[2]);
    
    register_data* data = new register_data();
    data->callback = Persistent<Function>::New(callback);
    data->addr = addr;
    data->nb = nb;
    data->mObj = mObj;
    
    QueueWork(data, ReadRegistersAsync, ReadRegistersAsyncAfter);
    
    return Undefined();
}

Handle<Value> ModbusObject::WriteRegisters(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int coil_addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();   
    Local<Array> values = Local<Array>::Cast(args[2]);
    
    uint16_t tab_value[nb];   

    for (int i = 0; i < nb; i++) {
        String::Utf8Value val (values->Get(i));
        tab_value[i] = atoi(*val);
    } 
    
    return Int32::New(modbus_write_registers(mObj->GetContext(), coil_addr, nb, tab_value));
}

/** INPUT REGISTERS **/

Handle<Value> ModbusObject::ReadInputRegisters(const Arguments& args) {
    ModbusObject *mObj = ObjectWrap::Unwrap<ModbusObject>(args.This());
    
    const int addr = args[0]->Int32Value();
    const int nb = args[1]->Int32Value();    
    Local<Function> callback = Local<Function>::Cast(args[2]);
    
    register_data* data = new register_data();
    data->callback = Persistent<Function>::New(callback);
    data->addr = addr;
    data->nb = nb;
    data->mObj = mObj;
    
    QueueWork(data, ReadInputRegistersAsync, ReadRegistersAsyncAfter);
    
    return Undefined();
}

/** FLOAT **/

Handle<Value> ModbusObject::SetFloat(const Arguments& args) {
    HandleScope scope;
    
    const float value = args[0]->NumberValue();
    
    uint16_t storage[2];    
    modbus_set_float(value, storage);
    
    return Int32::New(*(uint32_t*)storage);
}

Handle<Value> ModbusObject::GetFloat(const Arguments& args) {
    HandleScope scope;
    
    Local<Array> values = Local<Array>::Cast(args[0]);

    uint16_t storage[2];    
    
    for (int i = 0; i < 2; i++) {
        String::Utf8Value val (values->Get(i));
        storage[i] = atoi(*val);
    } 
    
    float result = modbus_get_float(storage);
    
    return Number::New(result);
}