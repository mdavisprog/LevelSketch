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

#include "../Assert.hpp"
#include "../Traits.hpp"
#include "Array.hpp"
#include "HashMapConstants.hpp"
#include "Pair.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

//
// Unordered Map
// A cache-concious container.
//

template<typename K, typename V, typename KTraits = Traits::Base<K>, typename Constants = HashMapConstants>
class HashMap
{
private:
    struct Entry
    {
        Pair<K, V> Contents;
        bool Occupied { false };
    };

public:
    using ValueType = Entry;
    using BucketType = Array<ValueType>;

    HashMap()
    {
        m_Buckets.Resize(Constants::BucketSize());
    }

    ~HashMap()
    {
        Clear();
    }

    bool IsEmpty() const
    {
        return m_Size == 0;
    }

    u64 Capacity() const
    {
        return m_Buckets.Size();
    }

    u64 Size() const
    {
        return m_Size;
    }

    V& operator[](const K& Key)
    {
        ConditionalGrow();
        ValueType* Result { TryInsert(Key) };
        return Result->Contents.Second;
    }

    const V& At(const K& Key) const
    {
        ValueType const* Result { Get(Key) };
        LS_ASSERT(Result != nullptr);
        return Result->Contents.Second;
    }

    bool Remove(const K& Key)
    {
        for (BucketType& Bucket : m_Buckets)
        {
            for (u64 I = 0; I < Bucket.Size(); I++)
            {
                ValueType& Value { Bucket[I] };

                if (Value.Contents.First == Key)
                {
                    Bucket.Remove(I);
                    Value.Occupied = false;
                    m_Size--;
                    return true;
                }
            }
        }

        return false;
    }

    void Clear()
    {
        m_Buckets.Clear();
        m_Size = 0;
    }

    Array<K> Keys() const
    {
        Array<K> Result;

        for (const BucketType& Bucket : m_Buckets)
        {
            for (const ValueType& Value : Bucket)
            {
                if (Value.Occupied)
                {
                    Result.Push(Value.Contents.First);
                }
            }
        }

        return Result;
    }

    bool Contains(const K& Key) const
    {
        for (const BucketType& Bucket : m_Buckets)
        {
            for (const ValueType& Value : Bucket)
            {
                if (Value.Contents.First == Key)
                {
                    return true;
                }
            }
        }

        return false;
    }

private:
    ValueType const* Find(const K& Key) const
    {
        const u64 Index { KTraits::Hash(Key) % m_Buckets.Size() };
        const BucketType& Bucket { m_Buckets[Index] };

        for (const ValueType& Value : Bucket)
        {
            if (Value.Contents.First == Key)
            {
                return &Value;
            }
        }

        return nullptr;
    }

    ValueType* TryInsert(const K& Key)
    {
        const u64 Index { Hash(Key) };
        BucketType& Bucket { m_Buckets[Index] };

        for (ValueType& Value : Bucket)
        {
            if (Value.Contents.First == Key)
            {
                return &Value;
            }
        }

        m_Size++;
        Bucket.Push({ MakePair(Key, V()), true });
        return &Bucket.Back();
    }

    ValueType const* Get(const K& Key) const
    {
        const u64 Index { Hash(Key) };
        const BucketType& Bucket { m_Buckets[Index] };

        for (const ValueType& Value : Bucket)
        {
            if (Value.Contents.First == Key)
            {
                return &Value;
            }
        }

        return nullptr;
    }

    u64 Hash(const K& Key) const
    {
        return KTraits::Hash(Key) % m_Buckets.Size();
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
                for (ValueType& Value : Bucket)
                {
                    ValueType* Element { TryInsert(Value.Contents.First) };

                    if constexpr (std::is_nothrow_move_constructible_v<V> || !std::is_copy_constructible_v<V>)
                    {
                        Element->Contents.Second = std::move(Value.Contents.Second);
                    }
                    else
                    {
                        Element->Contents.Second = Value.Contents.Second;
                    }
                }
            }
        }
    }

    Array<BucketType> m_Buckets {};
    u64 m_Size { 0 };
};

}
}

template<typename K,
    typename V,
    typename KTraits = Core::Traits::Base<K>,
    typename Constants = Core::Containers::HashMapConstants>
using HashMap = Core::Containers::HashMap<K, V, KTraits, Constants>;

}
