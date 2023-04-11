#ifndef METHODS_H
#define METHODS_H

#include <cstddef>
#include <iostream>
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

#define ACCESSOR_DEF(type, conv_func)                                 \
  type get_or(const std::string& name, const type& def) const         \
  {                                                                   \
    std::unordered_map<std::string, std::string>::const_iterator it = \
        map.find(name);                                               \
    if (it == map.end())                                              \
      return def;                                                     \
    return conv_func(it->second);                                     \
  }

  ACCESSOR_DEF(int, std::stoi);
  ACCESSOR_DEF(unsigned, std::stoul);
  ACCESSOR_DEF(unsigned long long, std::stoull);

#define ACCESSOR(type, fname, conv_func)                         \
  type get_##fname(const std::string& name) const                \
  {                                                              \
    try {                                                        \
      return conv_func(map.at(name));                            \
    } catch (const std::out_of_range& e) {                       \
      throw std::invalid_argument("Missing parameter: " + name); \
    }                                                            \
  }

  ACCESSOR(int, i, std::stoi);
  ACCESSOR(unsigned, ul, std::stoul);
  ACCESSOR(unsigned long long, ull, std::stoull);
};

class Method {
 public:
  virtual embedder_variant make_embedder(InputBitStream& input) const = 0;
  virtual extractor_variant make_extractor() const = 0;
  virtual ssize_t capacity(std::size_t samples) const = 0;
};

class LSBMethod : public Method {
 public:
  LSBMethod(const Params& params) { bitmask = params.get_or("bitmask", 0x1); };

  embedder_variant make_embedder(InputBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  unsigned bitmask;
};

class PhaseMethod : public Method {
 public:
  PhaseMethod(const Params& params);
  embedder_variant make_embedder(InputBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
};

class EchoHidingMethod : public Method {
 public:
  EchoHidingMethod(const Params& params);
  embedder_variant make_embedder(InputBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
  unsigned delay0;
  unsigned delay1;
};

class ToneInsertionMethod : public Method {
 public:
  ToneInsertionMethod(const Params& params);
  embedder_variant make_embedder(InputBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
  unsigned freq0;
  unsigned freq1;
  unsigned samplerate;
};

class EchoHidingHCMethod : public Method {
 public:
  EchoHidingHCMethod(const Params& params);
  embedder_variant make_embedder(InputBitStream& input) const override;
  extractor_variant make_extractor() const override;
  virtual ssize_t capacity(std::size_t samples) const override;

 protected:
  std::size_t frame_size;
};

#endif  // METHODS_H
