#include <print>
#include <atomic>
#include <chrono>

#include <Patterns.hpp>
#include <MappedFile.hpp>

#define IMP_CAPTURETAG  int
#define IMP_USERDATA    int
using pt = IMP_PATTERNS();

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
        pt::Any(pt::Set<pt::Space + pt::LineBreak>())
    )

    IMP_MAKE_RULE(object,
        pt::Str<"{">() >> pt::Fn<spacing>() >>
        pt::Opt(
            pt::Fn<field>() >> pt::Fn<spacing>() >> pt::Any(
                pt::Str<",">() >> pt::Fn<spacing>() >>
                pt::Fn<field>() >> pt::Fn<spacing>()
            )
        ) >> pt::Str<"}">()
    )

    IMP_MAKE_RULE(array,
        pt::Str<"[">() >> pt::Fn<spacing>() >> pt::Opt(
            pt::Fn<value>() >> pt::Fn<spacing>() >>
            pt::Any(
                pt::Str<",">() >> pt::Fn<spacing>() >>
                pt::Fn<value>() >> pt::Fn<spacing>()
            )
        ) >> pt::Str<"]">()
    )

    IMP_MAKE_RULE(field,
        pt::Fn<string>() >> pt::Fn<spacing>() >>
        pt::Str<":">() >> pt::Fn<spacing>() >>
        pt::Fn<value>()
    )

    IMP_MAKE_RULE(value,
        pt::Fn<object>()    | pt::Fn<array>()  |
        pt::Fn<string>()    | pt::Fn<number>() |
        pt::Fn<boolean>()   | pt::Fn<null>()
    )

    IMP_MAKE_RULE(boolean, (
        pt::Dict<"true", "false">()
    ))

    IMP_MAKE_RULE(null,
        pt::Str<"null">()
    )

    IMP_MAKE_RULE(string,
        pt::Str<"\"">() >>
        pt::Any(pt::NegSet<"\"\\">()) >> pt::Any(
            pt::Fn<escseq>() >> pt::Any(pt::NegSet<"\"\\">())
        ) >> pt::Str<"\"">()
    )

    IMP_MAKE_RULE(escseq,
        pt::Str<"\\">() >> (
            pt::Set<"/\"\\bfnrt">() |
            pt::Str<"u">() >> pt::Exactly<4>(pt::Set<pt::HexDigit>()) |
            pt::Str<"U">() >> pt::Exactly<8>(pt::Set<pt::HexDigit>())
        )
    )

    IMP_MAKE_RULE(number,
        pt::Opt(pt::Set<"+-">()) >> pt::Some(pt::Set<pt::Digit>()) >>
        pt::Opt(
            pt::Str<".">() >> pt::Some(pt::Set<pt::Digit>()) >>
            pt::Opt(
                pt::Set<"eE">() >>
                pt::Opt(pt::Set<"+-">()) >> pt::Some(pt::Set<pt::Digit>())
            )
        )
    )

    IMP_MAKE_HANDLER(static eval_handler, {
        if (!m)
                std::print(stderr, "failed to parse JSON at {}\n", stream.GetPos());
            else
                std::print("success\n");
            return m;
    });

    IMP_MAKE_RULE(eval,
        pt::Fn<spacing>() >> pt::Opt(
            pt::Fn<value>() >> pt::Fn<spacing>()
        ) >> pt::NoChars() / pt::Fn<eval_handler>()
    )
}

int main() {
    static constexpr std::string_view
        strvFile    = "./assets/test.json";

    try {
        imp::MappedFile
            mapped_file = {strvFile};
        std::span<const char>
            file_view   = mapped_file.GetView();

        if (file_view.empty())
            throw std::runtime_error("empty file view");

        std::atomic_signal_fence(
            std::memory_order_seq_cst);
        auto
            tmStart     = std::chrono::high_resolution_clock::now();

        imp::MemStream
            stream      = {file_view};
        imp::Match
            mResult     = pt::Eval(pt::Fn<grammJSON::eval>(), stream);

        std::atomic_signal_fence(
            std::memory_order_seq_cst);
        auto
            tmEnd       = std::chrono::high_resolution_clock::now();

        size_t
            uFileSize   = file_view.size_bytes();
        double
            secParseTm  = std::chrono::duration<double>{tmEnd - tmStart}.count(),
            fParseSpeed = (double)uFileSize / (1024 * 1024) / secParseTm;
        std::print("parse time: {:.3f} sec ({:.2f} MB/s)\n",
            secParseTm, fParseSpeed);

        return (bool)mResult
            ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (const std::exception& err) {
        std::print(stderr, "Error: {}\n",
            err.what());
        return EXIT_FAILURE;
    }
    catch (const boost::exception& err) {
        std::print(stderr, "Error: {}\n",
            boost::diagnostic_information(err));
        return EXIT_FAILURE;
    }
}