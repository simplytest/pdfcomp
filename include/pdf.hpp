#pragma once

#include <memory>
#include <optional>
#include <filesystem>

#include <tl/expected.hpp>

namespace pdfcomp
{
    namespace fs = std::filesystem;

    enum class error
    {
        bad_file,
        bad_directory,
        mismatching_pages,
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
        [[nodiscard]] tl::expected<double, error> compare(const pdf &other, std::optional<fs::path> output) const;

      public:
        [[nodiscard]] static tl::expected<pdf, error> from(const fs::path &);
    };
} // namespace pdfcomp
