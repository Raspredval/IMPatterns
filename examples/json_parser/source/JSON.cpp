#include <print>
#include <chrono>
#include <atomic>

#include <Patterns.hpp>
#include <MappedFile.hpp>


namespace grammJSON {
    using namespace imp;

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
        Any(Set<Space + NewLine>())
    )

    IMP_MAKE_RULE(object,
        Str<"{">() >> Fn<spacing>() >>
        Opt(
            Fn<field>() >> Fn<spacing>() >> Any(
                Str<",">() >> Fn<spacing>() >>
                Fn<field>() >> Fn<spacing>()
            )
        ) >> Str<"}">()
    )

    IMP_MAKE_RULE(array,
        Str<"[">() >> Fn<spacing>() >> Opt(
            Fn<value>() >> Fn<spacing>() >>
            Any(
                Str<",">() >> Fn<spacing>() >>
                Fn<value>() >> Fn<spacing>()
            )
        ) >> Str<"]">()
    )

    IMP_MAKE_RULE(field,
        Fn<string>() >> Fn<spacing>() >>
        Str<":">() >> Fn<spacing>() >>
        Fn<value>()
    )

    IMP_MAKE_RULE(value,
        Fn<object>()  | Fn<array>()  |
        Fn<string>()  | Fn<number>() |
        Fn<boolean>() | Fn<null>()
    )

    IMP_MAKE_RULE(boolean, (
        Dict<"true", "false">()
    ))

    IMP_MAKE_RULE(null,
        Str<"null">()
    )

    IMP_MAKE_RULE(string,
        Str<"\"">() >>
        Any(NegSet<"\"\\">()) >> Any(
            Fn<escseq>() >> Any(NegSet<"\"\\">())
        ) >> Str<"\"">()
    )

    IMP_MAKE_RULE(escseq,
        Str<"\\">() >> (
            Set<"/\"\\bfnrt">() |
            Str<"u">() >> Exactly<4>(Set<HexDigit>()) |
            Str<"U">() >> Exactly<8>(Set<HexDigit>())
        )
    )

    IMP_MAKE_RULE(number,
        Opt(Set<"+-">()) >> Some(Set<Digit>()) >>
        Opt(
            Str<".">() >> Some(Set<Digit>()) >>
            Opt(
                Set<"eE">() >>
                Opt(Set<"+-">()) >> Some(Set<Digit>())
            )
        )
    )

    IMP_MAKE_RULE(eval,
        Fn<spacing>() >> Opt(
            Fn<value>() >> Fn<spacing>()
        ) >> NoChars() /=
        [] (MemStream& stream, Match m, CaptureView, UserData) -> Match {
            if (!m)
                std::print(stderr, "failed to parse JSON at {}\n", stream.GetPos());
            else
                std::print("success\n");
            return m;
        }
    )
}

int main() {
    static constexpr std::string_view
        strvFile    = "./assets/test.json";

    imp::MappedFile
        mmfileJSON  = {strvFile};

    if (!mmfileJSON) {
        std::print(stderr, "failed to open file: {}\n",
            strvFile);

        return EXIT_FAILURE;
    }

    std::atomic_signal_fence(
        std::memory_order_seq_cst);
    auto
        tmStart     = std::chrono::high_resolution_clock::now();

    imp::MemStream
        stream      = mmfileJSON.GetView();
    imp::Match
        mResult     = imp::Eval(imp::Fn<grammJSON::eval>(), stream);

    std::atomic_signal_fence(
        std::memory_order_seq_cst);
    auto
        tmEnd       = std::chrono::high_resolution_clock::now();

    size_t
        uFileSize   = mmfileJSON.GetView().size_bytes();
    double
        secParseTm  = std::chrono::duration<double>{tmEnd - tmStart}.count(),
        fParseSpeed = (double)uFileSize / (1024 * 1024) / secParseTm;
    std::print("parse time: {:.3f} sec ({:.2f} MB/s)\n",
        secParseTm, fParseSpeed);

    return (bool)mResult
        ? EXIT_SUCCESS : EXIT_FAILURE;
}