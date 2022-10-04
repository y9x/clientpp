#include "./StringUtil.h"
#include <stdexcept>
#include <string>
#include <vector>

template <typename chr>
using stringof = std::basic_string<chr, std::char_traits<chr>, std::allocator<chr>>;

template <typename In, typename Out>
stringof<Out> convert_string(const stringof<In> &input)
{
  stringof<Out> output;

  for (const In &c : input)
  {
    output += static_cast<Out>(c);
  }

  return output;
}

template <typename T>
stringof<T> lowercase_string(const stringof<T> &string)
{
  stringof<T> output;
  for (const T &c : string)
    output += tolower(c);
  return output;
}

template <typename T>
stringof<T> uppercase_string(const stringof<T> &string)
{
  stringof<T> output;
  for (const T &c : string)
    output += toupper(c);
  return output;
}

namespace ST
{
  std::wstring wstring(const std::string &str)
  {
    return convert_string<char, wchar_t>(str);
  }

  std::string string(const std::wstring &str)
  {
    return convert_string<wchar_t, char>(str);
  }

  template <typename T>
  stringof<T> replace_all_string(stringof<T> string, const stringof<T> &from,
                                 const stringof<T> &to)
  {
    size_t start_pos = 0;
    while ((start_pos = string.find(from, start_pos)) != stringof<T>::npos)
    {
      string.replace(start_pos, from.length(), to);
      start_pos +=
          to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return string;
  }

  std::string replace_all(std::string string, const std::string &from,
                          const std::string &to)
  {
    return replace_all_string<char>(string, from, to);
  }

  std::wstring replace_all(std::wstring string, const std::wstring &from,
                           const std::wstring &to)
  {
    return replace_all_string<wchar_t>(string, from, to);
  }

  std::string lowercase(const std::string &string)
  {
    return lowercase_string<char>(string);
  }

  std::wstring lowercase(const std::wstring string)
  {
    return lowercase_string<wchar_t>(string);
  }

  std::string uppercase(const std::string &string)
  {
    return uppercase_string<char>(string);
  }

  std::wstring uppercase(const std::wstring string)
  {
    return uppercase_string<wchar_t>(string);
  }
}; // namespace ST