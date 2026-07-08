#include <cstdlib>
#include <cstdio>
#include <memory>

using CFile = std::unique_ptr<FILE,
    decltype([] (FILE* hFile) { if (hFile) fclose(hFile); })>;

constexpr const char*
    szFilename  = "./assets/test.json";

int main() {
    CFile
        uptrJSON    = CFile{fopen64("./assets/test.json", "w")};
    if (!uptrJSON) {
        fprintf(stderr, "failed to open %s\n", szFilename);
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

    srand(time(nullptr));
    fprintf(uptrJSON.get(), "{\n\t\"scores\": [\n");
    for (size_t i = 0; i != 5'000'000; ++i) {
        fprintf(uptrJSON.get(), "\t\t{ \"%s %s\": %d },\n",
            lpPrefixes[rand() % 8], lpPostfixes[rand() % 8], rand() % 100 + 1);
    }
    fprintf(uptrJSON.get(), "\t\t{ \"%s %s\": %d }\n\t]\n}",
        lpPrefixes[rand() % 8], lpPostfixes[rand() % 8], rand() % 100 + 1);

    return EXIT_SUCCESS;
}