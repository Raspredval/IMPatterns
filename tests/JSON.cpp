#include <cstdio>
#include <memory>
#include <Patterns.hpp>

#include "MappedFile.hpp"

namespace grammJSON {
    IMP_DECL_RULE(static spacing);
    IMP_DECL_RULE(static object);
    IMP_DECL_RULE(static array);
    IMP_DECL_RULE(static field);
    IMP_DECL_RULE(static value);
    IMP_DECL_RULE(static boolean);
    IMP_DECL_RULE(static null);
    IMP_DECL_RULE(static string);
    IMP_DECL_RULE(static escseq);
    IMP_DECL_RULE(static number);

    IMP_MAKE_RULE(spacing,
        imp::Any(imp::SpaceOrNewLine())
    )

    IMP_MAKE_RULE(object,
        imp::Str<"{">() >> imp::Fn<spacing>() >>
        imp::Opt(
            imp::Fn<field>() >> imp::Fn<spacing>() >>
            imp::Any(
                imp::Str<",">() >> imp::Fn<spacing>() >>
                imp::Fn<field>() >> imp::Fn<spacing>()
            )
        ) >> imp::Str<"}">()
    )

    IMP_MAKE_RULE(array,
        imp::Str<"[">() >> imp::Fn<spacing>() >>
        imp::Opt(
            imp::Fn<value>() >> imp::Fn<spacing>() >>
            imp::Any(
                imp::Str<",">() >> imp::Fn<spacing>() >>
                imp::Fn<value>() >> imp::Fn<spacing>()
            )
        ) >> imp::Str<"]">()
    )

    IMP_MAKE_RULE(field,
        imp::Fn<string>() >> imp::Fn<spacing>() >>
        imp::Str<":">() >> imp::Fn<spacing>() >>
        imp::Fn<value>()
    )

    IMP_MAKE_RULE(value,
        imp::Fn<object>()  | imp::Fn<array>()  |
        imp::Fn<string>()  | imp::Fn<number>() |
        imp::Fn<boolean>() | imp::Fn<null>()
    )

    IMP_MAKE_RULE(boolean, (
        imp::Dict<"true", "false">()
    ))

    IMP_MAKE_RULE(null,
        imp::Str<"null">()
    )

    IMP_MAKE_RULE(string,
        imp::Str<"\"">() >>
        imp::Any(imp::NegSet<"\"\\">()) >> imp::Any(
            imp::Fn<escseq>() >> imp::Any(imp::NegSet<"\"\\">())
        ) >> imp::Str<"\"">()
    )

    IMP_MAKE_RULE(escseq,
        imp::Str<"\\">() >> (
            imp::Set<"/\"\\bfnrt">() |
            imp::Str<"u">() >> imp::Exactly<4>(imp::HexDigit()) |
            imp::Str<"U">() >> imp::Exactly<8>(imp::HexDigit())
        )
    )

    IMP_MAKE_RULE(number,
        imp::Opt(imp::Set<"+-">()) >> imp::Some(imp::Digit()) >>
        imp::Opt(
            imp::Str<".">() >> imp::Some(imp::Digit()) >>
            imp::Opt(
                imp::Set<"eE">() >>
                imp::Opt(imp::Set<"+-">()) >> imp::Some(imp::Digit())
            )
        )
    )

    IMP_MAKE_RULE(eval,
        imp::Fn<spacing>() >> imp::Opt(
            imp::Fn<value>() >> imp::Fn<spacing>()
        ) >> imp::NoChars() /=
        [] (imp::MemStream& stream, const imp::Match& m, imp::CapturesView, const std::any&) -> imp::Match {
            if (!m)
                fprintf(stderr, "failed to parse JSON at %zi\n", stream.GetPos());
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

    imp::MappedFile
        mmfileJSON  = {strvFile};

    if (!mmfileJSON) {
        fprintf(stderr, "failed to open file: %s\n",
            strvFile.data());

        return EXIT_FAILURE;
    }

    imp::MemStream
        stream      = mmfileJSON.GetView();
    return (bool)imp::Eval(imp::Fn<grammJSON::eval>(), stream)
        ? EXIT_SUCCESS : EXIT_FAILURE;
}