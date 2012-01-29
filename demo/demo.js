var modbus = require("../build/Release/modbus.node"), 
    client = new modbus.ModbusObject("tcp");

if ( client.connect() ) {
    console.log("write result = " + client.write_bit(0x13, 1));
    
    client.read_bits(0x13, 1, function(err, res) {
        console.log("result code:" + err + ",data:" + res);
    });    
}