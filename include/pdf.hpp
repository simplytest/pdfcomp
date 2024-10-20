#pragma once

#include <string>
#include <memory>

#include <filesystem>

#include <cstdint>
#include <optional>

#include <tl/expected.hpp>

namespace pdfcomp
{
    namespace fs = std::filesystem;

    enum class error : std::uint8_t
    {
        bad_file,
        bad_directory,
        mismatching_pages,
    };

    enum class algorithm : std::uint8_t
    {
        simple,
        difference,
        double_compare,
    };

    struct options
    {
        double fuzz{0};
        double tolerance{0};
        algorithm method{algorithm::simple};

      public:
        std::string prefix;
        std::optional<fs::path> output;
    };

    class pdf
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      private:
        pdf(impl);

      public:
        pdf(pdf &&other) noexcept;
        pdf(const pdf &other) noexcept;

      public:
        ~pdf();

      public:
        [[nodiscard]] std::size_t pages() const;
        [[nodiscard]] tl::expected<double, error> compare(const pdf &other, const options &opts) const;

      public:
        [[nodiscard]] static tl::expected<pdf, error> from(const fs::path &, const std::string &density);
    };
} // namespace pdfcomp
