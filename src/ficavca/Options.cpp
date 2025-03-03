#include "ficavca/Options.hpp"
#include <boost/program_options.hpp>
#include <iostream>

namespace graphs {

namespace ficavca {

std::optional<Config> readConfig(int argc, const char *argv[]) {
  namespace po = boost::program_options;
  po::options_description GenericOpts("Generic options");
  GenericOpts.add_options()("help,h", "help message");
  po::options_description ConfigOpts("Configuration");
  ConfigOpts.add_options()(
      "dem,d", po::value<bool>()->implicit_value(true)->default_value(false),
      "democracy")("seed,s", po::value<unsigned int>(),
                   "seed")("input,i", po::value<std::string>(), "input graph");
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
    if (VM.count("dem")) {
      Cfg.Democracy.emplace(VM["dem"].as<bool>());
    }
    if (VM.count("seed")) {
      Cfg.Seed.emplace(VM["seed"].as<unsigned int>());
    }
    if (VM.count("input")) {
      Cfg.GraphInput = VM["input"].as<std::string>();
    }
  } catch (...) {
    std::cout << Opts << std::endl;
    return {};
  }
  return Cfg;
}

} // namespace ficavca

} // namespace graphs
