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
#include "Pair.hpp"
#include "RedBlackTree.hpp"

#include <utility>

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

template<typename K, typename V>
class Map : private RedBlackTree<K, V>
{
public:
    using ValueType = Pair<K, V>;

    Map() = default;
    Map(Map&&) = default;
    Map& operator=(Map&&) = default;

    // FIXME: Don't allow copy until RedBlackTree copy is implemented.
    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;

    // FIXME: Initializer list operate on copy only. Would like to offer
    // a move version instead. Look into variadic templates or std::array.
    Map(const std::initializer_list<ValueType>& List)
    {
        Insert(List);
    }

    bool IsEmpty() const
    {
        return RedBlackTree<K, V>::IsEmpty();
    }

    u64 Size() const
    {
        return RedBlackTree<K, V>::Size();
    }

    Map& Clear()
    {
        RedBlackTree<K, V>::Clear();
        return *this;
    }

    V& operator[](const K& Key)
    {
        typename RedBlackTree<K, V>::Node* Result = RedBlackTree<K, V>::TryInsert(Key, V(0), false);
        return Result->Value();
    }

    const V& operator[](const K& Key) const
    {
        V const* Result { RedBlackTree<K, V>::Find(Key) };
        LS_ASSERT(Result != nullptr);
        return *Result;
    }

    bool Contains(const K& Key) const
    {
        return RedBlackTree<K, V>::Find(Key) != nullptr;
    }

private:
    void Insert(const std::initializer_list<ValueType>& List)
    {
        for (std::size_t I = 0; I < List.size(); I++)
        {
            const ValueType& Item { List.begin()[I] };
            RedBlackTree<K, V>::Insert(Item.First, std::move(Item.Second));
        }
    }
};

}
}

template<typename K, typename V>
using Map = Core::Containers::Map<K, V>;

}
