#include "base.hpp"


template <typename T, typename E>
struct expected
{
private:
    union 
    {
        T val;
        E err;
    };

    bool valid;
};


int main(int argc, char const *argv[])
{
    
    return 0;
}

