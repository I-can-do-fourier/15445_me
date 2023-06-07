//
// Created by 昕航韩 on 2023/6/7.
//

#include <iostream>
#include <sstream>

#include "log_helper.h"

// Helper function to format the log message
template<typename... Args>
std::string StringFormat(const std::string& format, Args... args)
{
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::unique_ptr<char[]> buffer(new char[size]);
  snprintf(buffer.get(), size, format.c_str(), args...);
  return std::string(buffer.get(), buffer.get() + size - 1);
}

// Helper function to get the parameter string
template<typename T>
std::string GetParameterString(const T& arg)
{
  std::ostringstream oss;
  oss << arg;
  return oss.str();
}

template<typename T, typename... Args>
std::string GetParameterString(const T& arg, Args... args)
{
  std::ostringstream oss;
  oss << arg << ", " << GetParameterString(args...);
  return oss.str();
}
