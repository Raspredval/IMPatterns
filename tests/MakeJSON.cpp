#include <cstdlib>
#include <cstdio>
#include <memory>

#include <unistd.h>

using CFile = std::unique_ptr<FILE,
    decltype([] (FILE* hFile) { if (hFile) fclose(hFile); })>;

constexpr const char*
    szFilename  = "./assets/test.json";

int main() {
    if (access(szFilename, F_OK) == 0) {
        printf("%s already exists\n", szFilename);
        return EXIT_SUCCESS;
    }

    CFile
        uptrJSON    = CFile{fopen64("./assets/test.json", "wb")};
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
            "Lil"
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

    fprintf(uptrJSON.get(), "{\n\t\"scores\": [\n");
    for (size_t i = 0; i != 1'000'000'000; ++i) {
        int iRnd    = rand();
        fprintf(uptrJSON.get(), "\t\t{ \"%s %s\": %f },\n",
            lpPrefixes[iRnd % 8], lpPostfixes[iRnd % 8],
            (double)iRnd / (double)RAND_MAX);
    }
    fprintf(uptrJSON.get(), "\t]\n}");

    return EXIT_SUCCESS;
}