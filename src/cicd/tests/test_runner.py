import os
import subprocess
import sys
from colors import Color

test_presets = "test_presets.txt"

def run(my, og, pwd):
    os.chdir(pwd)
    if not os.path.isfile(test_presets):
        print("File with arg presets 'test_presets.txt' not found")
        sys.exit(1)
        
    if not os.path.isfile(my):
        print(f"Пожалуйста, положи свой {Color.BOLD}{my[2:]}{Color.NC} в {Color.BOLD}{pwd}{Color.NC}")
        sys.exit(1)

    with open(test_presets) as tests:
        for test in tests:
            if not test.rstrip():
                print('----')
                continue

            args = test.rstrip().split(' ')
            with open("out1", "w") as out1:
                subprocess.run([my] + args, stdout=out1)
            with open("out2", "w") as out2:
                subprocess.run([og] + args, stdout=out2)

            diff_res = subprocess.run(["diff", "-q", "out1", "out2"], capture_output=True, text=True)
            if not diff_res.stdout:
                print(f"{Color.GREEN}PASS:{Color.NC} ", end="")
                print(" ".join(args))
            else:
                print(f"{Color.RED}FAIL:{Color.NC} ", end="")
                print(" ".join(args))
                print("Expected: ")
                print(Color.BLUE, end='')
                with open("out2", "r") as f:
                    print(f.read(), end='')
                print(Color.NC, end='')
                print("Got: ")
                print(Color.YELLOW, end='')
                with open("out1", "r") as f:
                    print(f.read(), end='')
                print(Color.NC, end='')
                subprocess.run(["rm", "-rf", "out1"])
                subprocess.run(["rm", "-rf", "out2"])
                sys.exit(1)

            subprocess.run(["rm", "-rf", "out1"])
            subprocess.run(["rm", "-rf", "out2"])