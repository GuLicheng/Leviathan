#include <vector>
#include <iostream>

enum ObjectType
{
    OBJ_INT,
    OBJ_PAIR,
} ObjectType;

template <typename T>
class GCObject
{
    bool marked = false;

public:

    inline static std::vector<T*> pool;

    inline static std::vector<T*> stack; // interpreter


    void mark() 
    { 
        if (is_marked())
            return;
        std::cout << "marked object\n";
        marked = true; 
    }

    void unmark() { marked = false; }

    bool is_marked() { return marked; }

    static void mark_all()
    {
        T::mark_all_object();
    }

    static void sweep()
    {
        std::vector<T*> new_pool;

        for (auto obj : pool)
        {
            if (obj->is_marked())
            {
                new_pool.emplace_back(obj);
                obj->unmark();
            }
            else
            {
                if (obj->type == OBJ_INT)
                {
                    std::cout << "remove one int object" << obj->value << "\n";
                }
                else
                {
                    std::cout << "remove list\n";
                }
                delete obj;
            }
        }

        pool = std::move(new_pool);
    }

    template <typename... Args>
    static T* alloc(Args&&... args)
    {
        T* obj = new T((Args&&) args...);
        pool.emplace_back(obj);
        stack.emplace_back(obj);
        return obj;
    }

    static void gc()
    {
        mark_all();
        sweep();
    }

};



struct Object : GCObject<Object>
{
    enum ObjectType type;

    Object(int value) : type(OBJ_INT), value(value) { }

    Object(Object* head, Object* tail) : type(OBJ_PAIR), head(head), tail(tail) { }

    union 
    {
        int value; // OBJ_INT

        struct // OBJ_PAIR
        {
            Object* head;
            Object* tail;
        };
    };

    static void mark_all_object()
    {
        for (auto obj : stack)
        {
            switch (obj->type)
            {
            case OBJ_INT:
            {
                obj->mark();
                break;
            }
            case OBJ_PAIR:
            {
                obj->head->mark();
                obj->tail->mark();
            }
            default: break;
            }
        }
    }

    static void print_stack()
    {
        for (auto obj : stack)
        {
            if (obj->type == OBJ_INT)
            {
                std::cout << "INT ";
            }
            else
            {
                std::cout << "PAIR ";
            }
        }
    }

    static void print_pool()
    {
        for (auto obj : pool)
        {
            if (obj->type == OBJ_INT)
            {
                std::cout << "INT ";
            }
            else
            {
                std::cout << "PAIR ";
            }
        }
    }

};

int main()
{
    auto int1 = Object::alloc(1);
    auto int2 = Object::alloc(2);
    auto int3 = Object::alloc(3);
    auto int4 = Object::alloc(4);

    auto pair1 = Object::alloc(int1, int2);
    auto pair2 = Object::alloc(int3, int4);

    pair1->tail = pair2;
    pair2->head = pair1;

    Object::stack.erase(Object::stack.begin(), Object::stack.begin() + 4);

    Object::print_stack();
    std::endl(std::cout);
    Object::print_pool();
    std::endl(std::cout);

    Object::gc();
}

