#include <Windows.h>
#include <string>

namespace IOUtil {
// Iterates over files in the specified directory
class DirectoryIterator {
private:
  HANDLE find;
  WIN32_FIND_DATA find_data;
  std::string folder;
  std::wstring folder_ws;
  bool bad = true;
  bool first = true;
  bool stopped = false;
  bool cnext = false;
  bool frame_filen = false;
  std::string filen;
  bool test_filename();

public:
  DirectoryIterator(std::string folder, std::string filter = "*.*");
  ~DirectoryIterator();
  void stop();
  std::string file();
  std::string path();
  // Starts iterating or goes to the next file
  bool operator++();
};

class WDirectoryIterator {
private:
  HANDLE find;
  WIN32_FIND_DATA find_data;
  std::wstring folder;
  std::wstring filter;
  bool bad = true;
  bool first = true;
  bool stopped = false;
  bool cnext = false;
  bool frame_filen = false;
  std::wstring filen;
  bool test_filename();

public:
  WDirectoryIterator(std::wstring folder, std::wstring filter = L"*.*");
  ~WDirectoryIterator();
  void stop();
  std::wstring file();
  std::wstring path();
  // Starts iterating or goes to the next file
  bool operator++();
};

// returns true if a handle to the file can be created, false if failure
bool file_exists(std::wstring path);
bool file_exists(std::string path);

// reads a file into buffer, returns true if successful, false if failure
bool read_file(std::wstring path, std::string &buffer);
bool read_file(std::string path, std::string &buffer);

// writes a file from the buffer, for small data
bool write_file(std::string path, std::string buffer);
bool write_file(std::wstring path, std::string buffer);

}; // namespace IOUtil