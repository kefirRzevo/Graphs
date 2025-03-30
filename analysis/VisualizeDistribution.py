import matplotlib.pyplot as plt
from pathlib import Path

repo_path = Path(__file__).parent.parent

def read_data(file_path):
    data = []
    with open(file_path, "r") as file:
        current_vcount = None
        current_ratio = None
        current_data = {}

        for line in file:
            line = line.strip()
            if line.startswith("VCount:"):
                if current_data:
                    data.append((current_vcount, current_ratio, current_data))
                    current_data = {}
                
                parts = line.split()
                current_vcount = int(parts[1])
                current_ratio = float(parts[3])
            elif line.startswith("Diff:"):
                parts = line.split()
                diff = int(parts[1])
                count = int(parts[3])
                current_data[diff] = count

        if current_data:
            data.append((current_vcount, current_ratio, current_data))
    
    return data

def plot_data(data):
    plt.figure(figsize=(10, 6))

    for vcount, ratio, counts in data:
        if vcount != 300:
            continue
        if ratio != 0.5:
            pass
        diffs = list(counts.keys())
        values = list(counts.values())

        plt.plot(diffs, values, marker="o", label=f"VertexCount {vcount}, Ratio: {ratio}")

    plt.title("Greedy and FICAVCA compare on randomized graphs")
    plt.xlabel("Color degree difference between greedy and FICAVCA")
    plt.ylabel("Randomized graphs count")
    plt.legend()
    plt.grid()
    plt.savefig(repo_path / "results" / "distribution.png")

if __name__ == "__main__":
    file_path = repo_path / "results" / "distribution.txt"
    data = read_data(file_path)
    plot_data(data)
