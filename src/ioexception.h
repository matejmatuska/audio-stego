#ifndef IOEXCEPTION_H
#define IOEXCEPTION_H

#include <stdexcept>

class IOException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

#endif  // IOEXCEPTION_H
