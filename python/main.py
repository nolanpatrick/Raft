"""
Python experimental implementation for proof-of-concept. Bits and pieces will be ported to C periodically throughout development.


TODO: Definitions, string-print (.")
            
"""

class Token:
    def __init__(self, val, ttype):
        self.val = val
        self.ttype = ttype

def main():
    print("[INFO] *** PHORTH: Python test implementation for North ***")

    filename = "C:\\data\\program.f"

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
        print(token_list)

        program_tokens = []
        main_stack = [] # this will be the main stack for the program.

        # Stage: parsing file, taking note of what types are contained
        for ti, tc in enumerate(token_list):
            if tc.isnumeric():
                print(f"[{ti}] found numeric")
                if '.' in tc: program_tokens.append(Token(float(tc), 0))
                else: program_tokens.append(Token(int(tc), 0))
            elif tc[0] == "-" and tc[1:].isnumeric():
                print(f"[{ti}] found numeric")
                if '.' in tc: program_tokens.append(Token(float(tc), 0))
                else: program_tokens.append(Token(int(tc), 0))
            elif tc == "+":
                print(f"[{ti}] found + operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "-":
                print(f"[{ti}] found - operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "*":
                print(f"[{ti}] found * operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "/":
                print(f"[{ti}] found / operator")
                program_tokens.append(Token(tc, 1))
            elif tc == ".":
                print(f"[{ti}] found . operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "swap":
                print(f"[{ti}] found swap operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "dup":
                print(f"[{ti}] found dup operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "drop":
                print(f"[{ti}] found drop operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "over":
                print(f"[{ti}] found over operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "rot":
                print(f"[{ti}] found orot operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "emit":
                print(f"[{ti}] found emit operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "cr":
                print(f"[{ti}] found cr operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "=":
                print(f"[{ti}] found = operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "<":
                print(f"[{ti}] found < operator")
                program_tokens.append(Token(tc, 1))
            elif tc == ">":
                print(f"[{ti}] found > operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "and":
                print(f"[{ti}] found and operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "or":
                print(f"[{ti}] found or operator")
                program_tokens.append(Token(tc, 1))
            elif tc == "invert":
                print(f"[{ti}] found invert operator")
                program_tokens.append(Token(tc, 1))

            elif tc[0] == "\"":
                tc_str = tc.replace("\"", "")
                if tc[-1] == "\"": # need a better way of parsing strings
                    print(f"[{ti}] found string literal")
                    #tc_str = tc.replace("\"", "")
                    program_tokens.append(Token(tc_str, 2))

            else:
                assert False, f"[{ti}] found unknown keyword"

        # Stage: Interpreting program by keywords
        for ti, tc in enumerate(program_tokens):
            if tc.ttype == 0:                   # integer
                main_stack.append(tc.val)

            elif tc.ttype == 1 and tc.val == "+": # + operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v1 + v2)

            elif tc.ttype == 1 and tc.val == "-": # - operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2 - v1)

            elif tc.ttype == 1 and tc.val == "*": # * operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2 * v1)

            elif tc.ttype == 1 and tc.val == "/": # / operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                if v2 % v1 == 0: main_stack.append(int(v2 / v1))
                else: main_stack.append(v2 / v1)

            elif tc.ttype == 1 and tc.val == ".": # . operator (print)
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                print(v1, end=" ")

            elif tc.ttype == 1 and tc.val == "swap": # swap operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v2)

            elif tc.ttype == 1 and tc.val == "dup": # dup operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v1)
            
            elif tc.ttype == 1 and tc.val == "drop": # drop operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc}"
                main_stack.pop()

            elif tc.ttype == 1 and tc.val == "over": # over operator
                assert len(main_stack) >= 2, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                main_stack.append(v2)
                main_stack.append(v1)
                main_stack.append(v2)

            elif tc.ttype == 1 and tc.val == "rot": # rot operator
                assert len(main_stack) >= 3, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                v2 = main_stack.pop()
                v3 = main_stack.pop()
                main_stack.append(v1)
                main_stack.append(v2)
                main_stack.append(v3)

            elif tc.ttype == 1 and tc.val == "emit": # emit operator
                assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti, tc}"
                v1 = main_stack.pop()
                print(chr(v1), end="")

            elif tc.ttype == 1 and tc.val == "cr": # cr operator
                #assert len(main_stack) >= 1, f"Error: stack contents not sufficient for operation. {ti}"
                print("\n", end="")

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
                else: assert False, f"Error: Not a boolean operator. {ti, tc}"

            elif tc.ttype == 2:                   # string literal
                main_stack.append(tc.val)
                #print(f"[{ti}] Stack: {main_stack}")
            else:
                assert False, "Unreachable"

if __name__=="__main__": main()