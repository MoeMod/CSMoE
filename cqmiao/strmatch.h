#pragma once

#include <memory>
#include <algorithm>

namespace strmatch
{
    // 计算两个字符串的最小编辑距离
    inline int CalcMinEditDistance(const wchar_t *a, const wchar_t *b)
    {
        // 没有VLA用，buffer优先用栈上的
        int static_buffer[1024]; // 4kb栈不至于会溢出
        std::unique_ptr<int[]> dynamic_buffer = nullptr;
        int* buffer = static_buffer;
        const auto a_len = wcslen(a);
        const auto b_len = wcslen(b);
        const auto w = (b_len + 1);
        const auto allocate_size = (a_len + 1) * (b_len + 1);
        if (allocate_size > 1024)
        {
            dynamic_buffer = std::unique_ptr<int[]>(new int[allocate_size]);
            buffer = dynamic_buffer.get();
        }

        for (int i = 0; i <= a_len; ++i)
            buffer[i * w + 0] = i;
        for (int j = 0; j <= b_len; ++j)
            buffer[0 + j] = j;

        for (int i = 1; i <= a_len; ++i)
        {
            const wchar_t ch1 = a[i - 1];
            for (int j = 1; j <= b_len; ++j)
            {
                const wchar_t ch2 = b[j - 1];
                buffer[i * w + j] = std::min({ buffer[(i - 1) * w + j] + 1, buffer[i * w + (j - 1)] + 1, buffer[(i - 1) * w + (j - 1)] + (ch1 != ch2) });
            }
        }
        return buffer[a_len * w + b_len];
    }

}