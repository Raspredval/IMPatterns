#include <print>
#include <memory>
#include <cstdlib>

using CFile = std::unique_ptr<FILE,
    decltype([] (FILE* hFile) { if (hFile) fclose(hFile); })>;

constexpr const char*
    szFilename  = "./assets/test.json";

int main() {
    CFile
        uptrJSON    = CFile{fopen("./assets/test.json", "w")};
    if (!uptrJSON) {
        std::print(stderr, "failed to open {}\n", szFilename);
        return EXIT_FAILURE;
    }

    const char*
        lpPrefixes[8] = {
            "Long",
            "Pretty",
            "Cheesy",
            "Glorious",

            "Smelly",
            "Fearless",
            "Lil",
            "L33t"
        };
    const char*
        lpPostfixes[8] = {
            "dog",
            "pig",
            "dude",
            "girl",

            "car",
            "mousey",
            "ninja",
            "vim enjoyer"
        };

    srand((unsigned)time(nullptr));
    std::print(uptrJSON.get(), "{{\n\t\"scores\": [\n");
    for (size_t i = 0; i != 100'000'000; ++i) {
        std::print(uptrJSON.get(), "\t\t{{ \"{} {}\": {} }},\n",
            lpPrefixes[rand() % 8], lpPostfixes[rand() % 8], rand() % 100 + 1);
    }
    std::print(uptrJSON.get(), "\t\t{{ \"{} {}\": {} }}\n\t]\n}}",
        lpPrefixes[rand() % 8], lpPostfixes[rand() % 8], rand() % 100 + 1);

    return EXIT_SUCCESS;
}