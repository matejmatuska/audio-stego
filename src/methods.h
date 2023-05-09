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
#ifndef METHODS_H
#define METHODS_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "embedder.h"
#include "extractor.h"

using embedder_variant = std::variant<std::unique_ptr<Embedder<double>>,
                                      std::unique_ptr<Embedder<int>>>;

using extractor_variant = std::variant<std::unique_ptr<Extractor<double>>,
                                       std::unique_ptr<Extractor<int>>>;

class Params {
 private:
  std::unordered_map<std::string, std::string> map;

 public:
  Params() : map(){};
  Params(std::unordered_map<std::string, std::string> map) : map(map){};

  void insert(std::string name, std::string value) { map[name] = value; }

  void dump() const
  {
    for (const auto& [key, value] : map) {
      std::cout << "param: \"" << key;
      std::cout << "\", value: \"" << value << "\"\n";
    }
  }

#define ACCESSOR_OR_DEF(type, conv_func)                              \
  type get_or(const std::string& name, const type& def) const         \
  {                                                                   \
    std::unordered_map<std::string, std::string>::const_iterator it = \
        map.find(name);                                               \
    if (it == map.end())                                              \
      return def;                                                     \
    return conv_func(it->second);                                     \
  }

  ACCESSOR_OR_DEF(int, std::stoi);
  ACCESSOR_OR_DEF(unsigned, std::stoul);
  ACCESSOR_OR_DEF(unsigned long long, std::stoull);
  ACCESSOR_OR_DEF(double, std::stod);

#define ACCESSOR_DEF(type, fname, conv_func)                     \
  type get_##fname(const std::string& name) const                \
  {                                                              \
    try {                                                        \
      return conv_func(map.at(name));                            \
    } catch (const std::out_of_range& e) {                       \
      throw std::invalid_argument("Missing parameter: " + name); \
    }                                                            \
  }

  ACCESSOR_DEF(int, i, std::stoi);
  ACCESSOR_DEF(unsigned, ul, std::stoul);
  ACCESSOR_DEF(unsigned long long, ull, std::stoull);
  ACCESSOR_DEF(double, d, std::stod);
};

// TODO document
class Method {
 public:
  virtual embedder_variant make_embedder(InBitStream& input) const = 0;
  virtual extractor_variant make_extractor() const = 0;
  virtual ssize_t capacity(std::size_t samples) const = 0;
};

#endif  // METHODS_H
