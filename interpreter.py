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
    
    def __init__(self):
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
        self._loop_bounds = {}    # Maps closing brackets to opening brackets
    def increment(self):
        self._tape[self._tape_pointer] += 1
    def decrement(self):
        self._tape[self._tape_pointer] -= 1
    def move_left(self):
        if self._tape_pointer == 0:
            self._tape = np.concatenate((np.zeros(1000, dtype=np.uint8), self._tape))
            self._tape_pointer += 1000
        self._tape_pointer -= 1
    def move_right(self):
        if self._tape_pointer == len(self._tape) - 1:
            self._tape = np.concatenate((self._tape, np.zeros(1000, dtype=np.uint8)))
        self._tape_pointer += 1
    def write(self):
        intval = self._tape[self._tape_pointer]
        print(chr(intval), end='')
    def replace(self, input_value):
        self._tape[self._tape_pointer] = input_value
    def is_currentcell_zero(self):
        if self._tape[self._tape_pointer] is 0:
            return True
        else:
            return False
    def preprocess_code(self):
        stack = []
        for idx, command in enumerate(self._preprocessed):
            if command == '[':
                stack.append(idx)
            elif command == ']':
                if not stack:
                    raise ValueError("Unmatched ']' found")
                start = stack.pop()
                self._loop_bounds[start] = idx
                self._loop_bounds[idx] = start
                # self._loop_end[start] = idx
        if stack:
            raise ValueError("Unmatched '[' found")
    def interpret(self, code, filename):
        # preprocess the input
        for eachword in code:
            if eachword not in ['>', '<', '+', '-', '.', ',', '[', ']']:
                continue    # skip the invalid commands
            else:
                self._preprocessed.append(eachword)
        self.preprocess_code()
        print(self._loop_bounds)

        
        PC_index = 0
        while PC_index < len(self._preprocessed):
            # print(f"PC_index = {PC_index}")
            # print(f"Current cell value = {self._tape[self._tape_pointer]}")
            instruction = self._preprocessed[PC_index]
                
            if instruction == '>':
                dbgs(instruction)
                self.move_right()
            elif instruction == '<':
                dbgs(instruction)
                self.move_left()
            elif instruction == '+':
                dbgs(instruction)
                self.increment()
            elif instruction == '-':
                dbgs(instruction)
                self.decrement()
            elif instruction == '.':
                dbgs(instruction)
                self.write()
            elif instruction == '[':
                dbgs(instruction)
                if self.is_currentcell_zero():
                    PC_index = self._loop_bounds.get(PC_index)
            elif instruction == ']':           
                dbgs(instruction)
                if not self.is_currentcell_zero():
                    PC_index = self._loop_bounds.get(PC_index)
            else:
                print(f"Failed to interpret code from file={filename}, at position = {PC_index}: and at instruction = '{instruction}'")
                sys.exit(1)

            PC_index += 1
                

        print(f"\nSuccessfully interpreted code from file={filename}")



def dbgs(object):
    if (False):
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