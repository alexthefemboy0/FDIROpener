#include "func.cpp"

int main() {
    int code = Init();

    if (code == 0) {
        std::cout << "The program exited with code 0. (Success)" << std::endl;
    } else if (code == 0) {
        std::cout << "The program exited with code 1. (Failure)" << std::endl;
    }

    return 0;
}