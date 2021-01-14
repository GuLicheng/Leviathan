#ifndef __TENSOR_HPP__
#define __TENSOR_HPP__

#include <vector>
#include <cstddef>

namespace leviathan::numeric
{

template <typename T, size_t N>
class tensor
{
public:
    explicit tensor(size_t size = 3);
    
    tensor<T, N - 1>& operator[](size_t dimension);
    
    const tensor<T, N - 1>& operator[](size_t dimension) const;

    void resize(size_t size);

    size_t size() const noexcept
    {
        return m_data.size();
    }

    virtual ~tensor() = default;

    auto begin() noexcept
    {
        return m_data.begin();
    }

    auto end() noexcept
    {
        return m_data.end();
    }

    auto begin() const noexcept
    {
        return m_data.begin();
    }

    auto end() const noexcept
    {
        return m_data.end();
    }

private:
    std::vector<tensor<T, N - 1>> m_data;
};

template <typename T>
class tensor<T, 1>
{
public:
    explicit tensor(size_t size = 3);

    T& operator[](size_t dimension);
    
    const T& operator[](size_t dimension) const;
    
    void resize(size_t size);
    
    auto size() const noexcept
    {
        return m_data.size();
    }

    auto begin() noexcept
    {
        return m_data.begin();
    }

    auto end() noexcept
    {
        return m_data.end();
    }

    auto begin() const noexcept
    {
        return m_data.begin();
    }

    auto end() const noexcept
    {
        return m_data.end();
    }

    virtual ~tensor() = default;

private:
    std::vector<T> m_data;
};

template <typename T, size_t N>
tensor<T, N>::tensor(size_t size)
{
    this->resize(size);
}

template <typename T, size_t N>
void tensor<T, N>::resize(size_t size)
{
    this->m_data.resize(size);
    for (auto& elem : m_data)
        elem.resize(size);
}

template <typename T, size_t N>
tensor<T, N - 1>& tensor<T, N>::operator[](size_t dimension)
{
    return m_data[dimension];
}

template <typename T, size_t N>
const tensor<T, N - 1>& tensor<T, N>::operator[](size_t dimension) const
{
    return m_data[dimension];
}

template <typename T>
tensor<T, 1>::tensor(size_t size)
{
    this->resize(size);
}

template <typename T>
void tensor<T, 1>::resize(size_t size)
{
    this->m_data.resize(size);
}

template <typename T>
T& tensor<T, 1>::operator[](size_t index)
{
    return this->m_data[index];
}

template <typename T>
const T& tensor<T, 1>::operator[](size_t index) const 
{
    return this->m_data[index];
}

}  // namespace numeric



#endif  // __TENSOR_HPP__