#define _CRT_SECURE_NO_WARNINGS
#include "./IOUtil.h"
#include "./StringUtil.h"
#include <windows.h>

namespace IOUtil {
	bool DirectoryIterator::test_filename() {
		std::string f = file();
		return f == "." || f == "..";
	}
	DirectoryIterator::DirectoryIterator(std::string f, std::string filter) : folder(f) {
		std::string wsfind = folder;
		if (wsfind.back() != '\\')wsfind += '\\';
		wsfind += filter;
		folder_ws = StringUtil::Convert::wstring(wsfind);
	}

	DirectoryIterator::~DirectoryIterator() {
		stop();
	}

	void DirectoryIterator::stop() {
		if (stopped)return;

		stopped = true;
		FindClose(find);
	}
	
	std::string DirectoryIterator::file() {
		if (frame_filen)return filen;
		else return filen = StringUtil::Convert::string(find_data.cFileName);
	}
	std::string DirectoryIterator::path() {
		return folder + "\\" + file();
	}

	bool DirectoryIterator::operator++ () {
		frame_filen = false;
		cnext = false;

		if (first) {
			first = false;
			if ((find = FindFirstFile(folder_ws.c_str(), &find_data)) == INVALID_HANDLE_VALUE) return stopped = true, false;
		}
		else if (!FindNextFile(find, &find_data))return false;

		if (test_filename())return operator++();

		return cnext = true;
	}

	bool WDirectoryIterator::test_filename() {
		std::wstring f = file();
		return f == L"." || f == L"..";
	}

	WDirectoryIterator::WDirectoryIterator(std::wstring f, std::wstring filter) : folder(f), filter(filter) {
		if (folder.back() == '\\')folder.pop_back();
	}

	WDirectoryIterator::~WDirectoryIterator() {
		stop();
	}

	void WDirectoryIterator::stop() {
		if (stopped)return;

		stopped = true;
		FindClose(find);
	}

	std::wstring WDirectoryIterator::file() {
		if (frame_filen)return filen;
		else return filen = find_data.cFileName;
	}
	std::wstring WDirectoryIterator::path() {
		return folder + L"\\" + file();
	}

	bool WDirectoryIterator::operator++ () {
		frame_filen = false;
		cnext = false;

		if (first) {
			first = false;
			if ((find = FindFirstFile((folder + L"\\" + filter).c_str(), &find_data)) == INVALID_HANDLE_VALUE) return stopped = true, false;
		}
		else if (!FindNextFile(find, &find_data))return false;

		if (test_filename())return operator++();

		return cnext = true;
	}

	bool read_file(std::string path, std::string& buffer) {
		FILE* file = fopen(path.c_str(), "r");

		if (!file)return false;

		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		rewind(file);

		buffer.resize(size);

		// size_t bytes_read = 
		// buffer.resize(
		buffer.resize(fread(&buffer[0], 1, size, file));

		fclose(file);

		return true;
	}

	bool wread_file(std::wstring path, std::string& buffer) {
		FILE* file = _wfopen(path.c_str(), L"r");

		if (!file)return false;
		
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		rewind(file);

		buffer.resize(size);

		buffer.resize(fread(&buffer[0], 1, size, file));

		fclose(file);

		return true;
	}

	// for small data
	bool write_file(std::string path, std::string buffer) {
		FILE* file = fopen(path.c_str(), "w");

		if (!file)return false;

		fwrite(buffer.data(), sizeof(char), buffer.length(), file);

		fclose(file);

		return true;
	}
	
	bool wwrite_file(std::wstring path, std::string buffer) {
		FILE* file = _wfopen(path.c_str(), L"w");

		if (!file)return false;

		fwrite(buffer.data(), sizeof(char), buffer.length(), file);

		fclose(file);

		return true;
	}
};