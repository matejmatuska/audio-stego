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

/**
 * @brief Wrapper around std::make_unique.
 * Can be used to create a function pointer to std::make_unique for Method objects.
 * @param params The parameters for the method.
 * @return A pointer to the created Method.
 */
template <typename T>
std::unique_ptr<Method> create_unique(const Params& params)
{
  return std::make_unique<T>(params);
}

/**
 * @brief Proxy for objects registered to the factory.
 */
struct MethodFactory::Register {
  /**
   * @brief Pointer to creation used to create this Method.
   * @see create_unique
   */
  MethodFactory::method_creator creator;
  /**
   * @brief A list of parameters accepted by the Method.
   */
  std::vector<Param> params;
};

MethodFactory::creator_map MethodFactory::method_map = {
    {"lsb",
     Register{create_unique<LSBMethod>,
              {Param("lsbs", "number of lsbs in samples to substitute")}}},
    {"phase", Register{create_unique<PhaseMethod>,
                       {Param("framesize", "the length of one audio frame")}}},
    {"echo", Register{create_unique<EchoHidingMethod>,
                      {
                          Param("delay0", "echo delay for bit 0"),
                          Param("delay1", "echo delay for bit 1"),
                          Param("amp", "echo amplitude (0-1)"),
                          Param("framesize", "the length of one audio frame"),
                      }}},
    {"echo-hc",
     Register{create_unique<EchoHidingHCMethod>,
              {
                  Param("interval", "space between echo positions"),
                  Param("amp", "echo amplitude (0-1)"),
                  Param("framesize", "the length of one audio frame"),
              }}},
    {"tone", Register{create_unique<ToneInsertionMethod>,
                      {
                          Param("freq0", "the frequency for bit 0"),
                          Param("freq1", "the frequency for bit 1"),
                          Param("framesize", "the length of one audio frame"),
                      }}}};

std::unique_ptr<Method> MethodFactory::create(const std::string& method_name,
                                              const Params& params)
{
  if (method_map.find(method_name) == method_map.end()) {
    throw std::invalid_argument("Unknown method: " + method_name);
  }
  return method_map[method_name].creator(params);
}

std::vector<std::string> MethodFactory::list_methods()
{
  std::vector<std::string> keys;
  for (auto it = method_map.begin(); it != method_map.end(); ++it) {
    keys.push_back(it->first);
  }
  return keys;
}

const std::vector<MethodFactory::Param>& MethodFactory::get_method_params(
    const std::string& method)
{
  return method_map[method].params;
}
