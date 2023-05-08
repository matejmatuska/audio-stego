#ifndef METHODS_H
#define METHODS_H

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "embedder.h"
#include "extractor.h"

using embedder_variant = std::variant<std::unique_ptr<Embedder<double>>,
                                      std::unique_ptr<Embedder<short>>>;

using extractor_variant = std::variant<std::unique_ptr<Extractor<double>>,
                                       std::unique_ptr<Extractor<short>>>;

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

class Method {
 public:
  virtual embedder_variant make_embedder(InBitStream& input) const = 0;
  virtual extractor_variant make_extractor() const = 0;
  virtual ssize_t capacity(std::size_t samples) const = 0;
};

#endif  // METHODS_H
