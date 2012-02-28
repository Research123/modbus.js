//
// This class is a native C++ Node.js extension for creating GTK+ desktop notification
//
#include <v8.h>
#include <node.h>

#include <string>
#include <errno.h>
#include <modbus.h>

using namespace node;
using namespace v8;

#ifndef MODBUS_OBJECT_H
#define MODBUS_OBJECT_H
namespace modbus {
        
    class ModbusObject : ObjectWrap {
    private:
        modbus_t *ctx;
    public:
        ModbusObject(modbus_t *ctx);
        ~ModbusObject();
                                
        static void Init(Handle<Object> target);
        static Handle<Value> New(const Arguments& args);
        static Handle<Value> Connect(const Arguments& args);
        static Handle<Value> Disconnect(const Arguments& args);
        
        static Handle<Value> WriteBit(const Arguments& args);
        static Handle<Value> ReadBits(const Arguments& args);
        static Handle<Value> WriteBits(const Arguments& args);
        
        static Handle<Value> ReadInputBits(const Arguments& args);
        
        static Handle<Value> WriteRegister(const Arguments& args);
        static Handle<Value> ReadRegisters(const Arguments& args);
        static Handle<Value> WriteRegisters(const Arguments& args);

        static Handle<Value> ReadInputRegisters(const Arguments& args);        

        static Handle<Value> SetFloat(const Arguments& args);                
        static Handle<Value> GetFloat(const Arguments& args);
        
        Handle<Value> WrappedObject(const Arguments& args);    
        modbus_t* GetContext();
    };
};
#endif