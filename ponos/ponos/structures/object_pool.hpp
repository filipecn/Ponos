#ifndef PONOS_STRUCTURES_OBJECT_POOL_H
#define PONOS_STRUCTURES_OBJECT_POOL_H

#include "common/defs.h"

#include <vector>

namespace ponos {

	template<class T>
		struct FreeListNode {
			T* next;
		};

	template<class T, uint S = 0>
		class ObjectPool {
			public:
				ObjectPool();

				T* create();
				void destroy(T* object);

			protected:
				T* firstFree;
				bool limitSize;
				std::vector<T> pool;
		};

	template<class T, uint S>
		ObjectPool<T, S>::ObjectPool() {
			limitSize = S;
			firstFree = nullptr;
			if(limitSize) {
				pool.resize(S);
				firstFree = &pool[0];
				FreeListNode<T> *fl;
				for(uint i = 0; i < S - 1; i++) {
					fl = reinterpret_cast<FreeListNode<T> *>(&pool[i]);
					fl->next = &pool[i + 1];
				}
				fl = reinterpret_cast<FreeListNode<T> *>(&pool[S - 1]);
				fl->next = nullptr;
			}
		}

	template<class T, uint S>
		T* ObjectPool<T, S>::create() {
			if(firstFree != nullptr) {
				T* first = firstFree;
				firstFree = reinterpret_cast<FreeListNode<T> *>(firstFree)->next;
				return first;
			}
			if(limitSize)
				return nullptr;
			pool.emplace_back();
			return &pool[pool.size() - 1];
		}

	template<class T, uint S>
		void ObjectPool<T, S>::destroy(T* object) {
			if(object) {
				FreeListNode<T> *fl = reinterpret_cast<FreeListNode<T> *>(object);
				fl->next = firstFree;
				firstFree = object;
			}
		}

} // ponos namespace

#endif
