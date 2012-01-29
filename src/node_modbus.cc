//
// This class is a native C++ Node.js extension for creating GTK+ desktop notification
//
#include <v8.h>
#include <node.h>

#include "modbus_object.h"

using namespace node;
using namespace v8;
using namespace modbus;

extern "C" {
    void init(Handle<Object> target) {
        
        ModbusObject::Init(target);
    };
    NODE_MODULE(modbus,init);
}