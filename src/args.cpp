/*
 * Copyright (C) 2023 Matej Matuska
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
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
  if (++i >= argc) {                                               \
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
  if (argc < 2) {
    throw std::invalid_argument("No command specified");
  }

  std::string cmd{argv[1]};
  if (cmd == "--help" && argc != 2) {
    throw std::invalid_argument("--help doesn't take arguments");
  }

  struct args args;
  if (cmd == "embed") {
    string_set required{"-sf", "-cf", "-m"};
    string_set optional{"-mf", "-k", "-l", "-e"};
    parse_opts(args, argc, argv, required, optional);

  } else if (cmd == "extract") {
    string_set required{"-sf", "-m"};
    string_set optional{"-mf", "-k", "-l", "-e"};
    parse_opts(args, argc, argv, required, optional);
  } else if (cmd == "info") {
    if (argc < 3) {
      throw std::invalid_argument(
          "Expected at least one argument for info, see --help");
    }

    if (argc > 3) {
      string_set optional{"-k"};
      string_set required;
      parse_opts(args, argc - 1, argv + 1, required, optional);
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

unsigned long parse_limit(const char* limit_str)
{
  if (limit_str[0] == '-')
    throw std::invalid_argument(
        "argument -l expects positive integer argument");

  unsigned long limit;
  std::size_t pos;
  try {
    limit = std::stoul(limit_str, &pos);
  } catch (const std::out_of_range& e) {
    throw std::invalid_argument("length too big");
  } catch (const std::invalid_argument& e) {
    throw std::invalid_argument("argument expects a positive number: -l");
  }

  if (limit_str[pos] == 'b')
    return limit;

  return limit * 8;
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
    } else if (arg == "-l") {
      REQUIRE_OPT_ARG(arg);
      args.limit = parse_limit(argv[i]);
    } else if (arg == "-e") {
      args.use_err_correction = true;
    } else {
      throw std::invalid_argument("unknown option: " + arg);
    }
  }

  if (!required.empty()) {
    std::string arg = *required.begin();
    throw std::invalid_argument("Required argument not given: " + arg);
  }
}
