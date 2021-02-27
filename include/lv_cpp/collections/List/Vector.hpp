/*
Code for Data Structure and Algorithm Analysis in C++ (Fourth Edition)
                                                                    --------Mark Allen Weiss
 */


#include <algorithm>

#ifndef _VECTOR_HPP_
#define _VECTOR_HPP_

template<typename T>
class Vector{
public:
    explicit Vector(int size = 0) : size{size}, theCapacity{size + SPACE_CAPACITY}{
        object = new T[theCapacity];
    }    
    Vector(const Vector& rhs) : size{rhs.size}, theCapacity{rhs.theCapacity}, object{nullptr}{
        object = new T[theCapacity];
        for(register int k = 0; k < size; ++k)
            object[k] = rhs.object[k];
    }
    Vector(Vector&& rhs) : size{rhs.size}, theCapacity{rhs.thsCapacity}, object{rhs.object}{
        rhs.object = nullptr;
        rhs.size = 0;
        rhs.theCapacity = 0;
    }
    Vector& operator= (const Vector& rhs){
        Vector temp = rhs;
        std::swap(*this, temp);
        return *this;
    }
    Vector& operator= (Vector&& rhs){
        std::swap(size, rhs.size);
        std::swap(theCapacity, rhs.theCapacity);
        std::swap(object, rhs.object);
        return *this;
    }
    ~Vector(){
        delete[] object;
    }
    void reserve(int newCapacity){
        if(newCapacity < size) return;
        T *newarray = new T[newCapacity];
        for(register int i = 0; i < size; ++i){
            newarray[i] = std::move(object[i]);
        }
        theCapacity = newCapacity;
        std::swap(object, newarray);
        delete[] newarray;
    }
    void resize(int newsize){
        if(newsize > theCapacity) reserve(newsize << 1);
        size = newsize;
    }
    T& operator[](int index){
        return object[index];
    }
    const T& operator[](int index) const {
        return object[index];
    }
    bool empty() const {
        return size == 0;
    }
    int _size() const {
        return size;
    }
    int capacity() const {
        return theCapacity;
    }
    void push_back(T&& x){
        if(size == theCapacity) reserve((theCapacity << 1) + 1);
        object[size++] = std::move(x);
    }
    void push_back(const T& x){
        if(size == theCapacity) reserve((theCapacity << 1) + 1);
        object[size++] = x;
    }
    void pop_back(){
        --size;
    }
    const T& back() const {
        return object[size - 1];
    }

    static const int SPACE_CAPACITY = 16;
    
    using Iterator = T*;
    using Const_iterator = const T*;

    Iterator begin(){
        return &object[0];
    }
    Const_iterator begin() const {
        return &object[0];
    }
    Iterator end(){
        return &object[size];
    }
    Const_iterator end() const {
        return &object[size];
    }

private:

    int size;
    int theCapacity;
    T *object;

};



#endif // !_VECTOR_HPP_
