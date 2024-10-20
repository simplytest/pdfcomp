#include "pdf.hpp"

#include <numeric>
#include <utility>

#include <mutex>
#include <print>

#include <vector>
#include <ranges>
#include <format>

#include <Magick++.h>

namespace pdfcomp
{
    using Magick::ColorRGB;
    using Magick::CompositeOperator;
    using Magick::Geometry;
    using Magick::Image;
    using Magick::MetricType;

    struct pdf::impl
    {
        std::vector<Image> pages;

      public:
        template <algorithm Algorithm>
        static std::pair<Image, Image> compare(Image &, Image &);
    };

    pdf::pdf(impl data) : m_impl(std::make_unique<impl>(std::move(data))) {}

    pdf::~pdf() = default;

    pdf::pdf(pdf &&other) noexcept : m_impl(std::move(other.m_impl)) {}

    pdf::pdf(const pdf &other) noexcept : m_impl(std::make_unique<impl>(*other.m_impl)) {}

    std::size_t pdf::pages() const
    {
        return m_impl->pages.size();
    }

    template <>
    std::pair<Image, Image> pdf::impl::compare<algorithm::simple>(Image &first, Image &second)
    {
        second.lowlightColor(ColorRGB{0, 0, 0, 0});
        second.highlightColor(ColorRGB{0, 0, 255});

        [[maybe_unused]] double distortion{};

        return {
            first.compare(second, MetricType::AbsoluteErrorMetric, &distortion),
            second,
        };
    }

    template <>
    std::pair<Image, Image> pdf::impl::compare<algorithm::difference>(Image &first, Image &second)
    {
        auto diff = first;
        diff.composite(second, 0, 0, CompositeOperator::ChangeMaskCompositeOp);

        first.lowlightColor(ColorRGB{0, 0, 0, 0});

        [[maybe_unused]] double distortion{};
        auto highlight = first.compare(second, MetricType::AbsoluteErrorMetric, &distortion);

        return {diff, highlight};
    }

    template <>
    std::pair<Image, Image> pdf::impl::compare<algorithm::double_compare>(Image &first, Image &second)
    {
        second.lowlightColor(ColorRGB{0, 0, 0, 0});
        second.highlightColor(ColorRGB{0, 0, 255});

        [[maybe_unused]] double distortion{};

        return {
            first.compare(second, MetricType::AbsoluteErrorMetric, &distortion),
            second.compare(first, MetricType::AbsoluteErrorMetric, &distortion),
        };
    }

    tl::expected<double, error> pdf::compare(const pdf &other, const options &opts) const
    {
        if (m_impl->pages.size() != other.m_impl->pages.size())
        {
            return tl::unexpected{error::mismatching_pages};
        }

        std::vector<double> diffs;

        for (const auto &[first, second] : std::views::zip(m_impl->pages, other.m_impl->pages))
        {
            first.colorFuzz(opts.fuzz);
            diffs.emplace_back(first.compare(second, MetricType::AbsoluteErrorMetric));
        }

        const auto total = std::accumulate(diffs.begin(), diffs.end(), 0.0);

        if (!opts.output)
        {
            return total;
        }

        const auto output = opts.output.value();

        std::error_code ec{};
        fs::create_directories(output);

        if (!fs::is_directory(output))
        {
            return tl::unexpected{error::bad_directory};
        }

        for (const auto &[index, difference] : diffs | std::views::enumerate)
        {
            if (difference <= opts.tolerance)
            {
                continue;
            }

            auto &first  = m_impl->pages[index];
            auto &second = other.m_impl->pages[index];

            std::pair<Image, Image> result;

            switch (opts.method)
            {
            case algorithm::simple:
                result = impl::compare<algorithm::simple>(first, second);
                break;
            case algorithm::difference:
                result = impl::compare<algorithm::difference>(first, second);
                break;
            case algorithm::double_compare:
                result = impl::compare<algorithm::double_compare>(first, second);
                break;
            }

            const auto &[middle, right] = result;
            const auto size             = first.size();

            auto canvas = first;
            auto extent = Geometry{size.width() * 3, size.height()};

            canvas.extent(extent);

            canvas.composite(middle, static_cast<ssize_t>(size.width()), 0, CompositeOperator::AtopCompositeOp);
            canvas.composite(right, static_cast<ssize_t>(size.width() * 2), 0, CompositeOperator::AtopCompositeOp);

            const auto path = output / std::format("{}{}.png", opts.prefix, index);
            canvas.write(path.string());
        }

        return total;
    }

    tl::expected<pdf, error> pdf::from(const fs::path &document, const std::string &density)
    {
        static std::once_flag flag;
        std::call_once(flag, []() { Magick::InitializeMagick(fs::current_path().string().c_str()); });

        std::error_code ec{};
        const auto path = fs::canonical(document, ec);

        if (ec)
        {
            return tl::unexpected{error::bad_file};
        }

        impl data{};

        Magick::ReadOptions options{};
        options.density(density);

        try
        {
            Magick::readImages(&data.pages, path.string(), options);
        }
        catch (Magick::Error &err)
        {
            std::println(stderr, "Error ('{}'): {}", path.string(), err.what());
            return tl::unexpected{error::bad_file};
        }
        catch (...)
        {
            std::println(stderr, "Unknown error");
            return tl::unexpected{error::bad_file};
        }

        return pdf{std::move(data)};
    }
} // namespace pdfcomp
