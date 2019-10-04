#ifndef __RESULT_H__
#define __RESULT_H__

#include <string>

#define RESULT_CODES_TYPE unsigned int

#define MAIN_RESULT_CODES \
    OK = 0, \
    ERROR = 1 \

namespace twPro {

    template <class RC>
    class Result final
    {
    public:

        Result() noexcept : m_resultCode(RC::OK) {}
        Result(const bool _bool) noexcept : m_resultCode(_bool ? RC::OK : RC::ERROR) {}
        Result(const RC & _code) noexcept : m_resultCode(_code) {}
        Result(const RC & _code, const std::string & _text) noexcept : m_resultCode(_code), m_text(_text) {}

        ~Result() noexcept {}

        inline bool toBool() const { return m_resultCode == RC::OK; }

        inline operator bool() const { return toBool(); }
        inline bool operator!() const { bool b = toBool(); return !b; }

        inline Result& operator=(const RC & _code) { m_resultCode = _code; return *this; }
        inline Result& operator=(const bool _bool) { m_resultCode = _bool ? RC::OK : RC::ERROR; return *this; }
        inline Result& operator=(const std::pair<RC, std::string> & _pair) { m_resultCode = _pair.first; m_text = _pair.second; return *this; }

        inline RC code() const { return m_resultCode; }
        inline std::string text() const { return m_text; }

    private:

        RC m_resultCode;
        std::string m_text;

    };

    template<class RC, class V>
    struct RetVal
    {
        Result<RC> result;
        V value;

        RetVal() {}
        RetVal(const Result<RC> & _ret, const V & _value) : result(_ret), value(_value) {}
    };

}

#endif // __RESULT_H__