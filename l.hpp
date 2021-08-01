#include <bits/stdc++.h>
using namespace std;

static const int P = RAND_MAX / 4, MAX_LEVEL = 32;
class Skiplist {
public:
    struct Node {
        int val;
        vector<Node*> nexts;
        Node(int val=-1, int size = MAX_LEVEL) : val(val), nexts(size) {}
    };

    Node* head;
    int level = 1;
    std::size_t size = 0;

    Skiplist() {
        head = new Node();
    }
    
    bool search(int num) {
        Node* node = head;
        for (int i = level - 1; i >= 0; --i) {
            while (node->nexts[i] && num>node->nexts[i]->val) {
                node = node->nexts[i];
            }
            if (node->nexts[i] && num==node->nexts[i]->val) {
                return true;
            }
        }
        return false;
    }
    static int get_level()
    {
        constexpr auto p = std::random_device::max() / 4;
        static std::random_device rd;
        int level = 1;
        for (; rd() < p; ++level);
        return std::min(MAX_LEVEL, level);
    }
    Node* add(int num) {
        int rLevel = get_level();
        std::vector<Node *> prevNode(MAX_LEVEL, nullptr);
        Node* node = head;
        for (int i = level - 1; i >= 0; --i) {
            for (; node->nexts[i] && num>node->nexts[i]->val; node = node->nexts[i]);
            if (node->nexts[i] && node->nexts[i]->val == num) {
                // find it
                return node;
            }
            prevNode[i] = node;
        }
        Node* cur = new Node(num, rLevel);

        for (int i = 0; i < rLevel; ++i)
        {
            if (i >= this->level)
                head->nexts[i] = cur;
            else
            {
                cur->nexts[i] = prevNode[i]->nexts[i];
                prevNode[i]->nexts[i] = cur;
            }
        }
        level = max(level, rLevel);
        this->size ++;
        return cur;
    }
    
    std::size_t Size() const {
        return this->size;
    }

    bool erase(int num) {
        Node* node = head;
        for (int i = level - 1; i >= 0; --i) {
            while (node->nexts[i] && num>node->nexts[i]->val) {
                node = node->nexts[i];
            }
            if (node->nexts[i] && num==node->nexts[i]->val) {
                Node* del = node->nexts[i];
                for(;i>=0;--i){
                    while(node->nexts[i]!=del) node = node->nexts[i];
                    node->nexts[i] = node->nexts[i]->nexts[i];
                    if(!head->nexts[i]) level = i;
                }
                delete del;
                return true;
            }
        }
        return false;
    }

    int randomLevel(){
        int rLevel = 1;
        while (rand() < P && rLevel < MAX_LEVEL) ++rLevel;
        return rLevel;
    }
};


