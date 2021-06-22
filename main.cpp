#include <iostream>
#include <vector>
#include <utility>
/*
  Abdul Wahab Shafiq (S011313) Department	of	Computer Science

    *Tiny Ranges Implementation by Abdul Wahab
    * as part of CS409/509 - Advanced C++ Programming course in Ozyegin University
    * Supports transforming and filtering ranges and,
    * to<CONTAINER>() method for eagerly rendering the range into a CONTAINER.*/


// predicates
namespace predicates
{
    auto less_than = [](auto threshold) { return [=](auto value) { return value < threshold; }; };
    auto greater_than = [](auto threshold) { return [=](auto value) { return value > threshold; }; };
    auto all_of = [](auto ... preds) { return [=](auto value) { return (preds(value) && ...); }; };
}

namespace actions
{
    auto multiply_by = [](auto coef) { return [=](auto value) { return value * coef; }; };
    auto if_then = [](auto predicate, auto action) { return [=](auto value) { if(predicate(value)) value = action(value); return value; }; };
}

namespace views
{
    auto ints = [](int k=0) { return [k]() mutable { return k++; }; };
    auto odds = []() { return [k=1]() mutable { auto r = k; k += 2; return r; }; };
}

namespace algorithms
{
    // ---[ RANGE implementation
    template<typename Iterator>
    struct Range
    {
        using LazyIterator = Iterator; // required for accessing the used Iterator type from other locations
        Iterator m_begin;
        Iterator m_end;
        auto begin() { return m_begin; }
        auto end() { return m_end; }
    };

    template<typename Iterator>
    Range(Iterator, Iterator) -> Range<Iterator>;

    // ---[ TRANSFORM implementation
    template<typename Iterator, typename Callable>
    struct TransformingIterator : Iterator
    {
        using OriginalIterator = Iterator; // required for accessing the used Iterator type from other locations
        Callable callable;
        TransformingIterator(Iterator iterator, Callable callable) : Iterator{iterator}, callable{callable} { }
        Iterator& get_orig_iter() { return ((Iterator&)*this); }
        double operator*() { return callable(*get_orig_iter()); }
    };

    auto transform = [](auto action) {
        return [=](auto&& container) {
            using Container = std::decay_t<decltype(container)>;
            using Iterator = typename Container::iterator;
            using IteratorX = TransformingIterator<Iterator, decltype(action)>;
            return Range{IteratorX{container.begin(), action}, IteratorX{container.end(), action}};
        };
    };

    // ---[ FILTER implementation: implement your "Filtering Iterator" here
    template<typename Iterator, typename Callable>
    struct FilteringIterator : Iterator {
        using OriginalIterator = Iterator; // required for accessing the used Iterator type from other locations
        Callable callable;
        using OriginalIterator = Iterator;
        FilteringIterator(const Iterator begin,  Callable callable):Iterator(begin),callable(callable){} // constructor
        Iterator &get_orig_iter() { return static_cast<Iterator&>(*this); }
        auto operator++(){ return get_orig_iter()++;}  //  increment iterator
        bool operator==(const FilteringIterator& rhs) const { return get_orig_iter() == rhs.get_orig_iter(); }
        bool operator!=( FilteringIterator& end) {
            Iterator& this_iterator = get_orig_iter(); // current Iterator
            const auto& end_iterator = end.get_orig_iter(); // last Iterator
            while (this_iterator !=end_iterator && !callable(*get_orig_iter()))
                this_iterator++;
            if (get_orig_iter() == end) return false;  // to ensure begin() doesnt go pass end() and break for loop
            else return true; }
        auto operator*() { return *get_orig_iter();}  // returns value
    };

    auto filter = [](auto action) {
        return [=]( auto&& container) {
            using Container = std::decay_t<decltype(container)>;
            using Iterator = typename Container::iterator;
            using actiontype = decltype(action); //
            using filter_iterator = FilteringIterator<Iterator, actiontype>;
            return Range{filter_iterator{container.begin(),action}, filter_iterator{container.end(),action}};

        };
    };
    // ---[ TO implementation: implement your "render into a container" method here
    template<template <typename ...> typename stdvector>
    auto to = []() {
        return []( auto&& container) {
            using Container = std::decay_t<decltype(container)>; // to get the type of input container
            using IteratorType = typename Container::LazyIterator::value_type; //to get the type of original Iterator
            auto renderedContainer = stdvector<IteratorType>{};
            for (auto i : container) {
                renderedContainer.push_back(i); // push back elements of range to a new vector
            }
            return renderedContainer;
        };
    };
}

template<typename CONTAINER, typename RANGE>
auto operator |( CONTAINER&& container, RANGE&& range) { return range(std::forward<CONTAINER>(container)); }

using namespace predicates;
using namespace actions;
using namespace algorithms;

int main(int argc, char* argv[])
{
    auto new_line = [] { std::cout << std::endl; };

    auto v = std::vector<double>{};
    auto odd_gen = views::odds();
    for(int i=0; i<5; ++i)
        v.push_back(odd_gen() * 2.5);
    // v contains {2.5, 7.5, 12.5, 17.5, 22.5} here

    auto range = v | transform(if_then(all_of(greater_than(2.0), less_than(15.0)), multiply_by(20)));
    for(auto a : range) // transformation is applied on the range as the for loop progresses
        std::cout << a << std::endl;
    // original v is not changed. prints {50.0, 150.0, 250.0, 17.5, 22.5}

    new_line();
    for(auto a : v | filter(greater_than(15))) // filter is applied lazily as the range is traversed
        std::cout << a << std::endl;
    // prints 17.5 and 22.5 to the console

    new_line();
    auto u = std::vector<int>{10, 20, 30};
    auto u_transformed = u | transform(multiply_by(2)) | to<std::vector>();
    for(auto a : u_transformed) // u_transformed is an std::vector<int> automatically because of to<std::vector>
        std::cout << a << std::endl;
//  prints 20, 40 and 60

    new_line();

    auto  q3 = std::vector<int>{30, 20, 60} | transform(multiply_by(2)) | to<std::vector>();
    for(auto a : q3 )
        std::cout << a << std::endl;
    //  prints 60, 40 and 120

    return 0;
}
