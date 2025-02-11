import subprocess
import numpy as np
import pandas as pd
from concurrent.futures import ThreadPoolExecutor

# Function to run the executable and get the number of stages
def run_executable(A, B, E, runs=500):
    stages_list = []
    for _ in range(runs):
        result = subprocess.run(['./build/graphs', str(A), str(B), str(E)], capture_output=True, text=True)
        output = result.stdout.strip()
        
        # Extract the number of stages from the output
        if output.startswith("Stages"):
            stages = int(output.split()[1])
            stages_list.append(stages)
    
    # Calculate the average number of stages
    return np.mean(stages_list)

# Parameters
A_values = np.arange(0.0, 1.05, 0.1)
B_values = np.arange(0.0, 1.05, 0.1)
E_values = np.arange(0.0, 1.05, 0.1)

# Prepare to store results
results = []

# Use ThreadPoolExecutor to run in parallel
with ThreadPoolExecutor() as executor:
    futures = {}
    for A in A_values:
        for B in B_values:
            for E in E_values:
                # Submit a job to the executor
                futures[executor.submit(run_executable, A, B, E)] = (A, B, E)
    
    for future in futures:
        A, B, E = futures[future]
        average_stages = future.result()
        results.append((f"{A:.2f}", f"{B:.2f}", f"{E:.2f}", f"{average_stages:7.3f}"))

# Create a DataFrame
df = pd.DataFrame(results, columns=['A', 'B', 'E', 'AverageStages'])

# Sort the DataFrame by AverageStages
df['AverageStages'] = df['AverageStages'].astype(float)  # Convert to float for sorting
df = df.sort_values(by='AverageStages')

# Save to a text file
df.to_csv('average_stages.txt', index=False, sep='\t')

print("Average stages have been calculated, sorted, and saved to 'average_stages.txt'.")
