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

#include "../Memory/UniquePtr.hpp"
#include "../Types.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

//
// Doubly-Linked list.
//
// The list and all nodes own the pointer to the next node.
// Nodes have a non-owning previous pointer.
//

template<typename T>
class List
{
public:
    class Node
    {
        friend class List;

    public:

        Node const* Next() const
        {
            return m_Next.Get();
        }

        Node const* Previous() const
        {
            return m_Previous;
        }

        T& Data()
        {
            return m_Data;
        }

        const T& Data() const
        {
            return m_Data;
        }

    private:
        Node() = default;
        Node(Node&&) = default;
        Node& operator=(Node&&) = default;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        Node(T&& Data)
            : m_Data(std::move(Data))
        {
        }

        UniquePtr<Node> m_Next { nullptr };
        Node* m_Previous { nullptr };
        T m_Data { 0 };
    };

    List() = default;
    List(List&&) = default;
    List& operator=(List&&) = default;

    List(const List&) = delete;
    List& operator=(const List&) = delete;

    bool IsEmpty() const
    {
        return m_Size == 0;
    }

    u64 Size() const
    {
        return m_Size;
    }

    Node const* Head() const
    {
        return m_Head.Get();
    }

    Node const* Tail() const
    {
        return m_Tail;
    }

    List& InsertBeginning(T&& Data)
    {
        if (m_Head == nullptr)
        {
            m_Head = UniquePtr<Node>::Adopt(new Node(std::move(Data)));
            m_Tail = m_Head.Get();
            m_Size++;
        }
        else
        {
            InsertBefore(m_Head, std::move(Data));
        }

        return *this;
    }

    List& InsertEnd(T&& Data)
    {
        if (m_Tail == nullptr)
        {
            InsertBeginning(std::move(Data));
        }
        else
        {
            if (m_Tail->m_Previous == nullptr)
            {
                InsertAfter(m_Head, std::move(Data));
            }
            else
            {
                InsertAfter(m_Tail->m_Previous->m_Next, std::move(Data));
            }
        }

        return *this;
    }

    bool Delete(const T& Data)
    {
        Node* Step { m_Head.Get() };

        while (Step != nullptr)
        {
            if (Step->m_Data == Data)
            {
                if (Step->m_Previous == nullptr)
                {
                    Delete(m_Head);
                }
                else
                {
                    Delete(Step->m_Previous->m_Next);
                }

                return true;
            }

            Step = Step->m_Next.Get();
        }

        return false;
    }

private:
    List& InsertAfter(UniquePtr<Node>& Anchor, T&& Data)
    {
        UniquePtr<Node> New { UniquePtr<Node>::Adopt(new Node(std::move(Data))) };
        New->m_Previous = Anchor.Get();

        if (Anchor->m_Next == nullptr)
        {
            m_Tail = New.Get();
        }
        else
        {
            New->m_Next = std::move(Anchor->m_Next);
            New->m_Next->m_Previous = New.Get();
        }

        Anchor->m_Next = std::move(New);
        m_Size++;

        return *this;
    }

    List& InsertBefore(UniquePtr<Node>& Anchor, T&& Data)
    {
        UniquePtr<Node> New { UniquePtr<Node>::Adopt(new Node(std::move(Data))) };
        New->m_Next = std::move(Anchor);
        Node* Anchor_ { New->m_Next.Get() };

        if (Anchor_->m_Previous == nullptr)
        {
            m_Head = std::move(New);
        }
        else
        {
            New->m_Previous = Anchor_->m_Previous;
            Anchor_->m_Previous->m_Next = std::move(New);
        }

        Anchor_->m_Previous = New.Get();
        m_Size++;

        return *this;
    }

    List& Delete(UniquePtr<Node>& Target)
    {
        UniquePtr<Node> Local { std::move(Target) };
        Node* Previous { Local->m_Previous };
        Node* Next { Local->m_Next.Get() };

        if (Local->m_Previous == nullptr)
        {
            m_Head = std::move(Local->m_Next);
        }
        else
        {
            Previous->m_Next = std::move(Local->m_Next);
        }

        if (Next == nullptr)
        {
            m_Tail = Previous;
        }
        else
        {
            Next->m_Previous = Previous;
        }

        m_Size--;

        return *this;
    }

    UniquePtr<Node> m_Head { nullptr };
    Node* m_Tail { nullptr };
    u64 m_Size { 0 };
};

}
}

template<typename T>
using List = Core::Containers::List<T>;

}
