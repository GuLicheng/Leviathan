/*
    https://pintia.cn/problem-sets/994805342720868352/problems/994805354108403712
*/

#ifndef __UNION_SET_HPP__
#define __UNION_SET_HPP__

#include <iostream>

// for x = father[node], node is the number of this cluster if x < 0, otherwise root of node
// index start with 0
class union_set
{
public:
    union_set(int max_size) : father{new int[max_size]}, size{max_size} 
    {
        for (int i = 0; i < max_size; ++i) father[i] = -1;
    }

    int find(int x) noexcept
    {
        return father[x] < 0 ? x : father[x] = find(father[x]);//路径压缩
    }

    bool check(int x, int y) 
    {
        return find(x) == find(y); 
    }

    bool merge(int x, int y) 
    {
        int fx = find(x);
        int fy = find(y);
        if (fx == fy)
            return false;
        // make smaller root
        else if (fx < fy)
        {
            father[fx] += father[fy];
            father[fy] = fx;
        }
        else
        {
            father[fy] += father[fx];
            father[fx] = fy;
        }
        return true;
    }

    int count(int l = 0) const noexcept
    {
        int res = 0;
        for (int i = l; i < size; ++i)
            if (father[i] < 0)
                res ++;
        return res;
    }

    // for extend
    int* data() noexcept
    { 
        return father; 
    }

    ~union_set() 
    {
        delete[] father;
    }

    friend std::ostream& operator<<(std::ostream& os, const union_set& s1)
    {
        os << '[';
        for (int i = 0; i < s1.size; ++i)
        {
            if (i) std::cout << ',' << ' ';
            std::cout << s1.father[i];
        }
        return os << ']';
    }

private:
    int size;
    int* father;
};

#endif