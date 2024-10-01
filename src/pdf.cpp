#include "pdf.hpp"

#include <numeric>

#include <mutex>
#include <print>

#include <vector>
#include <ranges>
#include <format>

#include <Magick++.h>

namespace pdfcomp
{
    struct pdf::impl
    {
        std::vector<Magick::Image> pages;
    };

    pdf::pdf(impl data) : m_impl(std::make_unique<impl>(std::move(data))) {}

    pdf::~pdf() = default;

    pdf::pdf(pdf &&other) noexcept : m_impl(std::move(other.m_impl)) {}

    pdf::pdf(const pdf &other) noexcept : m_impl(std::make_unique<impl>(*other.m_impl)) {}

    std::size_t pdf::pages() const
    {
        return m_impl->pages.size();
    }

    tl::expected<double, error> pdf::compare(const pdf &other, std::optional<fs::path> output) const
    {
        if (m_impl->pages.size() != other.m_impl->pages.size())
        {
            return tl::unexpected{error::mismatching_pages};
        }

        std::vector<std::pair<Magick::Image, double>> comp;

        for (const auto &[first, second] : std::views::zip(m_impl->pages, other.m_impl->pages))
        {
            double distortion{};
            auto image = first.compare(second, Magick::AbsoluteErrorMetric, &distortion);

            comp.emplace_back(image, distortion);
        }

        const auto values = comp | std::views::values;
        auto differences  = std::accumulate(values.begin(), values.end(), 0.0);

        if (!output)
        {
            return differences;
        }

        std::error_code ec{};
        fs::create_directories(output.value());

        if (!fs::is_directory(output.value()))
        {
            return tl::unexpected{error::bad_directory};
        }

        for (auto [index, elem] : comp | std::views::enumerate)
        {
            auto &[image, difference] = elem;

            if (difference <= 0)
            {
                continue;
            }

            const auto path = output.value() / std::format("{}.png", index);
            image.write(path.string());
        }

        return differences;
    }

    tl::expected<pdf, error> pdf::from(const fs::path &document)
    {
        static std::once_flag flag;
        std::call_once(flag, []() { Magick::InitializeMagick(fs::current_path().string().c_str()); });

        auto path = fs::canonical(document);
        auto data = impl{};

        try
        {
            Magick::readImages(&data.pages, path.string());
        }
        catch (Magick::Error &err)
        {
            std::println(stderr, "Error ('{}'): {}", path.string(), err.what());
            return tl::unexpected{error::bad_file};
        }

        return pdf{std::move(data)};
    }
} // namespace pdfcomp
