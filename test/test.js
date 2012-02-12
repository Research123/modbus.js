var modbus = require("../build/Release/modbus.node"), 
    client = new modbus.ModbusObject("tcp");

function arrays_equal(a,b) { return !(a<b || b<a); }

module.exports = {
    connect: function(assert) {
        assert.expect(1);
        assert.ok(client.connect(), "Unable to connect to the server");
        assert.done();
    },
    write_bit: function(assert) {
        assert.expect(3);
        assert.equal(client.write_bit(0x13, 1), 1, "write_bit failed");
        client.read_bits(0x13, 1, function(err, res) {
            assert.equal(err, 1, "Expected 1, got " + err);
            assert.equal(res, 1, "Expected ON");
            assert.done();
        });         
    },
    write_bits: function(assert) {
        assert.expect(3);
        var values = [1,0,1,1,0,0,1,1,1,1,0,1,0,1,1,0,0,1,0,0,1,1,0,1,0,1,1,1,0,0,0,0,1,1,0,1,1];        
        assert.equal(client.write_bits(0x13, values.length, values), values.length, "write_bits failed");
        client.read_bits(0x13, values.length, function(err, res) {
            assert.equal(err, values.length, "Expected result shoud be equal input array length");
            assert.ok(arrays_equal(res, values), "Unexpected values " + res);
            assert.done();
        });         
    },
    read_input_bits: function(assert) {
        assert.expect(2);
        var values = [0,0,1,1,0,1,0,1,1,1,0,1,1,0,1,1,1,0,1,0,1,1];        
        client.read_input_bits(0xC4, values.length, function(err, res) {
            assert.equal(err, values.length, "Expected result shoud be equal input array length");
            assert.ok(arrays_equal(res, values), "Unexpected values " + res);
            assert.done();
        });         
    },
    write_register: function(assert) {
        assert.expect(3);
        assert.equal(client.write_register(0x6B, 0x1234), 1, "write_register failed");
        client.read_registers(0x6B, 1, function(err, res) {
            assert.equal(err, 1, "Expected 1, got " + err);
            assert.equal(res, 0x1234, "Expected 0x1234");
            assert.done();
        });         
    },
    write_registers: function(assert) {
        assert.expect(3);
        var values = [0x022B, 0x0001, 0x0064];        
        assert.equal(client.write_registers(0x6B, values.length, values), values.length, "write_registers failed");
        client.read_registers(0x6B, values.length, function(err, res) {
            assert.equal(err, values.length, "Expected result shoud be equal input array length");
            assert.ok(arrays_equal(res, values), "Unexpected values " + res);
            assert.done();
        });         
    },
    read_input_registers: function(assert) {
        assert.expect(2);
        client.read_input_registers(0x08, 1, function(err, res) {
            assert.equal(err, 1, "Expected result shoud be equal input array length");
            assert.equal(res, 0x000A, "Unexpected values " + res);
            assert.done();
        });         
    },
    set_float: function(assert) {
        assert.expect(1);
        assert.ok(arrays_equal(client.set_float(916.540649), [0x229A, 0x4465]), "Unexpected values");
        assert.done();
    },
    get_float: function(assert) {
        assert.expect(1);
        assert.ok(Math.abs(client.get_float([0x229A, 0x4465]) - 916.540649) < 0.001, "Unexpected values");
        assert.done();
    }
}