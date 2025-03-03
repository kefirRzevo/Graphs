import subprocess
import re
import os

# Define a function to extract the number from the answer file
def extract_data_to_compare(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    # Assuming the number {z} is the only number in the file
    match = re.search(r'(\d+)/(\d+)', content)
    return int(match.group(2)) if match else None

# Define a function to run the executable and get the output
def run_executable(input_file):
    try:
        result = subprocess.run(
            ["build/Ficavca", "-d", "-i", f"{input_file}"],
            capture_output=True,
            text=True,
            check=True
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error running the executable: {e}")
        return None

# Main function to process the files
def main():
    results = []

    for stage in range(14):  # From 0 to 13
        input_file = f'graphs/core_mark{stage}.dat'
        answer_file = f'graphs/core_mark{stage}.ans'
        
        # Check if the input and answer files exist
        if not os.path.exists(input_file) or not os.path.exists(answer_file):
            print(f"Files not found: {input_file} or {answer_file}")
            continue
        
        data_to_compare = extract_data_to_compare(answer_file)
        
        # Run the executable and capture the output
        output = run_executable(input_file)
        if output is not None:
            match = re.match(r"Nodes count (\d+)\nColor degree (\d+)", output)
            if match:
                data = int(match.group(2))
                # Store the results in a dictionary
                results.append({'stage': stage, 'data': data, 'data_to_compare': data_to_compare})

    # Print the results
    for result in results:
        print(f"stage: {result['stage']}, data: {result['data']}, data_to_compare: {result['data_to_compare']}")

if __name__ == "__main__":
    main()
