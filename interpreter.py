import sys
import numpy as np

import re

def check_balanced_parentheses(expression):
    # Remove all non-parenthesis characters
    cleaned_expr = re.sub(r'[^\[\]]', '', expression)
    # Check if the cleaned expression is balanced
    return cleaned_expr.count('[') == cleaned_expr.count(']') 

import os

class Constants:
    _SIZE_OF_TAPE = 1000
    _input_files = []  # Initialize as an empty list
    
    def __init__(self) -> None:
        # go in a directory and find all .b files and add into _input_files
        directory = 'brainfuck-benchmark/benches/'
        for filename in os.listdir(directory):
            if filename.endswith('.b'):
                self._input_files.append(os.path.join(directory, filename))
    
    # Adding property decorator makes it read only, can't mutate objects.
    @property
    def tape_size(self):
        return self._SIZE_OF_TAPE
    
    @property
    def input_files(self):
        return self._input_files

class Interpreter:
    def __init__(self, tape_size):
        self._tape = np.zeros(tape_size, dtype=np.uint8)    # uint8 tape initialized to 0(0-255) (Wrapping handled by Numpy)
        self._tape_pointer = tape_size // 2 # tape head points to center of tape
        self._preprocessed = []  # to store the preprocessed code
    
    def increment(self):
        self._tape[self._tape_pointer] += 1
    def decrement(self):
        self._tape[self._tape_pointer] -= 1
    def move_left(self):
        if self._tape_pointer == 0:
            # dynamically increase the list by prepending zeros
            self._tape = np.concatenate((np.zeros(1000, dtype=np.uint8), self._tape))
            self._tape_pointer += 1000
        self._tape_pointer -= 1
    def move_right(self):
        if (self._tape_pointer == len(self._tape) - 1):
            self._tape = np.concatenate((self._tape, np.zeros(1000, dtype=np.uint8)))   # Handled the dynamic increase of tape
        self._tape_pointer += 1
    def write(self):
        intval = self._tape[self._tape_pointer]
        print(chr(intval), end='')  # what does chr do? Converts int to char, so if value is 65, it returns 'A'. 
    def replace(self, input_value):
        self._tape[self._tape_pointer] = input_value
    def is_currentcell_zero(self):
        return self._tape[self._tape_pointer] == 0
    
    def execute(self, instruction, PC_Index=None, filename=None):
        if instruction == '>':
            self.move_right()
            PC_Index += 1
            return PC_Index
        elif instruction == '<':
            self.move_left()
            PC_Index += 1
            return PC_Index
        elif instruction == '+':
            self.increment()
            PC_Index += 1
            return PC_Index
        elif instruction == '-':
            self.decrement()
            PC_Index += 1
            return PC_Index
        elif instruction == '.':    # output
            self.write()
            PC_Index += 1
            return PC_Index
        # elif instruction == ',':
        #     #self.replace(input_value)
        #     pass
        elif instruction == '[':
            if self.is_currentcell_zero():
                temp_pc_index = PC_Index
                while temp_pc_index < len(self._preprocessed):
                    if self._preprocessed[temp_pc_index] == ']':
                        PC_Index = temp_pc_index
                        return PC_Index
                    temp_pc_index += 1
            else:
                PC_Index += 1
                return PC_Index
        elif instruction == ']':
            if not self.is_currentcell_zero():
                temp_pc_index = PC_Index
                while temp_pc_index >= 0:
                    if self._preprocessed[temp_pc_index] == '[':
                        PC_Index = temp_pc_index
                        return PC_Index
                    temp_pc_index -= 1
            else:
                PC_Index += 1
                return PC_Index
        else:
            print(f"Failed to interpret code from file={filename}, at position = {PC_Index}: and at instruction = '{instruction}', "
                  "either this instruction is not implemented or some other catastrophic error occurred.")
            sys.exit(1)
    

    def interpret(self, code, filename):
        print("len before = ", len(code))
        # preprocess the input
        for eachword in code:
            if eachword not in ['>', '<', '+', '-', '.', ',', '[', ']']:
                continue    # skip the invalid commands
            else:
                self._preprocessed.append(eachword)
        print("len = ", len(self._preprocessed))
        # do matching parenthesis using python regex library.
        if not check_balanced_parentheses(str(self._preprocessed)):
            print(f"Unbalanced parentheses in file={filename}")
            sys.exit(1)
                       
        # read the file and read each word one by one.
        PC_index = 0
        while PC_index < len(self._preprocessed):
            PC = self._preprocessed[PC_index]
            new_PC_index = self.execute(instruction=PC, PC_Index=PC_index, filename=filename)
            PC_index = new_PC_index
        print(f"\nSuccessfully interpreted code from file={filename}")


def dbgs(object):
    if (True):
        print("Debugging code: ", object)
    
#__main__
if __name__ == "__main__":
    import argparse
    import signal

    # Object creation
    const = Constants()
    
    parser = argparse.ArgumentParser(description='Brainfuck Interpreter')
    parser.add_argument('file', type=str, help='Brainfuck file to interpret')
    args = parser.parse_args()
    
    eachfile = args.file
    if eachfile in const.input_files:
        # Printing
        print("/" * 50)
        print("TAPE SIZE = ", const.tape_size)
        print("Running interpreter on input files = ", eachfile)        
        interpreter = Interpreter(const.tape_size)
        with open(eachfile, 'r') as f:
            code = f.read()
            interpreter.interpret(code, eachfile)
        # delete the object 
        del interpreter
        print("/" * 50)
    else:
        print(f"File {eachfile} not found in input files.")