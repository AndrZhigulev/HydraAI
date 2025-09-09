#include <iostream>

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
#ifdef  _WIN32
    std::cout << "Running on Windows!" << std::endl;
#elif __linux__
    std::cout << "Running on Linux!" << std::endl;
    #elif __APPLE__
    std::cout << "Running on macOS!" << std::endl;
    #else
    std::cout << "Running on an unknown OS!!" << std::endl;
#endif
    return 0;
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}