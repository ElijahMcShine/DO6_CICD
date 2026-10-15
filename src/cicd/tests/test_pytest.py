import os
import subprocess
import sys
import pytest


def collect_info(my, og, pwd):
    test_presets = "test_presets.txt"
    results = []
    
    os.chdir(pwd)
    if not os.path.isfile(test_presets):
        print("File with arg presets 'test_presets.txt' not found")
        sys.exit(1)
            
    if not os.path.isfile(my):
        print(f"Programm to tests not found")
        sys.exit(1)

    with open(test_presets) as tests:
        
        for test in tests:
            if not test.rstrip():
                print('----')
                continue

            args = test.rstrip().split(' ')
            my_prog_result = subprocess.run([my] + args, capture_output=True).stdout.strip()
            orig_prog_result = subprocess.run([og] + args, capture_output=True).stdout.strip()

            new_line = {'arguments':args, 'input':my_prog_result, 'expected_result':orig_prog_result}
            results.append(new_line)
    os.chdir("..")
    return results

@pytest.mark.parametrize('test_data', collect_info("./s21_cat", "cat", "./cat_tests"))
def test_cat(test_data):
    assert test_data['input'] == test_data['expected_result'], \
    f'Разница при вызове {test_data['arguments']}'

@pytest.mark.parametrize('test_data', collect_info("./s21_grep", "grep", "./grep_tests"))
def test_grep(test_data):
    assert test_data['input'] == test_data['expected_result'], \
    f'Разница при вызове {test_data['arguments']}'


