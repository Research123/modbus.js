#include "async_helper.h"

void QueueWork(async_data* data, uv_work_cb work_cb, uv_after_work_cb after_work_cb) {
    uv_work_t* req = new uv_work_t;
    req->data = data;
    uv_queue_work(uv_default_loop(), req, work_cb, after_work_cb);
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
    
    if (data->result_code == data->nb) {
        result = Array::New(data->nb);
        
        char coil[1];
        for (int i = 0; i < data->nb; i++) {            
            sprintf(coil, "%0X", data->result[i]);            
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

void ReadInputBitsAsync(uv_work_t* req) {
    
    bit_data* data = static_cast<bit_data*>(req->data);
    
    uint8_t *dest = (uint8_t *) malloc(data->nb * sizeof(uint8_t));
    
    data->result_code = modbus_read_input_bits(data->mObj->GetContext(), data->addr, data->nb, dest);
    data->result = dest;        
}

void ReadRegistersAsync(uv_work_t* req) {
    
    register_data* data = static_cast<register_data*>(req->data);
    
    uint16_t *dest = (uint16_t *) malloc(data->nb * sizeof(uint16_t));
    
    data->result_code = modbus_read_registers(data->mObj->GetContext(), data->addr, data->nb, dest);
    data->result = dest;        
}

void ReadRegistersAsyncAfter(uv_work_t* req) {
    
    register_data* data = static_cast<register_data*>(req->data);
    
    Local<Array> result = Array::New(0);
    
    if (data->result_code == data->nb) {
        result = Array::New(data->nb);
        
        char coil[1];
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

void ReadInputRegistersAsync(uv_work_t* req) {
    
    register_data* data = static_cast<register_data*>(req->data);
    
    uint16_t *dest = (uint16_t *) malloc(data->nb * sizeof(uint16_t));
    
    data->result_code = modbus_read_input_registers(data->mObj->GetContext(), data->addr, data->nb, dest);
    data->result = dest;        
}