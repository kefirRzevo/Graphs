#include "clavca/Options.hpp"
#include <boost/program_options.hpp>
#include <iostream>

namespace graphs {

namespace clavca {

std::optional<Config> readConfig(int argc, const char *argv[]) {
  namespace po = boost::program_options;
  po::options_description GenericOpts("Generic options");
  GenericOpts.add_options()("help,h", "help message");
  auto DefaultInput = std::string{"graph.txt"};
  po::options_description ConfigOpts("Configuration");
  ConfigOpts.add_options()("a", po::value<double>(), "a param")(
      "b", po::value<double>(), "b param")("e", po::value<double>(), "e param")(
      "s", po::value<unsigned int>(), "seed")(
      "i", po::value<std::string>()->default_value(DefaultInput), "input graph");
  auto Opts = po::options_description{};
  Opts.add(ConfigOpts).add(GenericOpts);
  Config Cfg;
  po::variables_map VM;
  try {
    auto Parser = po::command_line_parser(argc, argv);
    po::store(Parser.options(Opts).run(), VM);
    po::notify(VM);
    if (VM.count("help")) {
      std::cout << Opts << std::endl;
      return {};
    }
    if (VM.count("a")) {
      Cfg.ParamA.emplace(VM["a"].as<double>());
    }
    if (VM.count("b")) {
      Cfg.ParamB.emplace(VM["b"].as<double>());
    }
    if (VM.count("e")) {
      Cfg.ParamE.emplace(VM["e"].as<double>());
    }
    if (VM.count("s")) {
      Cfg.Seed.emplace(VM["s"].as<unsigned int>());
    }
    Cfg.GraphInput = DefaultInput;
    if (VM.count("i")) {
      Cfg.GraphInput = VM["i"].as<std::string>();
    }
  } catch (const po::required_option &e) {
    std::cout << Opts << std::endl;
    return {};
  }
  return Cfg;
}

} // namespace clavca

} // namespace graphs
