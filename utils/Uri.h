#include <uriparser/Uri.h>
#include <string>

class UriA
{
private:
    std::string uri;
    UriUriA uriParse;
    bool isValid_;

public:
    UriA(const std::string &uri);
    ~UriA();
    bool isValid() const;
    std::string scheme() const;
    std::string host() const;
    std::string port() const;
    std::string path() const;
    std::string query() const;
    std::string fragment() const;
    std::string toString() const;
    operator std::string() const;
};

class UriW
{
private:
    std::wstring uri;
    UriUriW uriParse;
    bool isValid_;

public:
    UriW(const std::wstring &uri);
    ~UriW();
    bool isValid() const;
    std::wstring scheme() const;
    std::wstring host() const;
    std::wstring port() const;
    std::wstring path() const;
    std::wstring query() const;
    std::wstring fragment() const;
    std::wstring toString() const;
    operator std::wstring() const;
};