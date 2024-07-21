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
error_file = 'error'
output_file = "output"

# Function to run a command in a given folder and capture its stdout
def run_command_in_folder(folder):
        # Change the current working directory to the subfolder
        os.chdir(folder)
        
        with open(command_file, 'r') as cmd_file:
            command = print(cmd_file.read().strip())
         


# Iterate through subfolders in the "tests" folder
test_folder_list = os.listdir(tests_folder)
test_folder_list.sort()
for subfolder in test_folder_list:
    print('-'*30 + '\n')
    
    print(f"\n{bcolors.OKBLUE}{subfolder}{bcolors.ENDC}")
    subfolder_path = os.path.join(tests_folder, subfolder)


    

    AHOJ = os.path.join(subfolder_path,command_file)
    with open(AHOJ, 'r') as cmd_file:
        print(cmd_file.read().strip())

    AHOJ = os.path.join(subfolder_path,output_file)
    with open(AHOJ, 'r') as cmd_file:
        print(cmd_file.read().strip())

    AHOJ = os.path.join(subfolder_path,error_file)
    with open(AHOJ, 'r') as cmd_file:
        print(bcolors.FAIL+cmd_file.read().strip()+bcolors.ENDC)

