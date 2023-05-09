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
#include "method_factory.h"
#include "echo_hiding.h"
#include "echo_hiding_hc.h"
#include "lsb_substitution.h"
#include "phase_coding.h"
#include "tone_insertion.h"

template <typename T>
std::unique_ptr<Method> create_unique(const Params& params)
{
  return std::make_unique<T>(params);
}

MethodFactory::creator_map MethodFactory::method_map = {
    {"lsb", create_unique<LSBMethod>},
    {"phase", create_unique<PhaseMethod>},
    {"echo", create_unique<EchoHidingMethod>},
    {"tone", create_unique<ToneInsertionMethod>},
    {"echo-hc", create_unique<EchoHidingHCMethod>}};

std::unique_ptr<Method> MethodFactory::create(const std::string& method_name,
                                              const Params& params)
{
  if (method_map.find(method_name) == method_map.end()) {
    throw std::invalid_argument("Unknown method: " + method_name);
  }
  return method_map[method_name](params);
}

std::vector<std::string> MethodFactory::list_methods()
{
  std::vector<std::string> keys;
  for (auto it = method_map.begin(); it != method_map.end(); ++it) {
    keys.push_back(it->first);
  }
  return keys;
}
