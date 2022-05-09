#pragma once

#include <variant>

namespace sv {
    inline namespace modconfig {
        using ModConfigData_t = std::variant<std::monostate, bool, int, float, std::string, time_point_t, duration_t>;

        namespace impl {
            template<class T> struct ToHelperBase {
                using Base = ToHelperBase;
                T operator()(const T &x)
                {
                    return x;
                }
                T operator()(...)
                {
                    // nothing match
                    return {};
                }
            };
            struct ToStringHelper : ToHelperBase<std::string> {
                using Base::operator();
                template<class T> auto operator()(T x) -> decltype(std::to_string(x))
                {
                    return std::to_string(x);
                }
            };
            struct ToIntHelper : ToHelperBase<int> {
                using Base::operator();
                int operator()(const std::string &x)
                {
                    return atoi(x.c_str());
                }
            };
            struct ToFloatHelper : ToHelperBase<float> {
                using Base::operator();
                float operator()(const std::string &x)
                {
                    return atof(x.c_str());
                }
                float operator()(duration_t d)
                {
                    return d / 1s;
                }
            };
            struct ToDurationHelper : ToHelperBase<duration_t> {
                using Base::operator();
                template<class U>
                auto operator()(const U &x) -> typename std::enable_if<std::is_arithmetic<U>::value, duration_t>::type
                {
                    return x * 1s;
                }
            };
        }

        inline std::string to_string(const ModConfigData_t &data)
        {
            return std::visit(impl::ToStringHelper(), data);
        }

        inline int to_int(const ModConfigData_t & data)
        {
            return std::visit(impl::ToIntHelper(), data);
        }

        inline int to_float(const ModConfigData_t & data)
        {
            return std::visit(impl::ToIntHelper(), data);
        }

        inline int to_bool(const ModConfigData_t & data)
        {
            return std::visit(impl::ToHelperBase<bool>(), data);
        }

        inline time_point_t to_time_point(const ModConfigData_t & data)
        {
            return std::visit(impl::ToHelperBase<time_point_t>(), data);
        }

        inline duration_t to_duration(const ModConfigData_t & data)
        {
            return std::visit(impl::ToDurationHelper(), data);
        }
    }
}