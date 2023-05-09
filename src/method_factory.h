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
#ifndef METHOD_FACTORY_H
#define METHOD_FACTORY_H

#include "methods.h"

/**
 * @brief A factory for creating Method objects.
 */
class MethodFactory {
 public:
  MethodFactory() = delete;

  /**
   * @brief Retrieves the list of all registered methods.
   * @return The list of all registered methods.
   */
  static std::vector<std::string> list_methods();

  /**
   * @brief Create a method object.
   * @params method_name The name of the method to create.
   * @params params The parameters (stego key) for the method.
   * @return The created method object.
   */
  static std::unique_ptr<Method> create(const std::string& method_name,
                                        const Params& params);

 private:
  using method_creator = std::unique_ptr<Method> (*)(const Params& params);
  using creator_map = std::map<std::string, method_creator>;

  static creator_map method_map;
};

#endif  // METHOD_FACTORY_H
