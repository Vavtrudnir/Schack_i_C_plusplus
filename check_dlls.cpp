#include <iostream>
#include <windows.h>

int main() {
    std::cout << "Checking DLL dependencies..." << std::endl;
    
    // List of required DLLs
    const char* requiredDLLs[] = {
        "libsfml-graphics-3.dll",
        "libsfml-window-3.dll", 
        "libsfml-system-3.dll",
        "libstdc++-6.dll",
        "libgcc_s_seh-1.dll",
        "libwinpthread-1.dll",
        "libfreetype-6.dll",
        "libpng16-16.dll",
        "zlib1.dll"
    };
    
    int missingCount = 0;
    
    for (const char* dll : requiredDLLs) {
        HMODULE hModule = LoadLibraryA(dll);
        if (hModule == NULL) {
            std::cout << "MISSING: " << dll << std::endl;
            missingCount++;
        } else {
            std::cout << "FOUND: " << dll << std::endl;
            FreeLibrary(hModule);
        }
    }
    
    if (missingCount == 0) {
        std::cout << "\nAll required DLLs are available!" << std::endl;
    } else {
        std::cout << "\n" << missingCount << " DLLs are missing!" << std::endl;
    }
    
    return missingCount;
}
