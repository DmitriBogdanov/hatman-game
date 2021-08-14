#pragma once

#include <unordered_map> /// ? change to set ?
#include <memory> // 'unique_ptr' type
#include <utility> // 'std::forward' type (forwarding argument packet to 'std::make_unique()')
#include <initializer_list> /// TEMP



// # _map_ptrkey_iterator #
// - NOT INTENDED FOR EXTERNAL USE!
// - Allows convenient iteration over map keys
// - Intended to be used for map with pointer ::key_type
template<typename Iter>
class _map_ptrkey_iterator : public Iter
{
public:
	_map_ptrkey_iterator() : Iter() {}
	_map_ptrkey_iterator(Iter iter) : Iter(iter) {}
	auto* operator->()
	{
		return Iter::operator->()->first;
	}
	auto& operator*()
	{
		return *(Iter::operator*().first);
	}
};



// # Collection<> #
// - Polymorphic container
// - Can be iterated through
// - Provides a robust 'handle' when inserting new elements
// - Allows erasion of elements through handles and iteration
template<class Object>
class Collection {
	using map_t = std::unordered_map< Object*, std::unique_ptr<Object> >;
public:


	// # Collection<>::Handle #
	// - Allows access and const-access to the element
	// - Allows safe erasion of the element
	// - Get invalidated if parent collection is deleted
	class handle {
	public:
		handle() : collection(nullptr), node_key(nullptr) {}

		handle(Collection<Object>* collection, Object* node) :
			collection(collection),
			node_key(node)
		{}

		Object& get() {
			return *this->node_key;
		}
		const Object& cget() {
			return *this->node_key;
		}

		bool erase() { // returns if node was erased, safe to call on already erased elements
			if (this->collection && this->node_key) {
				this->collection->storage.erase(this->node_key);
				this->node_key = nullptr;
				return true;
			}
			return false;
		}

	private:
		Collection<Object>* collection;
		Object* node_key; // nullptr when handle is invalid
	};


	// Iteration
	using iterator = _map_ptrkey_iterator<typename map_t::iterator>;
	using const_iterator = _map_ptrkey_iterator<typename map_t::const_iterator>;

	iterator begin() { return this->storage.begin(); }
	iterator end() { return this->storage.end(); }
	const_iterator begin() const { return this->storage.begin(); }
	const_iterator end() const { return this->storage.end(); }
	const_iterator cbegin() const { return this->storage.cbegin(); }
	const_iterator cend() const { return this->storage.cend(); }

	// Methods
	template<class... Args>
	handle insert(Args&&... args) {
		std::unique_ptr<Object> node_value = std::make_unique<Object>(std::forward<Args>(args)...);
		Object* node_key = node_value.get();

		this->storage[node_key] = std::move(node_value);

		return handle(this, node_key);
	}

	template<class DerivedObject>
	handle insert(std::unique_ptr<DerivedObject> &&node_value) {
		Object* node_key = node_value.get();

		this->storage[node_key] = std::move(node_value);

		return handle(this, node_key);
	}

	size_t erase(iterator iter) {
		return this->storage.erase(&(*iter)); // probably not the best way to do it
	}

private:
	map_t storage;
};



/// MOVE TO ANOTHER HEADER
// # make_init_list() #
// - A clutch function used to create initializer list with deducable type
// - Mostly used in 'std::make_unique()' with 'std::initializer_list' parameters
template<class T>
std::initializer_list<T> make_init_list(std::initializer_list<T> &&list) {
	return list;
}