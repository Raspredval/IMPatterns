#include <cstdio>
#include <memory>
#include <Patterns.hpp>

namespace grammar::JSON {
    IMP_DECL(spacing);
    IMP_DECL(object);
    IMP_DECL(array);
    IMP_DECL(field);
    IMP_DECL(value);
    IMP_DECL(boolean);
    IMP_DECL(null);
    IMP_DECL(string);
    IMP_DECL(strfill);
    IMP_DECL(escseq);
    IMP_DECL(number);
    IMP_DECL(numint);
    IMP_DECL(numfract);
    IMP_DECL(numexp);

    IMP_PROXY(eval, (
        imp::Handle(imp::Join(
            spacing,
            imp::UpTo<1>(imp::Join(
                value, spacing
            )),
            imp::None()
        ),
        [] (FILE* hFile, const imp::Match& m, imp::CapturesView, const std::any&) {
            if (!m) {
                fprintf(stderr, "failed to parse JSON at %zi\n", ftell(hFile));
                return;
            }

            m.ExportTo(hFile, stdout);
            printf("\n");
        })
    ))

    IMP_PROXY(spacing, (
        imp::AtLeast<0>(imp::SpaceOrNewLine())
    ))

    IMP_PROXY(object, (
        imp::Join(
            imp::Str<"{">(), spacing,
            imp::UpTo<1>(imp::Join(
                field, spacing,
                imp::AtLeast<0>(imp::Join(
                    imp::Str<",">(), spacing,
                    field, spacing
                ))
            )), imp::Str<"}">()
        )
    ))

    IMP_PROXY(array, (
        imp::Join(
            imp::Str<"[">(), spacing,
            imp::UpTo<1>(imp::Join(
                value, spacing,
                imp::AtLeast<0>(imp::Join(
                    imp::Str<",">(), spacing, value, spacing
                ))
            )), imp::Str<"]">()
        )
    ))

    IMP_PROXY(field, (
        imp::Join(
            string, spacing,
            imp::Str<":">(), spacing,
            value
        )
    ))

    IMP_PROXY(value, (
        imp::Choice(
            object, array, string, number, boolean, null
        )
    ))

    IMP_PROXY(boolean, (
        imp::Choice(
            imp::Str<"true">(), imp::Str<"false">()
        )
    ))

    IMP_PROXY(null, (
        imp::Str<"null">()
    ))

    IMP_PROXY(string, (
        imp::Join(
            imp::Str<"\"">(), strfill,
            imp::AtLeast<0>(imp::Join(
                escseq, strfill
            )), imp::Str<"\"">()
        )
    ))

    IMP_PROXY(strfill, (
        imp::AtLeast<0>(imp::Join(
            imp::LookAhead(imp::Any()),
            imp::Not(imp::Set<"\"\\">())
        ))
    ))

    IMP_PROXY(escseq, (
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

    IMP_PROXY(number, (
        imp::Join(
            numint,
            imp::UpTo<1>(imp::Join(
                imp::Str<".">(), numfract
            ))
        )
    ))

    IMP_PROXY(numint, (
        imp::Join(
            imp::UpTo<1>(imp::Str<"-">()),
            imp::AtLeast<1>(imp::Digit())
        )
    ))

    IMP_PROXY(numfract, (
        imp::Join(
            imp::AtLeast<1>(imp::Digit()),
            imp::UpTo<1>(imp::Join(
                imp::Set<"eE">(), numexp
            ))
        )
    ))

    IMP_PROXY(numexp, (
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

    return (imp::Eval(grammar::JSON::eval, uptrFile.get()))
        ? EXIT_SUCCESS : EXIT_FAILURE;
}