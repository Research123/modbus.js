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

enum {
    TCP,
    TCP_PI,
    RTU
};

struct server_data {
    modbus::ModbusObject* mObj;
    int use_backend;        
    modbus_mapping_t *mb_mapping;    
};

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

void WriteBitAsync(uv_work_t* req);
void WriteBitsAsync(uv_work_t* req);
void WriteBitsAsyncAfter(uv_work_t* req);
void ReadBitsAsync(uv_work_t* req);
void ReadInputBitsAsync(uv_work_t* req);
void ReadBitsAsyncAfter(uv_work_t* req);

void WriteRegisterAsync(uv_work_t* req);
void WriteRegistersAsync(uv_work_t* req);
void WriteRegistersAsyncAfter(uv_work_t* req);
void ReadRegistersAsync(uv_work_t* req);
void ReadRegistersAsyncAfter(uv_work_t* req);

void ReadInputRegistersAsync(uv_work_t* req);

void AccepQueryAsync(uv_work_t* req);
void ProcessQueryAsync(uv_work_t* req);