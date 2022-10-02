#include "./LoadRes.h"
#include <Windows.h>
// #define DEBUG_LOAD_RES

#ifdef DEBUG_LOAD_RES
#include <iostream>
#endif

bool load_resource(int resource, std::string &string) {
  bool ret = false;
  HRSRC src = FindResource(NULL, MAKEINTRESOURCE(resource), RT_RCDATA);

  if (src != NULL) {
    HGLOBAL header = LoadResource(NULL, src);
    if (header != NULL) {
      char *data = (char *)LockResource(header);

      if (data != NULL) {
        size_t size = SizeofResource(0, src);
        string.resize(size);
        memcpy(&string[0], data, size);
        ret = true;
      }
#ifdef DEBUG_LOAD_RES
      else
        std::cerr << "Failure locking resource. Last Error: " << GetLastError()
                  << std::endl;
#endif

      UnlockResource(header);
    }
#ifdef DEBUG_LOAD_RES
    else
      std::cerr << "Failure finding resource. Last Error: " << GetLastError()
                << std::endl;
#endif

    FreeResource(header);
  }
#ifdef DEBUG_LOAD_RES
  else
    std::cerr << "Failure loading resource. Last Error: " << GetLastError()
              << std::endl;
#endif

  return ret;
}