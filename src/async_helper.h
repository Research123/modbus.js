#include <v8.h>
#include <node.h>

#include <string>
#include <errno.h>
#include <modbus.h>

#include "modbus_object.h"
#include "unit-test.h"

using namespace modbus;
using namespace node;
using namespace v8;

struct async_data {
    Persistent<Function> callback;
    modbus::ModbusObject* mObj;
    int addr;
    
    int result_code;    
    
    virtual ~async_data() {
        callback.Dispose();
    }
};

struct bit_data : async_data {    
    int nb;
    uint8_t* result;
};

struct register_data : async_data {
    int nb;
    uint16_t* result;
};

void QueueWork(async_data* data, uv_work_cb work_cb, uv_after_work_cb after_work_cb);

void ReadBitsAsync(uv_work_t* req);
void ReadInputBitsAsync(uv_work_t* req);
void ReadBitsAsyncAfter(uv_work_t* req);

void ReadRegistersAsync(uv_work_t* req);
void ReadRegistersAsyncAfter(uv_work_t* req);

void ReadInputRegistersAsync(uv_work_t* req);
