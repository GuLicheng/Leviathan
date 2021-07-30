#include <bits/stdc++.h>
using namespace std;

static const int P = RAND_MAX / 4, MAX_LEVEL = 16;
class Skiplist {
public:
    struct Node {
        int val;
        vector<Node*> nexts;
        Node(int val=-1, int size = MAX_LEVEL) : val(val), nexts(size) {}
    };

    Node* head;
    int level = 0;

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
    
    void add(int num) {
        int rLevel = randomLevel();
        level = max(level, rLevel);
        Node* cur = new Node(num, rLevel);
        Node* node = head;
        for (int i = level - 1; i >= 0; --i) {
            while (node->nexts[i] && num>node->nexts[i]->val) {
                node = node->nexts[i];
            }
            if (i<rLevel) {
                Node* next = node->nexts[i];
                node->nexts[i] = cur;
                cur->nexts[i] = next;
            }
        }
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


