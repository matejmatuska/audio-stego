#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>

#include "args.h"

#define REQUIRE_ARG(arg)                                           \
  if (i >= argc) {                                                 \
    throw std::invalid_argument("missing argument for: " + (arg)); \
  }

#define REQUIRE_OPT_ARG(opt)                                       \
  if (++i >= argc || argv[i][0] == '-') {                          \
    throw std::invalid_argument("missing argument for: " + (opt)); \
  }

using string_set = std::set<std::string>;

static void parse_opts(struct args& args,
                       int argc,
                       char* argv[],
                       string_set& required,
                       const string_set& allowed = string_set());

struct args parse_args(int argc, char* argv[])
{
  struct args args;
  if (argc < 2) {
    throw std::invalid_argument("No command specified");
  }

  std::string cmd{argv[1]};
  if (cmd == "--help" && argc != 2) {
    throw std::invalid_argument("--help doesn't take arguments");
  }

  if (cmd == "embed") {
    string_set required{"-sf", "-cf", "-m"};
    string_set allowed{"-mf", "-k"};
    parse_opts(args, argc, argv, required, allowed);

  } else if (cmd == "extract") {
    string_set required{"-sf", "-m"};
    string_set allowed{"-mf", "-k"};
    parse_opts(args, argc, argv, required, allowed);
  } else if (cmd == "info") {
    if (argc < 3) {
      throw std::invalid_argument(
          "Expected at least one argument for info, see --help");
    }

    if (argc > 3) {
      string_set allowed{"-k"};
      string_set required;
      parse_opts(args, argc - 1, argv + 1, required, allowed);
    }
    args.coverfile = argv[2];
  } else {
    throw std::invalid_argument("Unrecognized command: \"" + cmd +
                                "\", see --help");
  }
  args.command = cmd;
  return args;
}

const std::vector<std::string> str_split(const std::string& str, char delim)
{
  std::vector<std::string> res;
  size_t start = 0;
  size_t found = str.find(delim);
  while (found != std::string::npos) {
    res.emplace_back(str.begin() + start, str.begin() + found);
    start = found + 1;
    found = str.find(delim, start);
  }

  if (start != str.size())
    res.emplace_back(str.begin() + start, str.end());
  return res;
}

string_map parse_key(const std::string& key)
{
  std::unordered_map<std::string, std::string> params;
  const std::vector<std::string> key_values = str_split(key, ',');

  for (const std::string& key_value : key_values) {
    const std::vector<std::string> kv = str_split(key_value, '=');
    params[kv.at(0)] = kv.at(1);
  }
  return params;
}

static void parse_opts(struct args& args,
                       int argc,
                       char* argv[],
                       string_set& required,
                       const string_set& optional)
{
  for (int i = 2; i < argc; i++) {
    std::string arg = std::string(argv[i]);

    string_set::iterator iter;
    string_set::iterator end;
    if ((iter = std::find(required.begin(), required.end(), arg)) !=
        required.end()) {
      required.erase(iter);
    } else if (std::find(optional.begin(), optional.end(), arg) ==
               optional.end()) {
      if (!optional.empty()) {
        std::string msg{"option not allowed for this command: " + arg};
        throw std::invalid_argument(msg);
      }
    }

    if (arg == "-k" || arg == "--key") {
      REQUIRE_OPT_ARG(arg);
      args.key = std::string(argv[i]);
    } else if (arg == "-m" || arg == "--method") {
      REQUIRE_OPT_ARG(arg);
      args.method = std::string(argv[i]);
    } else if (arg == "-cf") {
      REQUIRE_OPT_ARG(arg);
      args.coverfile = std::string(argv[i]);
    } else if (arg == "-sf") {
      REQUIRE_OPT_ARG(arg);
      args.stegofile = std::string(argv[i]);
    } else if (arg == "-mf") {
      REQUIRE_OPT_ARG(arg);
      args.msgfile = std::string(argv[i]);
    } else {
      throw std::invalid_argument("unknown option: " + arg);
    }
  }

  if (!required.empty()) {
    std::string arg = *required.begin();
    throw std::invalid_argument("Required argument not given: " + arg);
  }
}
