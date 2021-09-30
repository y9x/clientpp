#include <string>
#include <stdexcept>
#include <vector>
#include "./StringUtil.h"

namespace StringUtil {
	namespace Convert {
		std::wstring wstring(std::string str) {
			return std::wstring(str.begin(), str.end());
		}

		std::string string(std::wstring str) {
			return std::string(str.begin(), str.end());
		}
	};

	namespace Manipulate {
		std::string pad_start(std::string string, int pad, std::string fill) {
			std::string output;
			size_t missing = pad - string.length();

			if (missing > 0) {
				while (missing--) {
					output += fill;
				}
			}

			output += string;

			return output;
		}

		std::string slice(std::string string, int start, int end) {
			// std::cout << "start: " << start << ", len: " << string.length() << ", math: " << string.length() + start << std::endl;

			int sub_end = start - end,
				str_len = (int)string.length();

			if (start < 0) {
				start += str_len;
				sub_end = end - start;
			}

			if (end < 0) {
				sub_end = (str_len - start) + end;
			}

			if (sub_end > str_len)sub_end = str_len;

			try {
				return string.substr(start, sub_end);
			}
			catch (const std::out_of_range& err) {
				return "Failure substring(" + std::to_string(start) + ", " + std::to_string(sub_end) + "): " + err.what();
			}
		}

		std::string slice(std::string string, int start) {
			return slice(string, start, (int)string.length());
		}

		std::string replace_all(std::string string, const std::string& from, const std::string& to) {
			size_t start_pos = 0;
			while ((start_pos = string.find(from, start_pos)) != std::string::npos) {
				string.replace(start_pos, from.length(), to);
				start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
			}
			return string;
		}

		std::wstring replace_all(std::wstring string, const std::wstring& from, const std::wstring& to) {
			size_t start_pos = 0;
			while ((start_pos = string.find(from, start_pos)) != std::wstring::npos) {
				string.replace(start_pos, from.length(), to);
				start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
			}
			return string;
		}

		std::vector<std::string> split(const std::string& str, char delim) {
			std::vector<std::string> strings;
			size_t start;
			size_t end = 0;
			while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
				end = str.find(delim, start);
				strings.push_back(str.substr(start, end - start));
			}
			return strings;
		}
	};

	namespace Validate {
		char nums[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

		bool Number(std::string str) {
			for (char ch : str) {
				bool was_num = false;

				for (int ni = 0; ni < 10; ni++) {
					if (nums[ni] == ch) {
						was_num = true;
						break;
					}
				}

				if (!was_num)return false;
			}

			return true;
		}
	};
};