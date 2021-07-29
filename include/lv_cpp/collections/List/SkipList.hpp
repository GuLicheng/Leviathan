#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <random>


template<typename T>
struct SkipNode {
	T elem;
	std::vector<SkipNode *> next;		 // 后继

	SkipNode() = default;
	SkipNode(const T &x, std::size_t nextNum)
		:elem(x), next(nextNum, nullptr) { }
	SkipNode(T &&x, std::size_t nextNum)
		:elem(std::move(x)), next(nextNum, nullptr) { }

};

template<typename T, typename Cmp = std::less<T>>
class SkipList {

	using SkipNode = SkipNode<T>;
	
	constexpr static int MAXLEVEL = 32;

public:

	void show() const {
		auto cur = head->next[0];
		while (cur) {
			std::cout << cur->elem << "("<< cur->next.size() << ")  ";
			cur = cur->next[0];
		}
	}

	void insert(const T &x) {
		insert(head, x);
	}

	bool find(const T &x) {
		auto node = find(head, x);
		return node == head ? false : node->elem == x;
	}

	void remove(const T &x) {
		erase(head, x);
	}

	SkipList() :head{ new SkipNode(T{}, MAXLEVEL)}, cmp{}, curSize{ 0 }{ }

	auto size() const { return curSize; }
	bool empty() const { return curSize == 0; }
	

private:

	SkipNode*    head;
	Cmp          cmp;
	std::size_t  curSize;

	int getLevel() const {
		int level = 1;
		constexpr double p = 0.25;
		constexpr double rand_max = std::random_device::max();
		static std::random_device rd;
		for (; ((rd() / rand_max) < p && level < MAXLEVEL); ++level);
		return std::min(MAXLEVEL, level);
	}

	SkipNode *find(SkipNode *p, const T &x) const {
		//std::vector<SkipNode *> prevNode(MAXLEVEL, nullptr);
		SkipNode *cur = nullptr;
		for (int i = p->next.size() - 1; i >= 0; --i) {
			cur = p;
			for (; cur->next[i] != nullptr && cmp(cur->next[i]->elem, x); cur = cur->next[i]);

			if (cur->next[i] != nullptr && cur->next[i]->elem == x)
				//找到该节点
				return cur->next[i];
		}
		// !upper_bound ;if cur == head then fail
		return cur;
	}

	SkipNode *insert(SkipNode *p, const T &x) {
		std::vector<SkipNode *> prevNode(MAXLEVEL, nullptr);
		
		for (int i = p->next.size() - 1; i >= 0; --i) {
			SkipNode *cur = p;
			for (; cur->next[i] != nullptr && cmp(cur->next[i]->elem, x); cur = cur->next[i]);

			if (cur->next[i] != nullptr && cur->next[i]->elem == x)
				//找到该节点, 什么都不做
				return cur;
			prevNode[i] = cur;
		}

		auto newNodeLevel = this->getLevel();
		//更新前驱后继
		SkipNode *newNode = new SkipNode(x, newNodeLevel);
		for (int i = newNode->next.size() - 1; i >= 0; --i) {
			newNode->next[i] = prevNode[i]->next[i];
			prevNode[i]->next[i] = newNode;
		}
		++curSize;
		return newNode;
	}



	//返回被删除节点的下一个节点
	SkipNode *erase(SkipNode *p, const T &x) {
		std::vector<SkipNode *> prevNode(MAXLEVEL, nullptr);
		SkipNode *cur = p;
		bool exist = false;
		for (int i = p->next.size() - 1; i >= 0; --i) {
			for (; cur->next[i] != nullptr && cmp(cur->next[i]->elem, x); cur = cur->next[i]);

			if (cur->next[i] != nullptr && cur->next[i]->elem == x)
				//找到该节点, 什么都不做
				exist = true;
			prevNode[i] = cur;
		}

		if (!exist) 
			return cur->next[0];
		
		--curSize;
		SkipNode *deletedNode = cur->next[0];
		for (int i = 0; i < deletedNode->next.size(); ++i) 
			prevNode[i]->next[i] = deletedNode->next[i];
		
		delete deletedNode;
		return prevNode[0]->next[0];
	}



};




#endif // !SKIPLIST_H
