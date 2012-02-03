var modbus = require("../build/Release/modbus.node"), 
    client = new modbus.ModbusObject("tcp");

if ( client.connect() ) {
    /*console.log("write_bit result = " + client.write_bit(0x13, 1));
    
    client.read_bits(0x13, 1, function(err, res) {
        console.log("read_bit result code:" + err + ",data:" + res);
    });*/    
    
    /*console.log("write_bits result = " + 
        client.write_bits(0x13, 0x25, 
        [1,0,1,1,0,0,1,1,1,1,0,1,0,1,1,0,0,1,0,0,1,1,0,1,0,1,1,1,0,0,0,0,1,1,0,1,1]));
        
    client.read_bits(0x13, 0x25, function(err, res) {
        console.log("read_bits result code:" + err + ",data:" + res);
    });*/
    
    /*client.read_input_bits(0xC4, 0x16, function(err, res) {
        console.log("read_bits result code:" + err + ",data:" + res);
    });*/
    
    /*console.log("write_register result = " + client.write_register(0x6B, 0x1234));
    
    client.read_registers(0x6B, 1, function(err, res) {
        console.log("read_register result code:" + err + ",data:" + res);
    });*/
    
    /*console.log("write_registers result = " + 
        client.write_registers(0x6B, 0x3, [0x022B, 0x0001, 0x0064]));
        
    client.read_registers(0x6B, 0x3, function(err, res) {
        console.log("read_registers result code:" + err + ",data:" + res);
    });*/
    
    client.read_input_registers(0x08, 0x1, function(err, res) {
        console.log("read_registers result code:" + err + ",data:" + res);
    });
    
    console.log("set_float result = " + client.set_float(916.540649));
    console.log("get_float result = " + client.get_float([0x229A, 0x4465]));            
}