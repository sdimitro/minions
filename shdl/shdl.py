#!/usr/bin/python3

import sys

from textx.metamodel import metamodel_from_file


class Module(object):
    def __init__(self):
        self.name = '@NO_NAME@'
        self.input_vars = []
        self.output_vars = []
        self.registers = []
        self.wires = []
        self.funcs = {}

    def __str__(self):
        return '@NO_REPR@'

    def parse(self, model):
        self.name = model.name
        for c in model.declarations:
            if c.__class__.__name__ == 'InputDecl':
                self.input_vars += c.input_vars
            elif c.__class__.__name__ == 'OutputDecl':
                for q in c.output_vars:
                    if q.__class__.__name__ == 'RegDecl':
                        self.output_vars.append(q.identifier)
                        self.registers.append(q.identifier)
                    else:
                        self.output_vars.append(q)
            elif c.__class__.__name__ == 'FunDecl':
                self.funcs[c.fname] = ""
            else:
                print('Comment!')

    def spit(self):
        verilog = 'module ' + self.name
        verilog += " (\n\t"

        verilog += ",\n\t".join(self.input_vars)

        if len(self.output_vars) > 0:
            verilog += ",\n\t"
            verilog += ",\n\t".join(self.output_vars)

        verilog += "\n);\n\n"

        if len(self.input_vars) > 0:
            verilog += "// --- Input Ports ---\n"
            verilog += 'input '
            verilog += ",\n\t".join(self.input_vars)
            verilog += ";\n\n"

        if len(self.output_vars) > 0:
            verilog += "// --- Output Ports ---\n"
            verilog += 'output '
            verilog += ",\n\t".join(self.output_vars)
            verilog += ";\n\n"

        if len(self.registers) > 0:
            verilog += "// --- Internal Variables ---\n"
            verilog += 'reg '
            verilog += ",\n\t".join(self.registers)
            verilog += ";\n\n"

        verilog += "// --- Code Area Below ---\n"

        if len(self.funcs) > 0:
            for k in self.funcs:
                verilog += 'begin : '
                verilog += k
                verilog += "\n"
                verilog += "end\n"

        verilog += "// --- Code Area Above ---\n"

        verilog += "endmodule\n"

        print(verilog)
        print(self.name)
        print(self.input_vars)
        print(self.output_vars)
        print(self.registers)
        print(self.wires)
        print(self.funcs)

def main():
    shdl_mm = metamodel_from_file('shdl.tx')
    shdl_model = shdl_mm.model_from_file(sys.argv[1])
    module = Module()
    module.parse(shdl_model)
    module.spit()

if __name__ == '__main__': main()
