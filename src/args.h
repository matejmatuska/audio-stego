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
/**
 * @file Utilities for parsing program arguments.
 */
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Holds program arguments.
 */
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

/**
 * @brief Parse program arguments.
 * @param argc The argc passed to main.
 * @param argv The argv passed to main.
 * @return The parsed arguments.
 */
struct args parse_args(int argc, char* argv[]);

using string_map = std::unordered_map<std::string, std::string>;

/**
 * @brief Parse the method parameters (stego key).
 * @param key The string with the parameters (stego key).
 * @return A map mapping the params to their values.
 */
string_map parse_key(const std::string& key);
