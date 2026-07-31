#include <cstdio>
#include <memory>
#include <Patterns.hpp>

namespace grammJSON {
    IMP_DECL_PATTERN(static spacing);
    IMP_DECL_PATTERN(static object);
    IMP_DECL_PATTERN(static array);
    IMP_DECL_PATTERN(static field);
    IMP_DECL_PATTERN(static value);
    IMP_DECL_PATTERN(static boolean);
    IMP_DECL_PATTERN(static null);
    IMP_DECL_PATTERN(static string);
    IMP_DECL_PATTERN(static strfill);
    IMP_DECL_PATTERN(static escseq);
    IMP_DECL_PATTERN(static number);
    IMP_DECL_PATTERN(static numint);
    IMP_DECL_PATTERN(static numfract);

    IMP_MAKE_PATTERN(spacing,
        imp::AtLeast<0>(imp::SpaceOrNewLine())
    )

    IMP_MAKE_PATTERN(object,
        imp::Str<"{">() >> imp::Fn<spacing>() >>
        imp::UpTo<1>(
            imp::Fn<field>() >> imp::Fn<spacing>() >>
            imp::AtLeast<0>(
                imp::Str<",">() >> imp::Fn<spacing>() >>
                imp::Fn<field>() >> imp::Fn<spacing>()
            )
        ) >> imp::Str<"}">()
    )

    IMP_MAKE_PATTERN(array,
        imp::Str<"[">() >> imp::Fn<spacing>() >>
        imp::UpTo<1>(
            imp::Fn<value>() >> imp::Fn<spacing>() >>
            imp::AtLeast<0>(
                imp::Str<",">() >> imp::Fn<spacing>() >>
                imp::Fn<value>() >> imp::Fn<spacing>()
            )
        ) >> imp::Str<"]">()
    )

    IMP_MAKE_PATTERN(field,
        imp::Fn<string>() >> imp::Fn<spacing>() >>
        imp::Str<":">() >> imp::Fn<spacing>() >>
        imp::Fn<value>()
    )

    IMP_MAKE_PATTERN(value,
        imp::Fn<object>()  | imp::Fn<array>()  |
        imp::Fn<string>()  | imp::Fn<number>() |
        imp::Fn<boolean>() | imp::Fn<null>()
    )

    IMP_MAKE_PATTERN(boolean,
        imp::Str<"true">() | imp::Str<"false">()
    )

    IMP_MAKE_PATTERN(null,
        imp::Str<"null">()
    )

    IMP_MAKE_PATTERN(string,
        imp::Str<"\"">() >> imp::Fn<strfill>() >>
        imp::AtLeast<0>(
            imp::Fn<escseq>() >> imp::Fn<strfill>()
        ) >> imp::Str<"\"">()
    )

    IMP_MAKE_PATTERN(strfill,
        imp::AtLeast<0>(imp::NegSet<"\"\\">())
    )

    IMP_MAKE_PATTERN(escseq,
        imp::Str<"\\">() >> (
            imp::Set<"/\"\\bfnrt">() |
            imp::Set<"uU">() >> imp::Exactly<4>(imp::HexDigit())
        )
    )

    IMP_MAKE_PATTERN(number,
        imp::Fn<numint>() >> imp::UpTo<1>(imp::Fn<numfract>())
    )

    IMP_MAKE_PATTERN(numint,
        imp::UpTo<1>(imp::Set<"+-">()) >> imp::AtLeast<1>(imp::Digit())
    )

    IMP_MAKE_PATTERN(numfract,
        imp::Str<".">() >> imp::AtLeast<1>(imp::Digit()) >>
        imp::UpTo<1>(
            imp::Set<"eE">() >> imp::Fn<numint>()
        )
    )

    IMP_MAKE_PATTERN(eval,
        (imp::Fn<spacing>() >> imp::UpTo<1>(
            imp::Fn<value>() >> imp::Fn<spacing>()
        ) >> imp::None()) /
        [] (FILE* hFile, const imp::Match& m, imp::CapturesView, const std::any&) -> imp::Match {
            if (!m)
                fprintf(stderr, "failed to parse JSON at %zi\n", ftell(hFile));
            else
                printf("success\n");
            return m;
        }
    )
}

using CFile = std::unique_ptr<FILE,
    decltype([] (FILE* hFile) { if (hFile) fclose(hFile); })>;

int main() {
    static constexpr std::string_view
        strvFile    = "./assets/test.json";

    CFile
        uptrFile    = CFile(fopen(strvFile.data(), "rb"));

    if (!uptrFile) {
        fprintf(stderr, "failed to open file: %s\n",
            strvFile.data());
        return EXIT_FAILURE;
    }

    return (bool)imp::Eval(imp::Fn<grammJSON::eval>(), uptrFile.get())
        ? EXIT_SUCCESS : EXIT_FAILURE;
}