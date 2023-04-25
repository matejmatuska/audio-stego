#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct args {
  std::string command;
  std::optional<std::string> method;
  std::string key = "";
  std::optional<std::string> coverfile = std::nullopt;
  std::optional<std::string> stegofile = std::nullopt;
  std::optional<std::string> msgfile = std::nullopt;
  std::optional<unsigned long> limit = std::nullopt;  // in bits
  bool use_err_correction = false;
};

struct args parse_args(int argc, char* argv[]);

using string_map = std::unordered_map<std::string, std::string>;
string_map parse_key(const std::string& key);
