#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <concepts>
#include <memory>

namespace ADS
{

	template<typename T>
	class FixedQueueIterator;
	template<typename T>
	class ConstFixedQueueIterator;

	namespace Bases
	{

		template<typename TIter, typename T>
		concept i_iterator_ct = std::input_iterator<TIter> && std::is_convertible_v<typename TIter::value_type, T>;

		/*
		class for containing a fixed size queue
		if the size of the queue reaches the fixed size

		it is implemented as a cyclic buffer to prevent the whole queue being copied on every push / pop.
		meaning the "front" of the queue will not always be the front of the vector.

		(SIZE = 4)
		 FRONT    BACK
		 |        |
		[1, 2, 3, 4] push 5
				   |
				   |
		 B	F      |
		 |	|      v
		[5, 2, 3, 4]

		NO ELEMENTS ARE COPIED EXCEPT FOR THE PUSH ARGUMENT
		*/
		template<typename T>
		class FixedQueueBase
		{
		public:
			FixedQueueBase(T* data, size_t size)
				: m_data(data), m_fixed_size(size) {}

			T& front() { return m_data[m_front_index]; }
			T front() const { return m_data[m_front_index]; }

			void push_back(T elem);
			template<i_iterator_ct<T> TIter>
			void push_back(TIter begin, TIter end);
			template<typename TOther> requires std::is_convertible_v<TOther, T>
			void push_back(std::initializer_list<TOther> list);
			template<typename TOther> requires std::is_convertible_v<TOther, T>
			void push_back(FixedQueueBase<TOther>& other);
			template<typename TOther> requires std::is_convertible_v<TOther, T>
			void push_back(const FixedQueueBase<TOther>& other);
			template<typename TOther>
			void push_back(const std::vector<TOther>& vec) { push_back(vec.begin(), vec.end()); };
			template<typename TOther, size_t n>
			void push_back(const std::array<TOther, n>& arr) { push_back(arr.begin(), arr.end()); };

			inline void push(T elem) { push_back(elem); };
			template<i_iterator_ct<T> TIter>
			inline void push(TIter begin, TIter end) { push_back(begin, end); };
			template<typename TOther>
			inline void push(std::initializer_list<TOther> list) { push_back(list); };
			template<typename TOther>
			inline void push(const std::vector<TOther>& vec) { push_back(vec); };
			template<typename TOther, size_t n>
			inline void push(const std::array<TOther, n>& arr) { push_back(arr); };

			T& back() { return m_data[(m_front_index + m_size) % m_fixed_size]; }
			T back() const { return m_data[(m_front_index + m_size) % m_fixed_size]; }

			void pop_front(size_t elem_count = 1);
			inline void pop(size_t elem_count = 1) { pop_front(elem_count); };

			size_t size() const { return m_fixed_size; };
			size_t length() const { return m_size; };

			bool full() { return length() == size(); }
			bool empty() { return length() == 0; }

			T& operator[](size_t index);
			T operator[](size_t index) const;

			template<typename TCast = T> requires std::is_convertible_v<T, TCast>
			std::vector<TCast> toVector() const;
			template<typename TCast = T> requires std::is_convertible_v<T, TCast>
			std::unique_ptr<TCast> toCarr() const;

			// returns the avrage of all the elements in the queue
			//
			// TAvg = the data type of the variable that stores the avrage.
			// should be large enough to contain the sum of all the elements.
			// if no such datatype is present, use avgHuge instead.
			template<typename TAvg = T> requires requires(T x) { x + x / x; }
			T avg();

			// returns the avrage of all the elements in the queue using a less precise method, but has no size cap.
			//
			// TAvg = the data type that stores the avrage.
			// a floating point data type is recommended since this method relies on dividing each element before adding them to the result
			template<typename TAvg = float> requires requires(T x) { x + x / x; }
			T avgHuge();

			// returns the maximum value inside the queue from offset.
			T max(size_t offset = 0);
			// returns the index of the minimum value.
			// if there are more than one instance of the minimum value, the first minimum value from offset found will be returned.
			size_t iOfMax(size_t offset = 0);

			// returns the minimum value inside the queue from offset.
			T min(size_t offset = 0);
			// returns the index of the minimum value.
			// if there are more than one instance of the maximum value, the first maximum value from offset found will be returned.
			size_t iOfMin(size_t offset = 0);


			// sets size to 0 and puts front_index at the start of the m_data array
			void clear();

			FixedQueueIterator<T> begin();
			ConstFixedQueueIterator<T> begin() const;

			FixedQueueIterator<T> end();
			ConstFixedQueueIterator<T> end() const;

			// pushes the element to the que
			void operator<<(T elem) { push_back(elem); }
			template<typename TOther>
			void operator<<(const std::vector<TOther>& vec) { push_back(vec); }
			template<typename TOther, size_t n>
			void operator<<(const std::array<TOther, n>& arr) { push_back(arr); }
			template<typename TOther> requires std::is_convertible_v<TOther, T>
			void operator<<(FixedQueueBase<TOther>& other) { push_back(other); }
			template<typename TOther> requires std::is_convertible_v<TOther, T>
			void operator<<(const FixedQueueBase<TOther>& other) { push_back(other); }


		protected:
			size_t projectIndex(size_t index) const;

			size_t m_fixed_size;
			size_t m_size = 0;

			size_t m_front_index = 0;

			T* m_data = nullptr;
		};
	}

	
	template<typename T>
	class FixedQueue: public Bases::FixedQueueBase<T>
	{
	public:
		FixedQueue(size_t size = 0);

		void resize(size_t new_size);

	protected:

		using Bases::FixedQueueBase<T>::m_data;
		using Bases::FixedQueueBase<T>::m_fixed_size;
		using Bases::FixedQueueBase<T>::m_size;
		using Bases::FixedQueueBase<T>::m_front_index;
	};

	// a static version of FixedQueue
	template<typename T, size_t n>
	class SFixedQueue: public Bases::FixedQueueBase<T>
	{
	public:
		SFixedQueue()
			: Bases::FixedQueueBase<T>(m_data, n) {};

	protected:

		T m_data[n];
	};

	// Iterator for the FixedQueue class
	template<typename T>
	class FixedQueueIterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::random_access_iterator_tag;

		FixedQueueIterator(pointer ptr, const pointer q_front, const pointer q_back)
			: m_ptr(ptr), m_q_front(q_front), m_q_back(q_back) {}

		FixedQueueIterator& operator++();
		FixedQueueIterator operator++(int);

		FixedQueueIterator operator+(size_t offset);
		FixedQueueIterator& operator+=(size_t offset);

		FixedQueueIterator operator-(size_t offset);
		FixedQueueIterator& operator-=(size_t offset);

		FixedQueueIterator& operator--();
		FixedQueueIterator operator--(int);

		bool operator==(const FixedQueueIterator<T>& other) const;
		bool operator!=(const FixedQueueIterator<T>& other) const;

		reference operator*();

	protected:
		pointer m_ptr;
		// stores the front and back of the m_data array, not the queue itself
		const pointer m_q_front;
		const pointer m_q_back;
		bool past_self = false;
	};

	// Const Iterator for the Fixed Queue class
	template<typename T>
	class ConstFixedQueueIterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = const T;
		using pointer = const T*;
		using reference = const T&;
		using iterator_category = std::random_access_iterator_tag;

		ConstFixedQueueIterator(pointer ptr, const pointer q_front, const pointer q_back)
			: m_ptr(ptr), m_q_front(q_front), m_q_back(q_back) {}

		ConstFixedQueueIterator& operator++();
		ConstFixedQueueIterator operator++(int);

		ConstFixedQueueIterator operator+(size_t offset);
		ConstFixedQueueIterator& operator+=(size_t offset);

		ConstFixedQueueIterator operator-(size_t offset);
		ConstFixedQueueIterator& operator-=(size_t offset);

		ConstFixedQueueIterator& operator--();
		ConstFixedQueueIterator operator--(int);

		bool operator==(const ConstFixedQueueIterator<T>& other) const;
		bool operator!=(const ConstFixedQueueIterator<T>& other) const;

		reference operator*() const;

	protected:
		pointer m_ptr;
		// stores the front and back of the m_data array, not the queue itself
		const pointer m_q_front;
		const pointer m_q_back;
		bool past_self = false;
	};
}

// stores the front into target and pops the que
template<typename T, typename TVar> requires (!std::is_same_v<std::ostream, TVar>)
void operator<<(TVar& target, ADS::Bases::FixedQueueBase<T>& que);


template<typename T, typename TVec>
void operator<<(std::vector<TVec>& target, ADS::Bases::FixedQueueBase<T>& que);

template<typename T, typename TVec, size_t n>
void operator<<(std::array<TVec, n>& target, ADS::Bases::FixedQueueBase<T>& que);

template<typename T>
std::ostream& operator<<(std::ostream& stream, const ADS::Bases::FixedQueueBase<T>& queue);

#include "FixedQueue.ipp"


