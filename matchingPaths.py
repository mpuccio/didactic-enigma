import re
import subprocess
import argparse

def run_find_command(command):
    # Run the find command and capture the output
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running command: {command}")
        return []
    # Split the output into lines and return as a list
    return result.stdout.strip().split('\n')

def extract_run_number(path):
    # Regular expression to find the run number starting with '5'
    match = re.search(r'/5\d{5}/', path)
    if match:
        return match.group(0)[1:-1]  # Remove the leading and trailing '/'
    return None

def main(periodName):
    # Define the find commands with the variable periodName part
    find_command1 = f"find . -type f -wholename '*/{periodName}/*/apass4/*/AnalysisResults_fullrun.root'"
    find_command2 = f"find . -type f -wholename '*/{periodName}/*/apass4_skimmed/*/AnalysisResults_fullrun.root'"

    # Create the lists of paths using the find commands
    list1 = run_find_command(find_command1)
    list2 = run_find_command(find_command2)

    print(f"Found {len(list1)} skimmed outputs vs {len(list2)} original outputs")

    # Extract run numbers and paths into dictionaries
    dict1 = {extract_run_number(path): path for path in list1 if extract_run_number(path)}
    dict2 = {extract_run_number(path): path for path in list2 if extract_run_number(path)}

    # Find common run numbers
    common_run_numbers = set(dict1.keys()).intersection(dict2.keys())

    # Create a new list containing tuples of matching paths
    matched_paths = [(dict1[run_number], dict2[run_number]) for run_number in common_run_numbers]

    # Write the matched paths to a file
    with open(f"{periodName}.txt", 'w') as f:
        for pair in matched_paths:
            f.write(f"{pair[0]},{pair[1]}\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Find and match paths based on run numbers.')
    parser.add_argument('periodName', type=str, help='The periodName part of the path (e.g., LHC23v).')
    
    args = parser.parse_args()
    main(args.periodName)

