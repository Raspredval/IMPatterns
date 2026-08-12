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
    IMP_DECL_RULE(static strfill);
    IMP_DECL_RULE(static escseq);
    IMP_DECL_RULE(static number);
    IMP_DECL_RULE(static numint);
    IMP_DECL_RULE(static numfract);

    IMP_MAKE_RULE(spacing,
        imp::AtLeast<0>(imp::SpaceOrNewLine())
    )

    IMP_MAKE_RULE(object,
        imp::Str<"{">() >> imp::Fn<spacing>() >>
        imp::UpTo<1>(
            imp::Fn<field>() >> imp::Fn<spacing>() >>
            imp::AtLeast<0>(
                imp::Str<",">() >> imp::Fn<spacing>() >>
                imp::Fn<field>() >> imp::Fn<spacing>()
            )
        ) >> imp::Str<"}">()
    )

    IMP_MAKE_RULE(array,
        imp::Str<"[">() >> imp::Fn<spacing>() >>
        imp::UpTo<1>(
            imp::Fn<value>() >> imp::Fn<spacing>() >>
            imp::AtLeast<0>(
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

    IMP_MAKE_RULE(boolean,
        imp::Str<"true">() | imp::Str<"false">()
    )

    IMP_MAKE_RULE(null,
        imp::Str<"null">()
    )

    IMP_MAKE_RULE(string,
        imp::Str<"\"">() >> imp::Fn<strfill>() >>
        imp::AtLeast<0>(
            imp::Fn<escseq>() >> imp::Fn<strfill>()
        ) >> imp::Str<"\"">()
    )

    IMP_MAKE_RULE(strfill,
        imp::AtLeast<0>(imp::NegSet<"\"\\">())
    )

    IMP_MAKE_RULE(escseq,
        imp::Str<"\\">() >> (
            imp::Set<"/\"\\bfnrt">() |
            imp::Set<"uU">() >> imp::Exactly<4>(imp::HexDigit())
        )
    )

    IMP_MAKE_RULE(number,
        imp::Fn<numint>() >> imp::UpTo<1>(imp::Fn<numfract>())
    )

    IMP_MAKE_RULE(numint,
        imp::UpTo<1>(imp::Set<"+-">()) >> imp::AtLeast<1>(imp::Digit())
    )

    IMP_MAKE_RULE(numfract,
        imp::Str<".">() >> imp::AtLeast<1>(imp::Digit()) >>
        imp::UpTo<1>(
            imp::Set<"eE">() >> imp::Fn<numint>()
        )
    )

    IMP_MAKE_RULE(eval,
        (imp::Fn<spacing>() >> imp::UpTo<1>(
            imp::Fn<value>() >> imp::Fn<spacing>()
        ) >> imp::None()) /
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