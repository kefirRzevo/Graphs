import re
from pathlib import Path
import matplotlib.pyplot as plt

repo_path = Path(__file__).parent.parent

class Result:
    def __init__(self,
                 func_name: str,
                 pbqp_count: int,
                 basic_count: int,
                 greedy_count: int,
                 myregalloc_count: int
                 ) -> None:
        self.func_name = func_name
        self.pbqp_count = pbqp_count
        self.basic_count = basic_count
        self.greedy_count = greedy_count
        self.myregalloc_count = myregalloc_count

    def average(self) -> int:
        return (self.pbqp_count)

def main() -> None:
    file_path = repo_path / "results" / "510.txt"

    results: list[Result] = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

    func_idx = 0
    for i in range(len(lines)):
        func_name_match = re.match(r'^(.*\.txt)', lines[i])
        if func_name_match:
            func_name = func_name_match.group(1)
            if (i + 4 < len(lines) and
                re.match(r'pbqp: \d+', lines[i + 1]) and
                re.match(r'basic: \d+', lines[i + 2]) and
                re.match(r'greedy: \d+', lines[i + 3]) and
                re.match(r'myregalloc: \d+', lines[i + 4])):
                pbqp_count = int(lines[i + 1].split(': ')[1])
                basic_count = int(lines[i + 2].split(': ')[1])
                greedy_count = int(lines[i + 3].split(': ')[1])
                myregalloc_count = int(lines[i + 4].split(': ')[1])
                # Too much data - not interesting
                regs_min = 3
                if pbqp_count < regs_min or basic_count < regs_min or basic_count < regs_min or myregalloc_count < regs_min:
                    continue
                # Spills are currently unsupported
                regs_max = 16
                if pbqp_count > regs_max or basic_count > regs_max or basic_count > regs_max or myregalloc_count > regs_max:
                    continue
                results.append(Result(
                    func_name,
                    pbqp_count,
                    basic_count,
                    greedy_count,
                    myregalloc_count
                ))
    results.sort(key=lambda r: r.average())
    idxs = range(len(results))
    pbqps = [result.pbqp_count for result in results]
    greedies = [result.greedy_count for result in results]
    myregallocs = [result.myregalloc_count for result in results]
    basics = [result.basic_count for result in results]
    plt.figure(figsize=(10, 6))
    plt.plot(idxs, pbqps, marker="o", label="pbqp")
    plt.plot(idxs, greedies, marker="o", label="greedy")
    plt.plot(idxs, myregallocs, marker="o", label="myregalloc")
    plt.plot(idxs, basics, marker="o", label="basic")
    plt.title("Comparison of existing regallocs in llvm on benchmark SPEC CPU2017")
    plt.xlabel("510.parest_r functions' indexes")
    plt.ylabel("Color degree for different regallocs")
    plt.legend()
    plt.grid()
    output_file_path = repo_path / "results" / "510.png"
    plt.savefig(output_file_path)

if __name__ == "__main__":
    main()
