import subprocess
import re
import random
from pathlib import Path
import matplotlib.pyplot as plt

repo_path = Path(__file__).parent.parent
llvm_repo_path = repo_path.parent / "llvm-project"

class Benchmark:
    def __init__(self, pack_names: list[str], path: str, res_file: str):
        self.pack_names = pack_names
        self.path = path
        self.res_file = res_file


class TestPack:
    def __init__(self, pack_name: str):
        self.pack_name = pack_name
        self.test_funcs: list[TestFunc] = []


class TestFunc:
    def __init__(self, func_name: str):
        self.func_name = func_name
        self.test_results: list[TestResult] = []


class TestResult:
    def __init__(
        self,
        regalloc: str,
        virt_regs_count: int,
        virt_regs_num: int,
        phys_regs_num: int
    ):
        self.regalloc = regalloc
        self.virt_regs_count = virt_regs_count
        self.virt_regs_num = virt_regs_num
        self.phys_regs_num = phys_regs_num


def run_command(command: list[str]) -> str:
    result = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        shell=True,
        timeout=1
    )
    return result.stdout


def draw_results(test_packs: list[TestPack]):
    random.seed(0)
    colors = [f'#{random.randint(0, 0xFFFFFF):06x}' for _ in range(4)]
    for pack in test_packs:
        fig, axs = plt.subplots(len(pack.test_funcs), figsize=(8, 5))
        fig.suptitle(f"{pack.pack_name}")
        for j, func in enumerate(pack.test_funcs):
            heights = [result.phys_regs_num for result in func.test_results]
            labels = [result.regalloc for result in func.test_results]
            bars = axs[j].bar(labels, heights, color=colors)
            axs[j].set_title(get_short_name(func.func_name))
            axs[j].set_xlabel("Regallocs")
            for bar in bars:
                yval = bar.get_height()
                axs[j].text(bar.get_x() + bar.get_width()/2, yval, int(yval), ha='center', va='bottom')
    plt.show()

def get_short_name(func_name: str) -> str:
    file_name = func_name.split("/")[-1]
    short_name = file_name.split(".")[-2]
    return short_name


def print_results(benchmark: Benchmark, test_packs: list[TestPack]) -> None:
    res_file = benchmark.res_file
    with open(res_file, "w", encoding="UTF-8") as f:
        for test_pack in test_packs:
            f.write(f"{test_pack.pack_name}\n")
            for test_func in test_pack.test_funcs:
                f.write(f"{test_func.func_name}\n")
                for test_result in test_func.test_results:
                    f.write(f"{test_result.regalloc}: {test_result.phys_regs_num}\n")


def measure(benchmark: Benchmark) -> list[TestPack]:
    pack_names = benchmark.pack_names
    path = benchmark.path
    regallocs = ["pbqp", "basic", "greedy"]
    test_packs: list[TestPack] = []
    for pack_name in pack_names:
        test_pack = TestPack(pack_name)
        for regalloc in regallocs:
            command = f"{llvm_repo_path}/build/bin/llc -mtriple=riscv64-unknown-linux-gnu -regalloc {regalloc} {path}/{pack_name}"
            output = run_command(command)
            matches = re.findall(
                r"Virt regs count: (\d+)\nName: (.+?)\nVirt regs num: (\d+)\nPhys regs num: (\d+)",
                output
            )
            for match in matches:
                virt_regs_count, func_name, virt_regs_num, phys_regs_num = match
                virt_regs_count = int(virt_regs_count)
                virt_regs_num = int(virt_regs_num)
                phys_regs_num = int(phys_regs_num)

                test_result = TestResult(
                    regalloc,
                    virt_regs_count,
                    virt_regs_num,
                    phys_regs_num
                )
                found = False
                for test_func in test_pack.test_funcs:
                    if test_func.func_name == func_name:
                        test_func.test_results.append(test_result)
                        found = True
                if not found:
                    test_func = TestFunc(func_name)
                    test_func.test_results.append(test_result)
                    test_pack.test_funcs.append(test_func)

        for test_func in test_pack.test_funcs:
            command = f"{repo_path}/build/Ficavca -d -i {test_func.func_name}"
            output = run_command(command)
            match = re.findall(r"Nodes count (\d+)\nColor degree (\d+)", output)
            if not match[0]:
                print(f"fail to get output {command}\n")
                continue
            virt_regs_count, phys_regs_num = match[0]
            virt_regs_count = int(virt_regs_count)
            phys_regs_num = int(phys_regs_num)
            test_result = TestResult("myregalloc", virt_regs_count, virt_regs_count, phys_regs_num)
            test_func.test_results.append(test_result)
        test_packs.append(test_pack)
    return test_packs


def main():
    coremark = Benchmark(
        [
            "core_list_join.ll",
            "core_main.ll",
            "core_matrix.ll",
            "core_portme.ll",
            "core_state.ll",
            "core_util.ll"
        ],
        f"{llvm_repo_path}/coremark",
        f"{repo_path}/results/coremark.txt"   
    )
    dhry = Benchmark(
        ["dhry_1.ll", "dhry_2.ll"],
        f"{llvm_repo_path}/benchmark-dhrystone",
        f"{repo_path}/results/dhry.txt"   
    )

    test_packs = measure(coremark)
    print_results(coremark, test_packs)
    test_packs = measure(dhry)
    print_results(dhry, test_packs)

if __name__ == "__main__":
    main()
