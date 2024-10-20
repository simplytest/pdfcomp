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
            fs::path value;
            bool enabled{};
        } output;

        struct
        {
            std::string value{"100%x100%"};
            bool enabled{};
        } density;

        struct
        {
            std::string value;
            bool enabled{};
        } prefix;

        struct
        {
            double value{0};
            bool enabled{};
        } fuzz;

        struct
        {
            double value{0};
            bool enabled{};
        } tolerance;

        struct
        {
            std::uint8_t value{0};
            bool enabled{};
        } method;
    } args;

    auto options = clap::Options{
        "pdfcomp",
        "A utility to compare PDF and image files",
        "2.1.0",
    };

    options                                //
        .positional(args.first, "first")   //
        .positional(args.second, "second") //
        .optional(args.tolerance.value, args.tolerance.enabled, "t,tol", "Absolute tolerance", "<value>")
        .optional(args.density.value, args.density.enabled, "d,density", "Density to read image in", "<value>")
        .optional(args.output.value, args.output.enabled, "o,output", "Folder to save a difference image(s) to", "<path>")
        .optional(args.fuzz.value, args.fuzz.enabled, "f,fuzz", "Fuzziness to use for comparison", "<value>")
        .optional(args.prefix.value, args.prefix.enabled, "p,prefix", "Filename prefix to use", "<value>")
        .optional(args.method.value, args.method.enabled, "m,method",
                  "Highlighting algorithm to use (0 = simple, 1 = difference, 2 = double compare)", "<value>");

    auto result = options.parse(argc, argv);

    if (clap::should_quit(result))
    {
        return clap::return_code(result);
    }

    const auto density = args.density.value;
    auto first         = pdfcomp::pdf::from(args.first, density);

    if (!first)
    {
        std::println(stderr, "Failed to parse file: '{}'", args.first.string());
        return 1;
    }

    auto second = pdfcomp::pdf::from(args.second, density);

    if (!second)
    {
        std::println(stderr, "Failed to parse file: '{}'", args.second.string());
        return 1;
    }

    if (args.method.value > 1)
    {
        std::println(stderr, "Invalid method specified ({})", args.method.value);
        return 1;
    }

    const auto output = args.output.enabled ? std::optional{args.output.value} : std::nullopt;

    auto diff = first->compare(second.value(), {
                                                   .fuzz      = args.fuzz.value,
                                                   .tolerance = args.tolerance.value,
                                                   .method    = static_cast<pdfcomp::algorithm>(args.method.value),
                                                   .prefix    = args.prefix.value,
                                                   .output    = output,
                                               });

    if (!diff.has_value())
    {
        switch (diff.error())
        {
        case pdfcomp::error::bad_directory:
            std::println(stderr, "Given output directory ('{}') is not valid", args.output.value.string());
            break;
        case pdfcomp::error::mismatching_pages:
            std::println(stderr, "Given PDFs have differing page count ({}/{})", first->pages(), second->pages());
            break;
        default:
            std::unreachable();
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
