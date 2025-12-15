#include <string>
#include <cstdio>

class Log {
    public:
        static void i(std::string log, std::string tag = "NONE") {
            std::fprintf(stdout, "[INFO]  [%s] %s\n", tag.c_str(), log.c_str());
        }
        static void e(std::string log, std::string tag = "NONE") {
            std::fprintf(stdout, "[ERROR] [%s] %s\n", tag.c_str(), log.c_str());
        }
        static void d(std::string log, std::string tag = "NONE") {
            std::fprintf(stdout, "[DEBUG] [%s] %s\n", tag.c_str(), log.c_str());
        }
};
