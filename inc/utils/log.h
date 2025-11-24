#include <SDL3/SDL_log.h>
#include <string>

class Log {
    public:
        static void i(std::string log, std::string tag = "NONE") {
            SDL_Log(("[INFO]  [" + tag + "] " + log).c_str());
        }
        static void e(std::string log, std::string tag = "NONE") {
            SDL_Log(("[ERROR] [" + tag + "] " + log).c_str());
        }
        static void d(std::string log, std::string tag = "NONE") {
            SDL_Log(("[DEBUG] [" + tag + "] " + log).c_str());
        }
};
