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

#include "../Types.hpp"
#include "../Memory/UniquePtr.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

//
// Left Leaning Red-Black tree
// 

template<typename K, typename V>
class RedBlackTree
{
public:
    class Node final
    {
        friend class RedBlackTree;

    public:
        enum class Color : bool
        {
            Red,
            Black
        };

        bool IsRed() const { return m_Color == Color::Red; }
        bool IsBlack() const { return m_Color == Color::Black; }

        Node* Left() const
        {
            return m_Left.Get();
        }

        Node* Right() const
        {
            return m_Right.Get();
        }

        K& Key()
        {
            return m_Key;
        }

        const K& Key() const
        {
            return m_Key;
        }

        V& Value()
        {
            return m_Value;
        }

        const V& Value() const
        {
            return m_Value;
        }

    private:
        static Color Opposite(Color Value)
        {
            return Value == Color::Red ? Color::Black : Color::Red;
        }

        Node()
        {
        }

        Node(const K& Key, V&& Value)
            : m_Key(Key)
            , m_Value(std::move(Value))
        {
        }

        Node(Node&& Other)
            : m_Color(Other.m_Color)
            , m_Left(std::move(Other.m_Left))
            , m_Right(std::move(Other.m_Right))
            , m_Key(std::move(Other.m_Key))
            , m_Value(std::move(Other.m_Value))
        {
        }

        void FlipColors()
        {
            m_Color = Opposite(m_Color);

            if (m_Left != nullptr)
            {
                m_Left->m_Color = Opposite(m_Left->m_Color);
            }

            if (m_Right != nullptr)
            {
                m_Right->m_Color = Opposite(m_Right->m_Color);
            }
        }

        Color m_Color { Color::Red };
        UniquePtr<Node> m_Left { nullptr };
        UniquePtr<Node> m_Right { nullptr };
        K m_Key { 0 };
        V m_Value { 0 };
    };

    RedBlackTree()
    {
    }

    bool IsEmpty() const
    {
        return m_Root == nullptr;
    }

    Node* Root() const
    {
        return m_Root.Get();
    }

    RedBlackTree& Insert(const K& Key, const V& Value)
    {
        return Insert(Key, V(Value));
    }

    RedBlackTree& Insert(const K& Key, V&& Value)
    {
        m_Root = Insert(m_Root, Key, std::move(Value));
        m_Root->m_Color = Node::Color::Black;
        return *this;
    }

    bool Delete(const K& Key)
    {
        m_Root = Delete(m_Root, Key);

        if (m_Root != nullptr)
        {
            m_Root->m_Color = Node::Color::Black;
        }

        return true;
    }

    V* Find(const K& Key)
    {
        Node* Root { m_Root.Get() };

        while (Root != nullptr)
        {
            if (Key == Root->m_Key)
            {
                return &Root->m_Value;
            }
            else if (Key < Root->m_Key)
            {
                Root = Root->m_Left.Get();
            }
            else
            {
                Root = Root->m_Right.Get();
            }
        }

        return nullptr;
    }

    u64 Size() const
    {
        return m_Size;
    }

    RedBlackTree& Clear()
    {
        m_Root = nullptr;
        m_Size = 0;
        return *this;
    }

private:
    UniquePtr<Node> Insert(UniquePtr<Node>& Root, const K& Key, V&& Value)
    {
        if (Root == nullptr)
        {
            Root = UniquePtr<Node>::Adopt(new Node(Key, std::move(Value)));
            m_Size++;
        }
        else
        {
            if (Key == Root->m_Key)
            {
                Root->m_Value = std::move(Value);
            }
            else if (Key < Root->m_Key)
            {
                Root->m_Left = Insert(Root->m_Left, Key, std::move(Value));
            }
            else
            {
                Root->m_Right = Insert(Root->m_Right, Key, std::move(Value));
            }

            if (IsRed(Root->m_Right) && !IsRed(Root->m_Left))
            {
                Root = RotateLeft(Root);
            }

            if (IsRed(Root->m_Left) && (Root->m_Left != nullptr && IsRed(Root->m_Left->m_Left)))
            {
                Root = RotateRight(Root);
            }

            if (IsRed(Root->m_Left) && IsRed(Root->m_Right))
            {
                Root->FlipColors();
            }
        }

        return std::move(Root);
    }

    UniquePtr<Node> Delete(UniquePtr<Node>& Root, const K& Key)
    {
        if (Root == nullptr)
        {
            return nullptr;
        }

        if (Key < Root->m_Key)
        {
            if (!IsRed(Root->m_Left) && (Root->m_Left != nullptr && !IsRed(Root->m_Left->m_Left)))
            {
                Root = MoveRedLeft(Root);
            }

            if (Root != nullptr)
            {
                Root->m_Left = Delete(Root->m_Left, Key);
            }
        }
        else
        {
            if (IsRed(Root->m_Left))
            {
                Root = RotateRight(Root);
            }

            if (Root != nullptr)
            {
                if (Key == Root->m_Key && Root->m_Right == nullptr)
                {
                    m_Size--;
                    return nullptr;
                }

                if (!IsRed(Root->m_Right) && (Root->m_Right != nullptr && !IsRed(Root->m_Right->m_Left)))
                {
                    Root = MoveRedRight(Root);
                }

                if (Root != nullptr)
                {
                    if (Key == Root->m_Key)
                    {
                        const Node* Anchor { Min(Root->m_Right.Get()) };
                        Root->m_Value = Anchor != nullptr ? Anchor->m_Value : V(0);
                        Root->m_Key = Anchor != nullptr ? Anchor->m_Key : K(0);
                        Root->m_Right = DeleteMin(Root->m_Right);
                        m_Size--;
                    }
                    else
                    {
                        Root->m_Right = Delete(Root->m_Right, Key);
                    }
                }
            }
        }

        return FixUp(Root);
    }

    UniquePtr<Node> DeleteMin(UniquePtr<Node>& Anchor)
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        if (Anchor->m_Left == nullptr)
        {
            return nullptr;
        }

        if (!IsRed(Anchor->m_Left) && (Anchor->m_Left != nullptr && !IsRed(Anchor->m_Left->m_Left)))
        {
            Anchor = MoveRedLeft(Anchor);
        }

        if (Anchor != nullptr)
        {
            Anchor->m_Left = DeleteMin(Anchor->m_Left);
        }

        return FixUp(Anchor);
    }

    UniquePtr<Node> RotateLeft(UniquePtr<Node>& Anchor) const
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        UniquePtr<Node> X { std::move(Anchor->m_Right) };
        Anchor->m_Right = std::move(X->m_Left);
        X->m_Color = Anchor->m_Color;
        X->m_Left = std::move(Anchor);

        if (X->m_Left != nullptr)
        {
            X->m_Left->m_Color = Node::Color::Red;
        }

        return std::move(X);
    }

    UniquePtr<Node> RotateRight(UniquePtr<Node>& Anchor) const
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        UniquePtr<Node> X { std::move(Anchor->m_Left) };
        Anchor->m_Left = std::move(X->m_Right);
        X->m_Color = Anchor->m_Color;
        X->m_Right = std::move(Anchor);

        if (X->m_Right != nullptr)
        {
            X->m_Right->m_Color = Node::Color::Red;
        }

        return std::move(X);
    }

    bool IsColor(const UniquePtr<Node>& Anchor, typename Node::Color Value) const
    {
        if (Anchor == nullptr)
        {
            return false;
        }

        return Anchor->m_Color == Value;
    }

    bool IsRed(const UniquePtr<Node>& Anchor) const
    {
        return IsColor(Anchor, Node::Color::Red);
    }

    UniquePtr<Node> MoveRedLeft(UniquePtr<Node>& Anchor) const
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        Anchor->FlipColors();

        if (Anchor->m_Right != nullptr && IsRed(Anchor->m_Right->m_Left))
        {
            Anchor->m_Right = RotateRight(Anchor->m_Right);
            Anchor = RotateLeft(Anchor);

            if (Anchor != nullptr)
            {
                Anchor->FlipColors();
            }
        }

        return std::move(Anchor);
    }

    UniquePtr<Node> MoveRedRight(UniquePtr<Node>& Anchor) const
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        Anchor->FlipColors();

        if (Anchor->m_Left != nullptr && IsRed(Anchor->m_Left->m_Left))
        {
            Anchor = RotateRight(Anchor);

            if (Anchor != nullptr)
            {
                Anchor->FlipColors();
            }
        }

        return std::move(Anchor);
    }

    UniquePtr<Node> FixUp(UniquePtr<Node>& Root) const
    {
        if (Root == nullptr)
        {
            return nullptr;
        }

        if (IsRed(Root->m_Right))
        {
            Root = RotateLeft(Root);
        }

        if (Root != nullptr && IsRed(Root->m_Left) && (Root->m_Left != nullptr && IsRed(Root->m_Left->m_Left)))
        {
            Root = RotateRight(Root);
        }

        if (Root != nullptr && IsRed(Root->m_Left) && IsRed(Root->m_Right))
        {
            Root->FlipColors();
        }

        return std::move(Root);
    }

    Node* Min(Node* Anchor) const
    {
        if (Anchor == nullptr)
        {
            return nullptr;
        }

        while (Anchor->m_Left != nullptr)
        {
            Anchor = Anchor->m_Left.Get();
        }

        if (Anchor == nullptr)
        {
            return nullptr;
        }

        return Anchor;
    }

    UniquePtr<Node> m_Root { nullptr };
    u64 m_Size { 0 };
};

}
}
}
