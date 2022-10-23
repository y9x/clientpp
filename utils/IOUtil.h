#include <Windows.h>
#include <filesystem>
#include <string>

namespace IOUtil {
// returns true if a handle to the file can be created, false if failure
bool file_exists(const std::filesystem::path &path);

// reads a file into buffer, returns true if successful, false if failure
bool read_file(const std::filesystem::path &path, std::string &buffer);

// writes a file from the buffer, for small data
bool write_file(const std::filesystem::path &path, const std::string &buffer);

}; // namespace IOUtil