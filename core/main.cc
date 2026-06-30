#include <log/logger.h>

int main(int argc, char** argv) {
    Logger::GetInstance()->Init("log/1.log", Logger::console, Logger::info, 10, 3);
    LOG(info) << "start on demand demo";
    return 0;
}