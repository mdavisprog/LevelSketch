/**

MIT License

Copyright (c) 2024 Mitchell Davis <mdavisprog@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "../Traits.hpp"
#include "Array.hpp"
#include "HashMapConstants.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

template<typename T, typename TTraits = Traits::Base<T>, typename Constants = HashMapConstants>
class HashSet
{
public:
    using BucketType = Array<T>;

    HashSet()
    {
        m_Buckets.Resize(Constants::BucketSize());
    }

    HashSet(const std::initializer_list<T>& List)
    {
        for (u64 I = 0; I < List.size(); I++)
        {
            Insert(List.begin()[I]);
        }
    }

    ~HashSet()
    {
        Clear();
    }

    bool IsEmpty() const
    {
        return m_Size == 0;
    }

    u64 Size() const
    {
        return m_Size;
    }

    bool Contains(const T& Key) const
    {
        const u64 Index { Hash(Key) };
        const BucketType& Bucket { m_Buckets[Index] };
        return Bucket.Contains(Key);
    }

    bool Insert(const T& Key)
    {
        ConditionalGrow();
        return TryInsert(Key);
    }

    bool Remove(const T& Key)
    {
        const u64 Index { Hash(Key) };
        BucketType& Bucket { m_Buckets[Index] };

        if (!Bucket.Remove(Key))
        {
            return false;
        }

        m_Size--;

        return true;
    }

    void Clear()
    {
        m_Buckets.Clear();
        m_Size = 0;
    }

    Array<T> Keys() const
    {
        Array<T> Result {};

        for (const BucketType& Bucket : m_Buckets)
        {
            for (const T& Key : Bucket)
            {
                Result.Push(Key);
            }
        }

        return Result;
    }

private:
    bool TryInsert(const T& Key)
    {
        const u64 Index { Hash(Key) };
        BucketType& Bucket { m_Buckets[Index] };

        if (Bucket.Contains(Key))
        {
            return false;
        }

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
        {
            Bucket.Push(std::move(Key));
        }
        else
        {
            Bucket.Push(Key);
        }

        m_Size++;

        return true;
    }

    u64 Hash(const T& Key) const
    {
        return TTraits::Hash(Key) % m_Buckets.Size();
    }

    void ConditionalGrow()
    {
        if (Constants::ShouldGrow(m_Size, m_Buckets.Size()))
        {
            const u64 NewCapacity { Max<u64>(Constants::Grow(m_Buckets.Size()),
                m_Buckets.Size() + Constants::BucketSize()) };

            Array<BucketType> Buckets { std::move(m_Buckets) };
            m_Buckets.Resize(NewCapacity);
            m_Size = 0;

            for (BucketType& Bucket : Buckets)
            {
                for (T& Key : Bucket)
                {
                    TryInsert(Key);
                }
            }
        }
    }

    Array<BucketType> m_Buckets {};
    u64 m_Size { 0 };
};

}
}

template<typename T, typename TTraits = Core::Traits::Base<T>, typename Constants = Core::Containers::HashMapConstants>
using HashSet = Core::Containers::HashSet<T, TTraits, Constants>;

}
