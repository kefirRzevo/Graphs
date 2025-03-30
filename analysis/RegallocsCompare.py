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
    dhry = Benchmark(
        ["dhry_1.ll", "dhry_2.ll"],
        f"{llvm_repo_path}/benchmark-dhrystone",
        f"{repo_path}/results/dhry.txt"
    )
    parest_r510 = Benchmark(
        [
            "tria.ll",
            "conditional_ostream.ll",
            "fe_dgq.ll",
            "timer.ll",
            "dof_faces.ll",
            "tria_boundary.ll",
            "chunk_sparsity_pattern.ll",
            "tensor.ll",
            "grid_refinement.ll",
            "mapping_cartesian.ll",
            "field_discretization.ll",
            "matrix_lib.ll",
            "fe_nedelec_3d.ll",
            "data_out_rotation.ll",
            "state_discretization.ll",
            "precondition_block.ll",
            "petsc_parallel_sparse_matrix.ll",
            "data_postprocessor.ll",
            "geometry_info.ll",
            "petsc_precondition.ll",
            "petsc_vector_base.ll",
            "fe_values.ll",
            "mg_smoother.ll",
            "petsc_full_matrix.ll",
            "fe_abf.ll",
            "trilinos_vector_base.ll",
            "polynomials_abf.ll",
            "path_search.ll",
            "tensor_product_polynomials.ll",
            "mg_dof_accessor.ll",
            "fe_collection.ll",
            "grid_tools.ll",
            "data_out_faces.ll",
            "fe_tools.ll",
            "lapack_full_matrix.ll",
            "convergence_table.ll",
            "petsc_vector.ll",
            "fe_raviart_thomas.ll",
            "forward.ll",
            "sparsity_pattern.ll",
            "fe_raviart_thomas_nodal.ll",
            "base.ll",
            "tria_boundary_lib.ll",
            "memory_consumption.ll",
            "tensor_function.ll",
            "grid_out.ll",
            "mapping_q_eulerian.ll",
            "parameter_handler.ll",
            "matrix_out.ll",
            "derivative_approximation.ll",
            "all_dimensions.ll",
            "trilinos_precondition_block.ll",
            "mapping.ll",
            "mapping_q.ll",
            "function_parser.ll",
            "all_dimensions.ll",
            "fe_q.ll",
            "function_lib_cutoff.ll",
            "fe_poly.ll",
            "synthetic_data.ll",
            "vector_view.ll",
            "sparse_decomposition.ll",
            "histogram.ll",
            "thread_management.ll",
            "fe_dgp.ll",
            "polynomials_raviart_thomas.ll",
            "measurements.ll",
            "polynomial.ll",
            "time_dependent.ll",
            "factories.ll",
            "forward_solver_parameters.ll",
            "tria_accessor.ll",
            "grid_transfer.ll",
            "block_vector.ll",
            "sparsity_tools.ll",
            "function_derivative.ll",
            "trilinos_solver_block.ll",
            "tria_levels.ll",
            "quadrature_selector.ll",
            "fe_dgp_monomial.ll",
            "petsc_solver.ll",
            "all_dimensions.ll",
            "petsc_matrix_base.ll",
            "dof_objects.ll",
            "dof_renumbering.ll",
            "experiment_description.ll",
            "mapping_collection.ll",
            "solver.ll",
            "petsc_sparse_matrix.ll",
            "all_dimensions.ll",
            "geometry.ll",
            "full_matrix.ll",
            "petsc_parallel_block_sparse_matrix.ll",
            "function_lib.ll",
            "table_handler.ll",
            "mg_dof_handler.ll",
            "grid_in.ll",
            "fe_system.ll",
            "parsed_function.ll",
            "compressed_sparsity_pattern.ll",
            "block_sparsity_pattern.ll",
            "sparse_vanka.ll",
            "sparse_direct.ll",
            "all_dimensions.ll",
            "data_out_stack.ll",
            "block_matrix_array.ll",
            "message_log.ll",
            "error_estimator.ll",
            "log.ll",
            "trilinos_sparsity_pattern.ll",
            "data_out.ll",
            "tridiagonal_matrix.ll",
            "mapping_q1.ll",
            "trilinos_block_vector.ll",
            "global_parameters.ll",
            "forward_solver_evaluators.ll",
            "evaluations.ll",
            "statistics.ll",
            "trilinos_precondition.ll",
            "mapping_q1_eulerian.ll",
            "utilities.ll",
            "sparse_matrix_ez.ll",
            "fe.ll",
            "block_sparse_matrix.ll",
            "trilinos_solver.ll",
            "constraint_matrix.ll",
            "auto_derivative_function.ll",
            "graphical_display.ll",
            "intergrid_map.ll",
            "vector.ll",
            "quadrature_lib.ll",
            "persistent_tria.ll",
            "master.ll",
            "compressed_set_sparsity_pattern.ll",
            "trilinos_sparse_matrix.ll",
            "compressed_simple_sparsity_pattern.ll",
            "boundary_sources_phantom.ll",
            "polynomial_space.ll",
            "symmetric_tensor.ll",
            "boundary_sources_planarz8.ll",
            "petsc_parallel_vector.ll",
            "mg_transfer_block.ll",
            "targets.ll",
            "all_dimensions.ll",
            "fe_q_hierarchical.ll",
            "all_dimensions.ll",
            "subscriptor.ll",
            "petsc_block_sparse_matrix.ll",
            "precondition_block_ez.ll",
            "multiple_experiments.ll",
            "grid_generator.ll",
            "sparse_mic.ll",
            "me_parameters.ll",
            "mg_base.ll",
            "sparse_matrix.ll",
            "dof_tools.ll",
            "mapping_c1.ll",
            "fe_field_function.ll",
            "trilinos_block_sparse_matrix.ll",
            "fe_nedelec_2d.ll",
            "mg_dof_tools.ll",
            "boost_threads.ll",
            "grid_reordering.ll",
            "tools.ll",
            "control.ll",
            "mg_transfer_prebuilt.ll",
            "fe_nedelec_1d.ll",
            "trilinos_vector.ll",
            "swappable_vector.ll",
            "tria_faces.ll",
            "fe_poly_tensor.ll",
            "matrices.ll",
            "tria_objects.ll",
            "config.ll",
            "dof_handler.ll",
            "exceptions.ll",
            "function_time.ll",
            "sparse_ilu.ll",
            "all_dimensions.ll",
            "solution_transfer.ll",
            "step_length_control.ll",
            "dof_levels.ll",
            "bounds.ll",
            "block_sparse_matrix_ez.ll",
            "measurement_weights.ll",
            "flow_function.ll",
            "job_identifier.ll",
            "solver_control.ll",
            "fe_nedelec.ll",
            "data_out_base.ll",
            "problem_description.ll",
            "fe_dgp_nonparametric.ll",
            "me_slave.ll",
            "polynomials_p.ll",
            "coefficient.ll",
            "factory.ll",
            "polynomials_bdm.ll",
            "function.ll",
            "newton_method.ll",
            "multithread_info.ll",
            "top_level.ll",
            "me_tomography.ll",
            "chunk_sparse_matrix.ll",
            "fe_data.ll",
            "dof_accessor.ll",
            "all_dimensions.ll",
            "mg_transfer_component.ll",
            "quadrature.ll",
            "all_dimensions.ll",
            "petsc_parallel_block_vector.ll",
            "vector_memory.ll",
        ],
        "/home/timur/timur/regalloc/build-tests/SPEC-CFP2017rate-test/External/SPEC/CFP2017rate/510.parest_r",
        f"{repo_path}/results/510.txt"        
    )
    test_packs = measure(parest_r510)
    print_results(parest_r510, test_packs)

    # test_packs = measure(coremark)
    # print_results(coremark, test_packs)
    # test_packs = measure(dhry)
    # print_results(dhry, test_packs)

if __name__ == "__main__":
    main()
