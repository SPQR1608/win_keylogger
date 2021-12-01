#ifndef _HELPER_H
#define _HELPER_H

#include <ctime>
#include <string>
#include <sstream>
#include <fstream>

namespace Helper
{
    template<class T>
    inline std::string ToString(const T& e)
    {
        std::ostringstream s;
        s << e;
        return s.str();
    }

    inline std::tm localtime_save(std::time_t timer)
    {
        std::tm bt{};
#if defined(__unix__)
        localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
        localtime_s(&bt, &timer);
#else
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        bt = *std::localtime(&timer);
#endif
        return bt;
    }

    class DateTime
    {
    public:
        DateTime()
        {
            std::time_t ms;
            time(&ms);

            std::tm info = localtime_save(ms);
            d = info.tm_mday;
            m = info.tm_mon + 1;
            y = 1900 + info.tm_year;
            H = info.tm_hour;
            M = info.tm_min;
            S = info.tm_sec;
        }

        DateTime(int d, int m, int y, int H, int M, int S)
            : d(d), m(m), y(y), H(H), M(M), S(S)
        {}

        DateTime(int d, int m, int y)
            : d(d), m(m), y(y), H(0), M(0), S(0)
        {}

        DateTime Now() const { return DateTime(); }

        std::string GetDateString() const 
        { 
            return  std::string(d < 10 ? "0" : "") + ToString(d) +
                    std::string(m < 10 ? ".0" : ".") + ToString(m) +
                    "." + ToString(y);
        }

        std::string GetTimeString(const std::string &sep = ":") const
        {
            return std::string(H < 10 ? "0" : "") + ToString(H) + sep +
                   std::string(M < 10 ? "0" : "") + ToString(M) + sep +
                   std::string(S < 10 ? sep : "") + ToString(S);
        }

        std::string GetDateTimeString(const std::string& sep = ":") const 
        { 
            return GetDateString() + " " + GetTimeString(sep); 
        }

    private:
        int d, m, y, H, M, S;
    };

    inline void WriteAppLog(const std::string& ms)
    {
        std::ofstream file("App.log", std::ios::app);
        file << "[" << Helper::DateTime().GetDateTimeString() << "]"
             << "\t" << ms << std::endl;
        file.close();
    }
}

#endif // !_HELPER_H