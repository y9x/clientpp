/**
 * Quick util to work with files.
 */
#pragma once
#include <filesystem>

namespace IOUtil {
/// @brief Check if a file exists
/// @param path File to access
/// @return If the file handle was opened
bool fileExists(const std::filesystem::path &path);
/// @brief Read a file
/// @param path File to read from
/// @param buffer A pointer to store the contents of the file
/// @return If the file handle was opened
bool readFile(const std::filesystem::path &path, std::string &buffer);
/// @brief Write a file
/// @param path The file to write to
/// @param buffer The new contents the file
/// @return If the file handle was opened
bool writeFile(const std::filesystem::path &path, const std::string &buffer);
}; // namespace IOUtil