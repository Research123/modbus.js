import Options

srcdir = "."
blddir = "build"

def set_options(opt):
  opt.tool_options("compiler_cxx")  
          
def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")  
						 
def build(bld):
  modbusnode = bld.new_task_gen("cxx", "shlib", "node_addon")
  modbusnode.cxxflags = ["-I../lib/include"]
  modbusnode.ldflags = ["-L../lib", "-lmodbus"]
  modbusnode.target = "modbus"
  modbusnode.source = """
    src/node_modbus.cc
    src/modbus_object.cc
  """