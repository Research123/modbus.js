var modbus = require("../build/Release/modbus.node"), 
    client = new modbus.ModbusObject("tcp", "server");