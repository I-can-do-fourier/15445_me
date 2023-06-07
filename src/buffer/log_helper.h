//
// Created by 昕航韩 on 2023/6/7.
//

#include<string>

#ifndef BUSTUB_LOG_HELPER_H
#define BUSTUB_LOG_HELPER_H

#endif  // BUSTUB_LOG_HELPER_H

#define LOG_ENABLED  // Enable or disable logging globally

#ifdef LOG_ENABLED
#define LOG(format, ...) \
        std::cout << "[" << __FUNCTION__ << "] " \
                  << "Params: " << GetParameterString(__VA_ARGS__) << " -- " \
                  << "Message: " << StringFormat(format, __VA_ARGS__) \
                  << std::endl;
#else
#define LOG(format, ...)
#endif



template<typename... Args>
std::string StringFormat(const std::string& format, Args... args);

template<typename T>
std::string GetParameterString(const T& arg);

template<typename T, typename... Args>
std::string GetParameterString(const T& arg, Args... args);