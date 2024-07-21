import os
import subprocess
import filecmp

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

total = 0
ok = 0
failed = 0


# Define the path to the "tests" folder
tests_folder = "tests"

# Define the command to be executed in each subfolder
command_file = "command"

# Define the file to compare stdout with
expected_file = "expected"

output_file = "output"

error_file = "error"

# Function to run a command in a given folder and capture its stdout
def run_command_in_folder(folder):
    try:
        # Change the current working directory to the subfolder
        os.chdir(folder)
        
        # Read the command from the command_file
        with open(command_file, 'r') as cmd_file:
            command = cmd_file.read().strip()
        
        # Run the command and capture its stdout
        expected = subprocess.run("../../"+command+">"+output_file, shell=True, text=True, timeout=10.0,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        with open(error_file, mode='w') as f:
            f.write(expected.stderr)
        return expected.stdout
    except Exception as e:
        return None


# Iterate through subfolders in the "tests" folder
test_folder_list = os.listdir(tests_folder)
test_folder_list.sort()
for subfolder in test_folder_list:
    subfolder_path = os.path.join(tests_folder, subfolder)
    total +=1
    
    # Check if the subfolder is actually a directory
    if os.path.isdir(subfolder_path):
        print(f"[{total}] {subfolder_path}")
        
        # Run the command in the subfolder and capture its stdout and stderr
        stdout = run_command_in_folder(subfolder_path)
        
        if stdout is not None:
            # Compare stdout with the "expected" file
            expected_file_path = os.path.join(subfolder_path, expected_file)
            if os.path.isfile(expected_file):
                # Check if the stdout matches the content of the "expected" file
                with open(expected_file, 'r') as file:
                    expected_value = file.readlines()
                with open(output_file, 'r') as file:
                    actual_value = file.readlines()
                is_matching = sorted(expected_value) == sorted(actual_value)
                    
                # is_matching = filecmp.cmp(expected_file, output_file, shallow=False)
                if is_matching:
                    ok+=1
                    print(f" [{bcolors.OKGREEN}OK{bcolors.ENDC}]")
                else:
                    print(f"Test failed: stdout does not match the content of the '{expected_file}' file.")
                    print(f" [{bcolors.FAIL}FAIL{bcolors.ENDC}]")

            else:
                print(f"Test failed: '{expected_file}' file not found in the subfolder.")
        else:
            print(f"[{bcolors.FAIL}FAIL{bcolors.ENDC}] Test failed: Timeout or other command execution error")

        # Move back to the parent directory
        os.chdir('..')
    else:
        print("Test failed: Subfolder is not a directory.")
    os.chdir('..')
print("\nAll tests completed.\n")

failed = total - ok

print(f"Results: [{bcolors.BOLD}{total}{bcolors.ENDC}][{bcolors.OKGREEN}{ok}{bcolors.ENDC}][{bcolors.FAIL}{failed}{bcolors.ENDC}]")