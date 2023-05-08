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
