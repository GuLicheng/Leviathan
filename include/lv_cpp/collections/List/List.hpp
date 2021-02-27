/*
Code for Data Structure and Algorithm Analysis in C++ (Fourth Edition)
                                                                        --------Mark Allen Weiss
 */
#include <algorithm>
#ifndef _LIST_HPP_
#define _LIST_HPP_

template<typename T>
class List{

private:
    struct Node{
        T data;
        Node *prev;
        Node *next;

        Node(const T& data = T{}, Node *p = nullptr, Node *q = nullptr)
            : data{data}, prev{p}, next{q} {}
        Node(T&& d, Node *p = nullptr, Node *q = nullptr)
            : data{std::move(d)}, prev{p}, next{q} {}
    };

public:

    class Const_iterator{
        public:
            Const_iterator() : current{nullptr}{}
            const T& operator*() const {
                return retrieve();
            }
            Const_iterator& operator++(){
                current = current->next;
                return *this;
            }
            Const_iterator& operator--(){
                current = current->prev;
                return *this;
            }
            Const_iterator operator++(int){
                auto old = current;
                ++ *this;
                return old;
            }
            Const_iterator operator--(int){
                auto old = *this;
                -- *this;
                return old;
            }
            bool operator==(const Const_iterator &rhs) const {
                return current == rhs.current;
            }
            bool operator!=(const Const_iterator &rhs) const {
                return current != rhs.current;
            }

        protected:
            Node *current;
            T &retrieve() const { 
                return current->data;
            }
            Const_iterator(Node *p) : current{p} {}
            friend class List<T>;
    };
    class Iterator : public Const_iterator{
        public:
            Iterator(){}

            T& operator*(){
                return Const_iterator::retrieve();
            }
            const T& operator*() const {
                return Const_iterator::operator*();
            }
            Iterator& operator++(){
                this->current = this->current->next;
                return *this;
            }
            Iterator& operator--(){
                this->current = this->current->prev;
                return *this;
            }
            Iterator operator++(int){
                auto old = *this;
                ++ *this;
                return old;
            }
            Iterator operator--(int){
                auto old = *this;
                -- *this;
                return old;
            }
        protected:
            Iterator(Node *p) : Const_iterator{p}{}
            friend class List<T>;
    };

    List(){init();}
    ~List(){
        clear();
        delete head;
        delete tail;
    }
    List(const List& rhs){
        init();
        for(auto &x : rhs)
            push_back(x);
    }
    List& operator=(const List& rhs){
        auto temp = rhs;
        std::swap(*this, temp);
        return *this;
    }
    List(List&& rhs) : _size{rhs._size}, head{rhs.head}, tail{rhs.tail} {
        rhs._size = 0;
        rhs.head = rhs.tail = nullptr;
    }
    List& operator=(List&& rhs){
        std::swap(_size, rhs._size);
        std::swap(head, rhs.head);
        std::swap(tail, rhs.tail);
        return *this;
    }
    void  init(){
        _size = 0;
        head = new Node;
        tail = new Node;
        head->next = tail;
        tail->prev = head;
    }

    Iterator begin(){
        return {head->next};
    }
    Const_iterator begin() const {
        return {head->next};
    }
    Iterator end(){
        return {tail};
    }
    Const_iterator end() const {
        return {tail};
    }

    int size() const {
        return _size;
    }
    bool empty() const {
        return _size == 0;
    }
    void clear(){
        while(!empty()) pop_front();
    }
    T& front(){
        return *begin();
    }
    const T& front() const {
        return *begin();
    }
    T& back(){
        return *--end();
    }
    const T& back() const {
        return *--end();
    }
    void push_back(const T& x){
        insert(end(), x);
    }
    void push_back(T&& x){
        insert(end(), std::move(x));
    }
    void push_front(const T& x){
        insert(begin(), x);
    }
    void push_front(T&& x){
        insert(begin(), std::move(x));
    }
    void pop_front(){
        erase(begin());
    }
    void pop_back(){
        erase(--end());
    }
    Iterator insert(Iterator itr, const T& x){
        Node *p = itr.current;
        ++_size;
        return {p->prev = p->prev->next = new Node{x, p->prev, p}};
    }

    void insert2(Iterator itr, const T& x){
        auto p = itr.current;
        ++_size;
        
    }

    Iterator insert(Iterator itr, T&& x){
        Node *p = itr.current;
        ++_size;
        return {p->prev = p->prev->next = new Node{std::move(x), p->prev, p}};
    }

    Iterator erase(Iterator itr){
        Node *p = itr.current;
        Iterator retVal{p->next};
        p->prev->next = p->next;
        p->next->prev = p->prev;
        --_size;
        return retVal;
    }

    Iterator erase(Iterator from, Iterator to){
        for(auto it = from; it != to; it = erase(it));
        return to;
    }
private:

    int _size;
    Node *head;
    Node *tail;
    //void init();

};





#endif // !_LIST_HPP_