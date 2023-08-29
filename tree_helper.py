import subprocess
import os

# Start a shell process
shell_process = subprocess.Popen(
    "/bin/bash",  # Replace with your desired shell (e.g., "/bin/bash", "cmd.exe" on Windows)
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,  # Use text mode to work with strings
    shell=True  # Use shell to interpret the command
)

shell_process2 = subprocess.Popen(
    "/bin/bash",  # Replace with your desired shell (e.g., "/bin/bash", "cmd.exe" on Windows)
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,  # Use text mode to work with strings
    shell=True  # Use shell to interpret the command
)

# Define a function to send a command to the shell and get the output
def send_command(command):
    shell_process.stdin.write(command + "\n")
    shell_process.stdin.flush()

    # output=''
    # while True:
    #     st=shell_process.stdout.readline()
    #     if st !='\0':
    #         output=output+st
    #     else: break

    output = shell_process.stdout.readline()
    return output

def send_command2(command):
    shell_process2.stdin.write(command + "\n")
    shell_process2.stdin.flush()
    output = shell_process2.stdout.readline()
    return output

# Example usage

send_command('./bin/b_plus_tree_printer')
while True:
    user_input = input("Enter a command (or 'exit' to quit): ")
    if user_input.lower() == 'exit':
        break

    result=''
    if user_input=='show' or user_input=='s':
        user_input='g my-tree.dot'
        result = send_command(user_input)
        os.system('dot -Tpng -O my-tree.dot')
        os.system('open -a preview ./my-tree.dot.png')

    else: result = send_command(user_input)
    print(result)

# Close the shell process
shell_process.stdin.close()
shell_process.stdout.close()
shell_process.stderr.close()
shell_process.wait()

shell_process2.stdin.close()
shell_process2.stdout.close()
shell_process2.stderr.close()
shell_process2.wait()
