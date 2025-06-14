import matplotlib.pyplot as plt
from pathlib import Path

repo_path = Path(__file__).parent.parent

class Measure:
    def __init__(self, diff: int, count: int):
        self.diff = diff
        self.count = count

class Test:
    def __init__(self, vcount: int, ratio: float, measures: dict[int, int]):
        self.vcount = vcount
        self.ratio = ratio
        self.measures = measures

def read_tests(file_path) -> list[Test]:
    tests: list[Test] = []
    with open(file_path, "r") as file:
        current_vcount = None
        current_ratio = None
        measures: dict[int, int] = {}

        for line in file:
            line = line.strip()
            if line.startswith("VCount:"):
                if measures:
                    tests.append(Test(current_vcount, current_ratio, measures))
                    measures = {}
                
                parts = line.split()
                current_vcount = int(parts[1])
                current_ratio = float(parts[3])
            elif line.startswith("Diff:"):
                parts = line.split()
                diff = int(parts[1])
                count = int(parts[3])
                measures[diff] = count

        if measures:
            tests.append(Test(current_vcount, current_ratio, measures))
    return tests

def plot_data(tests: list[Test], path_to_png: Path, fix_vcount = None, fix_ratio = None) -> None:
    plt.figure(figsize=(10, 6))

    for test in tests:
        diffs = list(test.measures.keys())
        values = list(test.measures.values())
        if test.vcount == fix_vcount:
            plt.plot(diffs, values, marker="o", label=f"Ratio: {test.ratio}")
        elif test.ratio == fix_ratio:
            plt.plot(diffs, values, marker="o", label=f"Vertex Count: {test.vcount}")

    if fix_vcount is not None:
        plt.title(f"Greedy and FICAVCA compare on randomized graphs for VertexCount={fix_vcount}",fontsize=15)
    elif fix_ratio is not None:
        plt.title(f"Greedy and FICAVCA compare on randomized graphs for Ratio={fix_ratio}",fontsize=15)
    plt.xlabel("Color degree difference between Greedy and FICAVCA",fontsize=15)
    plt.ylabel("Randomized graphs count",fontsize=15)
    plt.xticks(fontsize=14)
    plt.yticks(fontsize=14)
    plt.legend(fontsize=14)
    plt.grid()
    plt.savefig(path_to_png)

if __name__ == "__main__":
    file_path = repo_path / "results" / "distribution.txt"
    tests = read_tests(file_path)
    plot_data(tests, repo_path / "results" / "distribution-fix-vcount.png", fix_vcount=300)
    plot_data(tests, repo_path / "results" / "distribution-fix-ratio.png", fix_ratio=0.5)
