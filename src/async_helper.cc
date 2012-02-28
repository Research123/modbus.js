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
            result->Set(Number::New(i), Number::New(atoi(coil)));        
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
    
    //uv_mutex_t mutex;
    //int r;
    
    //r = uv_mutex_init();
    //uv_mutex_lock(&mutex);
    data->result_code = modbus_read_input_bits(data->mObj->GetContext(), data->addr, data->nb, dest);
    data->result = dest;  
    //uv_mutex_unlock(&mutex);
    //uv_mutex_destroy(&mutex);      
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
            result->Set(Number::New(i), Number::New(atoi(coil)));        
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

void AccepQueryAsync(uv_work_t* req) {

    printf("Server started\n");    
    server_data* data = static_cast<server_data*>(req->data);    
    modbus_t *ctx = data->mObj->GetContext();            

    while (true) {
        uint8_t *query;
        int socket = 0;
        int rc = 0;

        printf("Waiting for connection\n");    
        if (data->use_backend == TCP) {
            query = (uint8_t*)malloc(MODBUS_TCP_MAX_ADU_LENGTH);
            socket = modbus_tcp_listen(ctx, 1);
            modbus_tcp_accept(ctx, &socket);
        } else if (data->use_backend == TCP_PI) {
            query = (uint8_t*)malloc(MODBUS_TCP_MAX_ADU_LENGTH);
            socket = modbus_tcp_pi_listen(ctx, 1);
            modbus_tcp_pi_accept(ctx, &socket);
        } else {
            query = (uint8_t*)malloc(MODBUS_RTU_MAX_ADU_LENGTH);
            int rc = modbus_connect(ctx);
            if (rc == -1) {
                modbus_free(ctx);
                printf("Unable to connect.");                                     
            }
        }

        printf("Client connected\n");    
        for (;;) {

            do {
                rc = modbus_receive(data->mObj->GetContext(), query);
            } while (rc == 0);            

            if (rc == -1) {
                printf("Connection closed by the client or error\n");
                break;
            }

            rc = modbus_reply(ctx, query, rc, data->mb_mapping);
            if (rc == -1) {
                printf("Unable to process response\n");
                break;
            }   
        }
        printf("Client disconnected\n");

        if (data->use_backend == TCP) {
            close(socket);
        }

        modbus_close(ctx);
        ctx = modbus_new_tcp("127.0.0.1", 1502);
        
        free(query);        
    }        

    modbus_mapping_free(data->mb_mapping);    

    printf("Server stopped\n");
}

void ProcessQueryAsync(uv_work_t* req) {
    
    /*server_data* data = static_cast<server_data*>(req->data);
    uint8_t* query = data->query;
    modbus_t *ctx = data->mObj->GetContext();    
    //int use_backend = data->use_backend;    
    modbus_mapping_t *mb_mapping = data->mb_mapping;
    int rc;
    int header_length = modbus_get_header_length(ctx);

    printf("got query number %0X\n", query[header_length]);
    */
    /* Read holding registers */
    /*if (query[header_length] == 0x03) {
        if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 3)
            == UT_REGISTERS_NB_SPECIAL) {
            printf("Set an incorrect number of values\n");
            MODBUS_SET_INT16_TO_INT8(query, header_length + 3,
                                     UT_REGISTERS_NB_SPECIAL - 1);
        } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                   == UT_REGISTERS_ADDRESS_SPECIAL) {
            printf("Reply to this special register address by an exception\n");
            modbus_reply_exception(ctx, query,
                                   MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY);
            //continue;
        } else if (MODBUS_GET_INT16_FROM_INT8(query, header_length + 1)
                   == UT_REGISTERS_ADDRESS_INVALID_TID_OR_SLAVE) {
            const int RAW_REQ_LENGTH = 5;
            uint8_t raw_req[] = {
                (use_backend == RTU) ? INVALID_SERVER_ID : 0xFF,
                0x03,
                0x02, 0x00, 0x00
            };

            printf("Reply with an invalid TID or slave\n");
            modbus_send_raw_request(ctx, raw_req, RAW_REQ_LENGTH * sizeof(uint8_t));
            //continue;
        }
    }*/

    //rc = modbus_reply(ctx, query, rc, mb_mapping);
    //if (rc == -1) {
    //    printf("Unable to process response\n");
    //    //break;
    //}
}