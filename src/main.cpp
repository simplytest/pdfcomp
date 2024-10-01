#include "pdf.hpp"

#include <print>
#include <filesystem>

#include <clap/clap.hpp>

int main(int argc, char **argv)
{
    namespace fs = std::filesystem;

    struct
    {
        fs::path first;
        fs::path second;

      public:
        struct
        {
            fs::path path;
            bool enabled{};
        } diff;

        struct
        {
            double value{0};
            bool enabled{};
        } tolerance;
    } args;

    auto options = clap::Options{
        "pdfcomp",
        "A utility to compare PDF files",
        "1.0.0",
    };

    options                                //
        .positional(args.first, "first")   //
        .positional(args.second, "second") //
        .optional(args.tolerance.value, args.tolerance.enabled, "t,tol", "Absolute tolerance", "<value>")
        .optional(args.diff.path, args.diff.enabled, "d,diff", "Folder to save a difference image(s) to", "<path>");

    auto result = options.parse(argc, argv);

    if (clap::should_quit(result))
    {
        return clap::return_code(result);
    }

    auto first = pdfcomp::pdf::from(args.first);

    if (!first)
    {
        std::println(stderr, "Failed to parse file: '{}'", args.first.string());
        return 1;
    }

    auto second = pdfcomp::pdf::from(args.second);

    if (!second)
    {
        std::println(stderr, "Failed to parse file: '{}'", args.second.string());
        return 1;
    }

    auto diff = first->compare(second.value(), args.diff.enabled ? std::optional{args.diff.path} : std::nullopt);

    if (!diff.has_value())
    {
        switch (diff.error())
        {
        case pdfcomp::error::bad_directory:
            std::println(stderr, "Given output directory ('{}') is not valid", args.diff.path.string());
            break;
        case pdfcomp::error::mismatching_pages:
            std::println(stderr, "Given PDFs have differing page count ({}/{})", first->pages(), second->pages());
            break;
        default:
            break;
        }

        return 1;
    }

    if (diff.value() > args.tolerance.value)
    {
        std::println(stderr, "Difference exceeds tolerance: {}", diff.value());
        return 2;
    }

    std::println("Given PDFs are equal");

    return 0;
}
