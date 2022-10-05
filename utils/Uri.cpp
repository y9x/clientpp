#include "./Uri.h"

std::string fromRange(const UriTextRangeA &rng) { return {rng.first, rng.afterLast}; }

std::wstring fromRange(const UriTextRangeW &rng) { return {rng.first, rng.afterLast}; }

std::string fromList(UriPathSegmentA *xs, const std::string &delim)
{
    UriPathSegmentStructA *head(xs);
    std::string accum;

    while (head)
    {
        accum += delim + fromRange(head->text);
        head = head->next;
    }

    return accum;
}

std::wstring fromList(UriPathSegmentW *xs, const std::wstring &delim)
{
    UriPathSegmentStructW *head(xs);
    std::wstring accum;

    while (head)
    {
        accum += delim + fromRange(head->text);
        head = head->next;
    }

    return accum;
}

UriA::UriA(const std::string &u)
    : uri(u)
{
    UriParserStateA state_;
    state_.uri = &uriParse;
    isValid_ = uriParseUriA(&state_, uri.c_str()) == URI_SUCCESS;
}
UriA::~UriA() { uriFreeUriMembersA(&uriParse); }
bool UriA::isValid() const { return isValid_; }
std::string UriA::scheme() const { return fromRange(uriParse.scheme); }
std::string UriA::host() const { return fromRange(uriParse.hostText); }
std::string UriA::port() const { return fromRange(uriParse.portText); }
std::string UriA::path() const { return fromList(uriParse.pathHead, "/"); }
std::string UriA::query() const { return fromRange(uriParse.query); }
std::string UriA::fragment() const { return fromRange(uriParse.fragment); }
std::string UriA::toString() const { return uri; }
UriA::operator std::string() const { return uri; }

UriW::UriW(const std::wstring &u)
    : uri(u)
{
    UriParserStateW state_;
    state_.uri = &uriParse;
    isValid_ = uriParseUriW(&state_, uri.c_str()) == URI_SUCCESS;
}
UriW::~UriW() { uriFreeUriMembersW(&uriParse); }
bool UriW::isValid() const { return isValid_; }
std::wstring UriW::scheme() const { return fromRange(uriParse.scheme); }
std::wstring UriW::host() const { return fromRange(uriParse.hostText); }
std::wstring UriW::port() const { return fromRange(uriParse.portText); }
std::wstring UriW::path() const { return fromList(uriParse.pathHead, L"/"); }
std::wstring UriW::query() const { return fromRange(uriParse.query); }
std::wstring UriW::fragment() const { return fromRange(uriParse.fragment); }
std::wstring UriW::toString() const { return uri; }
UriW::operator std::wstring() const { return uri; }
