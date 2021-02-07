#pragma once

#include <iostream>



struct foo
{
private:
    inline static int default_constructor = 0;
    inline static int copy_constructor = 0;
    inline static int copy_assignment = 0;
    inline static int move_constructor = 0;
    inline static int move_assignment = 0;
    inline static int destructor = 0;
public:

    int val;

    foo() : val{0} 
    { std::cout << "default_constructor :" << ++default_constructor << std::endl;}
    
    foo(const foo& rhs) : val{rhs.val}
    { std::cout << "copy_constructor :" << ++copy_constructor << std::endl;}

    foo(foo&& rhs) noexcept : val{rhs.val}
    { std::cout << "move_constructor :" << ++move_constructor << std::endl;}

    foo& operator=(const foo& rhs) 
    {
        this->val = rhs.val; 
        std::cout << "copy_assignment :" << ++copy_assignment << std::endl;
        return *this;
    }

    foo& operator=(foo&& rhs) noexcept
    { 
        this->val = rhs.val;
        std::cout << "move_assignment :" << ++move_assignment << std::endl;
        return *this;
    }

    ~foo() 
    { std::cout << "destructor :" << ++destructor << std::endl;}

    friend std::ostream& operator<<(std::ostream& os, const foo& f)
    {
        return os << f.val;
    }

};

struct empty_class { };
