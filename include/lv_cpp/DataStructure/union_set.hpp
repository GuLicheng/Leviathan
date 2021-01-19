#ifndef __UNION_SET_HPP__
#define __UNION_SET_HPP__

class union_set
{
public:
    union_set() 
    {
        for(int i = 0; i < N; ++i) father[i] = i;
    }

    int find(int x)
    {
        return x == father[x] ? x : father[x] = find(father[x]);//路径压缩
    }

    bool check(int x, int y) 
    {
        return find(x) == find(y); 
    }

    void merge(int x, int y) 
    {
        if(!check(x, y)) father[father[x]] = father[y];
    }

    // for extend
    int* data() 
    { 
        return father; 
    }

private:
    constexpr static int N = 10000;
    int father[N];
};

#endif