"""
Python experimental implementation for proof-of-concept. Bits and pieces will be ported to C periodically throughout development.


TODO: Definitions, string-print (."), conditionals, variables, arrays, keyboard input
            
"""
import sys

debug_show_parse = False
debug_show_stack = False

if "--show-parse" in sys.argv:
    debug_show_parse = True
if "--show-stack" in sys.argv:
    debug_show_stack = True


class Token:
    def __init__(self, val, ttype):
        self.val = val
        self.ttype = ttype

def main():
    print("[INFO] *** PHORTH: Python test implementation for North ***")

    filename = "./test/program.f"

    with open(filename, 'r') as file_in:
        file_buffer = file_in.read()

    if (file_buffer == ""):
        print("Error: File empty or error reading file...")

    else:
        """
        This differs from the C implementation, as the file is
        read outside of the conditional statement. In the C implementation,
        the file is read here.
        """
        print(f"[INFO] Read file {filename}")
        """
        Similarly here, the C implementation differs significantly as reading
        and parsing the raw file is much simpler in Python.
        """
        token_list = file_buffer.split()
        #print(token_list)

        program_tokens = []
        main_stack = [] # this will be the main stack for the program.
        variable_stack = {} # this holds constants and variables

        # Stage: parsing file, taking note of what types are contained
        for ti, tc in enumerate(token_list):
            if tc.isnumeric():
                if debug_show_parse: print(f"[{ti}] found numeric")
                if '.' in tc: program_tokens.append(Token(float(tc), 0))
                else: program_tokens.append(Token(int(tc), 0))
            elif tc[0] == '-' and len(tc) >= 2:
                if tc[1:].isnumeric():
                    if debug_show_parse: print(f"[{ti}] found negative numeric")
                    if '.' in tc: program_tokens.append(Token(float(tc), 0))
                    else: program_tokens.append(Token(int(tc), 0))
            elif tc == "+":
                if debug_show_parse: print(f"[{ti}] found + operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "-":
                if debug_show_parse: print(f"[{ti}] found - operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "*":
                if debug_show_parse: print(f"[{ti}] found * operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "/":
                if debug_show_parse: print(f"[{ti}] found / operator")
                program_tokens.append(Token(tc, 1))
            elif tc == ".":
                if debug_show_parse: print(f"[{ti}] found . operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "swap":
                if debug_show_parse: print(f"[{ti}] found swap operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "dup":
                if debug_show_parse: print(f"[{ti}] found dup operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "drop":
                if debug_show_parse: print(f"[{ti}] found drop operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "over":
                if debug_show_parse: print(f"[{ti}] found over operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "rot":
                if debug_show_parse: print(f"[{ti}] found orot operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "emit":
                if debug_show_parse: print(f"[{ti}] found emit operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "cr":
                if debug_show_parse: print(f"[{ti}] found cr operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "+!":
                if debug_show_parse: print(f"[{ti}] found +! operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "=":
                if debug_show_parse: print(f"[{ti}] found = operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "<":
                if debug_show_parse: print(f"[{ti}] found < operator")
                program_tokens.append(Token(tc, 1))
            elif tc == ">":
                if debug_show_parse: print(f"[{ti}] found > operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "and":
                if debug_show_parse: print(f"[{ti}] found and operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "or":
                if debug_show_parse: print(f"[{ti}] found or operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "invert":
                if debug_show_parse: print(f"[{ti}] found invert operator")
                program_tokens.append(Token(tc, 1))

            elif tc[0] == "\"":
                if debug_show_parse: print(f"[{ti}] found string literal")
                tc_str = tc.replace("\"", "")
                if tc[-1] == "\"": # need a better way of parsing strings
                    if debug_show_parse: print(f"[{ti}] found end of string literal")
                    #tc_str = tc.replace("\"", "")
                    program_tokens.append(Token(tc_str, 2))

            elif tc == "constant":
                if debug_show_parse: print(f"[{ti}] found constant initialization keyword")
                program_tokens.append(Token(tc, 3))

            elif (program_tokens[-1].ttype == 3) and not (tc.isnumeric()):
                if debug_show_parse: print(f"[{ti}] found constant name declaration")
                variable_stack[tc] = (program_tokens[-2].val, program_tokens[-2].ttype)
            
            elif tc in variable_stack:
                program_tokens.append(Token(variable_stack[tc][0], variable_stack[tc][1]))

            else:
                assert False, f"[{ti}] found unknown keyword: {tc}"

        # Stage: Interpreting program by keywords
        for ti, tc in enumerate(program_tokens):
            if "--show-stack" in sys.argv: print(f"Stack: {main_stack}")
            if tc.ttype == 0:                   # integer
                main_stack.append(tc.val)

            elif tc.ttype == 1 and tc.val == "+": # + operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v1 + v2)

            elif tc.ttype == 1 and tc.val == "-": # - operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2 - v1)

            elif tc.ttype == 1 and tc.val == "*": # * operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2 * v1)

            elif tc.ttype == 1 and tc.val == "/": # / operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if v2 % v1 == 0: main_stack.append(int(v2 / v1))
                else: main_stack.append(v2 / v1)

            elif tc.ttype == 1 and tc.val == ".": # . operator (print)
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                print(v1, end=" ")

            elif tc.ttype == 1 and tc.val == "swap": # swap operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v2)

            elif tc.ttype == 1 and tc.val == "dup": # dup operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v1)
            
            elif tc.ttype == 1 and tc.val == "drop": # drop operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                main_stack.pop()

            elif tc.ttype == 1 and tc.val == "over": # over operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2)
                main_stack.append(v1)
                main_stack.append(v2)

            elif tc.ttype == 1 and tc.val == "rot": # rot operator
                assert len(main_stack) >= 3, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                v3 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v2)
                main_stack.append(v3)

            elif tc.ttype == 1 and tc.val == "emit": # emit operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                v1 = main_stack.pop()
                print(chr(v1), end="")

            elif tc.ttype == 1 and tc.val == "cr": # cr operator
                #assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti}"
                print("\n", end="")

            elif tc.ttype == 1 and tc.val == "+!":
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc.val}"
                #v1 = main_stack.pop()
                #v2 = main_stack.pop()
                assert False, "+! not implemented"

            elif tc.ttype == 1 and tc.val == "=":
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if v1 == v2: main_stack.append(-1)
                else: main_stack.append(0)

            elif tc.ttype == 1 and tc.val == "<":
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if v2 < v1: main_stack.append(-1)
                else: main_stack.append(0)

            elif tc.ttype == 1 and tc.val == ">":
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if v1 < v2: main_stack.append(-1)
                else: main_stack.append(0)

            elif tc.ttype == 1 and tc.val == "and":
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if (v1 == -1 and v2 == -1): main_stack.append(-1)
                else: main_stack.append(0)

            elif tc.ttype == 1 and tc.val == "or":
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if (v1 == -1 or v2 == -1): main_stack.append(-1)
                else: main_stack.append(0)

            elif tc.ttype == 1 and tc.val == "invert":
                v1 = main_stack.pop()
                if v1 == -1: main_stack.append(0)
                elif v1 == 0: main_stack.append(-1)
                else: assert False, f"Error: Not a boolean operator. {ti, tc.val}"

            elif tc.ttype == 2:                   # string literal
                main_stack.append(tc.val)
                #print(f"[{ti}] Stack: {main_stack}")

            elif tc.ttype == 3:
                j = 0

            else: assert False, "Unreachable"
        print(main_stack)
        #assert len(main_stack) == 0, "Error: Unconsumed data on stack"

if __name__=="__main__": main()