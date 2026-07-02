#include <cstdio>
#include <memory>
#include <Patterns.hpp>

namespace grammar::JSON {
    IMP_DECL_CPATTERN(spacing);
    IMP_DECL_CPATTERN(object);
    IMP_DECL_CPATTERN(array);
    IMP_DECL_CPATTERN(field);
    IMP_DECL_CPATTERN(value);
    IMP_DECL_CPATTERN(boolean);
    IMP_DECL_CPATTERN(null);
    IMP_DECL_CPATTERN(string);
    IMP_DECL_CPATTERN(strfill);
    IMP_DECL_CPATTERN(escseq);
    IMP_DECL_CPATTERN(number);
    IMP_DECL_CPATTERN(numint);
    IMP_DECL_CPATTERN(numfract);
    IMP_DECL_CPATTERN(numexp);

    IMP_MAKE_CPATTERN(eval, (
        imp::Handle(imp::Join(
            imp::Fn<spacing>(),
            imp::UpTo<1>(imp::Join(
                imp::Fn<value>(), imp::Fn<spacing>()
            )),
            imp::None()
        ),
        [] (FILE* hFile, const imp::Match& m, imp::CapturesView, const std::any&) {
            if (!m)
                fprintf(stderr, "failed to parse JSON at %zi\n", ftell(hFile));
            else
                printf("success\n");
        })
    ))

    IMP_MAKE_CPATTERN(spacing, (
        imp::AtLeast<0>(imp::SpaceOrNewLine())
    ))

    IMP_MAKE_CPATTERN(object, (
        imp::Join(
            imp::Str<"{">(), imp::Fn<spacing>(),
            imp::UpTo<1>(imp::Join(
                imp::Fn<field>(), imp::Fn<spacing>(),
                imp::AtLeast<0>(imp::Join(
                    imp::Str<",">(), imp::Fn<spacing>(),
                    imp::Fn<field>(), imp::Fn<spacing>()
                ))
            )),
            imp::Str<"}">()
        )
    ))

    IMP_MAKE_CPATTERN(array, (
        imp::Join(
            imp::Str<"[">(), imp::Fn<spacing>(),
            imp::UpTo<1>(imp::Join(
                imp::Fn<value>(), imp::Fn<spacing>(),
                imp::AtLeast<0>(imp::Join(
                    imp::Str<",">(), imp::Fn<spacing>(),
                    imp::Fn<value>(), imp::Fn<spacing>()
                ))
            )),
            imp::Str<"]">()
        )
    ))

    IMP_MAKE_CPATTERN(field, (
        imp::Join(
            imp::Fn<string>(), imp::Fn<spacing>(),
            imp::Str<":">(), imp::Fn<spacing>(),
            imp::Fn<value>()
        )
    ))

    IMP_MAKE_CPATTERN(value, (
        imp::Choice(
            imp::Fn<object>(), imp::Fn<array>(),
            imp::Fn<string>(), imp::Fn<number>(),
            imp::Fn<boolean>(), imp::Fn<null>()
        )
    ))

    IMP_MAKE_CPATTERN(boolean, (
        imp::Choice(
            imp::Str<"true">(), imp::Str<"false">()
        )
    ))

    IMP_MAKE_CPATTERN(null, (
        imp::Str<"null">()
    ))

    IMP_MAKE_CPATTERN(string, (
        imp::Join(
            imp::Str<"\"">(), imp::Fn<strfill>(),
            imp::AtLeast<0>(imp::Join(
                imp::Fn<escseq>(), imp::Fn<strfill>()
            )),
            imp::Str<"\"">()
        )
    ))

    IMP_MAKE_CPATTERN(strfill, (
        imp::AtLeast<0>(imp::Join(
            imp::LookAhead(imp::Any()),
            imp::Not(imp::Set<"\"\\">())
        ))
    ))

    IMP_MAKE_CPATTERN(escseq, (
        imp::Join(
            imp::Str<"\\">(),
            imp::Choice(
                imp::Set<"/\"\\bfnrt">(),
                imp::Join(
                    imp::Set<"uU">(), imp::Exactly<4>(imp::HexDigit())
                )
            )
        )
    ))

    IMP_MAKE_CPATTERN(number, (
        imp::Join(
            imp::Fn<numint>(),
            imp::UpTo<1>(imp::Join(
                imp::Str<".">(), imp::Fn<numfract>()
            ))
        )
    ))

    IMP_MAKE_CPATTERN(numint, (
        imp::Join(
            imp::UpTo<1>(imp::Str<"-">()),
            imp::AtLeast<1>(imp::Digit())
        )
    ))

    IMP_MAKE_CPATTERN(numfract, (
        imp::Join(
            imp::AtLeast<1>(imp::Digit()),
            imp::UpTo<1>(imp::Join(
                imp::Set<"eE">(), imp::Fn<numexp>()
            ))
        )
    ))

    IMP_MAKE_CPATTERN(numexp, (
        imp::Join(
            imp::UpTo<1>(imp::Set<"+-">()),
            imp::AtLeast<1>(imp::Digit())
        )
    ))
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

    return (imp::Eval(imp::Fn<grammar::JSON::eval>(), uptrFile.get()))
        ? EXIT_SUCCESS : EXIT_FAILURE;
}