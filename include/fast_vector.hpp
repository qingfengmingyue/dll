//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DBN_FAST_VECTOR_HPP
#define DBN_FAST_VECTOR_HPP

#include "assert.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <array>

template <typename T, typename LeftExpr, typename BinaryOp, typename RightExpr>
class fast_vector_expr;

template<typename T>
struct plus_binary_op;

template<typename T>
struct minus_binary_op;

template<typename T>
struct mul_binary_op;

template<typename T>
struct div_binary_op;

template<typename T>
struct scalar;

template<typename T, std::size_t Rows>
class fast_vector {
    static_assert(Rows > 0, "Vector of size 0 do no make sense");

private:
    std::array<T, Rows> _data;

public:
    static constexpr const std::size_t rows = Rows;

    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;

    fast_vector(){
        //Nothing else to init
    }

    fast_vector(const T& value){
        std::fill(_data.begin(), _data.end(), value);
    }

    template<typename LE, typename Op, typename RE>
    fast_vector(fast_vector_expr<T, LE, Op, RE>&& e){
        for(std::size_t i = 0; i < Rows; ++i){
            _data[i] = e[i];
        }
    }

    template<typename LE, typename Op, typename RE>
    fast_vector& operator=(fast_vector_expr<T, LE, Op, RE>&& e){
        for(std::size_t i = 0; i < Rows; ++i){
            _data[i] = e[i];
        }

        return *this;
    }

    //Prohibit copy
    fast_vector(const fast_vector& rhs) = delete;
    fast_vector& operator=(const fast_vector& rhs) = delete;

    //Allow move
    fast_vector(fast_vector&& rhs) = default;
    fast_vector& operator=(fast_vector&& rhs) = default;

    //Modifiers

    //Set every element to the same scalar
    void operator=(const T& value){
        std::fill(_data.begin(), _data.end(), value);
    }

    //Divide each element by a scalar
    fast_vector& operator/=(const T& value){
        for(size_t i = 0; i < Rows; ++i){
            _data[i] /= value;
        }

        return *this;
    }

    template<typename RE>
    fast_vector& operator+=(RE&& rhs){
        for(size_t i = 0; i < Rows; ++i){
            _data[i] += rhs[i];
        }

        return *this;
    }

    //Add elements of vector together
    template<typename RE>
    auto operator+(RE&& re) const ->
    fast_vector_expr<T, const fast_vector&, plus_binary_op<T>, decltype(std::forward<RE>(re))> {
        return {*this, std::forward<RE>(re)};
    }

    //Sub elements of vector together
    template<typename RE>
    auto operator-(RE&& re) const ->
    fast_vector_expr<T, const fast_vector&, minus_binary_op<T>, decltype(std::forward<RE>(re))> {
        return {*this, std::forward<RE>(re)};
    }

    //Mul each element by a scalar
    auto operator*(T re) const ->
    fast_vector_expr<T, const fast_vector&, mul_binary_op<T>, scalar<T>> {
        return {*this, re};
    }

    //Mul elements of vector togethers
    template<typename RE>
    auto operator*(RE&& re) const ->
    fast_vector_expr<T, const fast_vector&, mul_binary_op<T>, decltype(std::forward<RE>(re))> {
        return {*this, std::forward<RE>(re)};
    }

    //Div each element by a scalar
    auto operator/(T re) const ->
    fast_vector_expr<T, const fast_vector&, div_binary_op<T>, scalar<T>> {
        return {*this, re};
    }

    //Div elements of vector togethers
    template<typename RE>
    auto operator/(RE&& re) const ->
    fast_vector_expr<T, const fast_vector&, div_binary_op<T>, decltype(std::forward<RE>(re))> {
        return {*this, std::forward<RE>(re)};
    }

    //Accessors

    constexpr size_t size() const {
        return rows;
    }

    T& operator()(size_t i){
        dbn_assert(i < rows, "Out of bounds");

        return _data[i];
    }

    const T& operator()(size_t i) const {
        dbn_assert(i < rows, "Out of bounds");

        return _data[i];
    }

    T& operator[](size_t i){
        dbn_assert(i < rows, "Out of bounds");

        return _data[i];
    }

    const T& operator[](size_t i) const {
        dbn_assert(i < rows, "Out of bounds");

        return _data[i];
    }

    const T* data() const {
        return _data;
    }

    const_iterator begin() const {
        return _data.begin();
    }

    iterator begin(){
        return _data.begin();
    }

    const_iterator end() const {
        return _data.end();
    }

    iterator end(){
        return _data.end();
    }
};

template <typename T, typename LeftExpr, typename BinaryOp, typename RightExpr>
class fast_vector_expr {
private:
    LeftExpr _lhs;
    RightExpr _rhs;

    typedef fast_vector_expr<T, LeftExpr, BinaryOp, RightExpr> this_type;

public:
    fast_vector_expr() = delete;

    fast_vector_expr(LeftExpr l, RightExpr r) :
            _lhs(std::forward<LeftExpr>(l)), _rhs(std::forward<RightExpr>(r)){
        //Nothing else to init
    }

    //No copying
    fast_vector_expr(const fast_vector_expr&) = delete;
    fast_vector_expr& operator=(const fast_vector_expr&) = delete;

    //Make sure move is supported
    fast_vector_expr(fast_vector_expr&&) = default;
    fast_vector_expr& operator=(fast_vector_expr&&) = default;

    //Accessors

    typename std::add_lvalue_reference<LeftExpr>::type lhs(){
        return _lhs;
    }

    typename std::add_lvalue_reference<typename std::add_const<LeftExpr>::type>::type lhs() const {
        return _lhs;
    }

    typename std::add_lvalue_reference<RightExpr>::type rhs(){
        return _rhs;
    }

    typename std::add_lvalue_reference<typename std::add_const<RightExpr>::type>::type rhs() const {
        return _rhs;
    }

    //Create more complex expressions

    template<typename RE>
    auto operator+(RE&& re) const -> fast_vector_expr<T, this_type const&, plus_binary_op<T>, decltype(std::forward<RE>(re))>{
        return {*this, std::forward<RE>(re)};
    }

    template<typename RE>
    auto operator-(RE&& re) const -> fast_vector_expr<T, this_type const&, minus_binary_op<T>, decltype(std::forward<RE>(re))>{
        return {*this, std::forward<RE>(re)};
    }

    auto operator*(T re) const -> fast_vector_expr<T, this_type const&, mul_binary_op<T>, scalar<T>> {
        return {*this, re};
    }

    template<typename RE>
    auto operator*(RE&& re) const -> fast_vector_expr<T, this_type const&, mul_binary_op<T>, decltype(std::forward<RE>(re))>{
        return {*this, std::forward<RE>(re)};
    }

    auto operator/(T re) const -> fast_vector_expr<T, this_type const&, div_binary_op<T>, scalar<T>> {
        return {*this, re};
    }

    template<typename RE>
    auto operator/(RE&& re) const -> fast_vector_expr<T, this_type const&, div_binary_op<T>, decltype(std::forward<RE>(re))>{
        return {*this, std::forward<RE>(re)};
    }

    //Apply the expression

    auto operator[](std::size_t i) const -> decltype(BinaryOp::apply(this->lhs()[i], this->rhs()[i])) {
        return BinaryOp::apply(lhs()[i], rhs()[i]);
    }
};

template<typename T>
struct scalar {
    T value;
    scalar(T v) : value(v) {}

    T operator[](std::size_t) const {
        return value;
    }
};

template<typename T>
struct plus_binary_op {
    static T apply(const T& lhs, const T& rhs){
        return lhs + rhs;
    }
};

template<typename T>
struct minus_binary_op {
    static T apply(const T& lhs, const T& rhs){
        return lhs - rhs;
    }
};

template<typename T>
struct mul_binary_op {
    static T apply(const T& lhs, const T& rhs){
        return lhs * rhs;
    }
};

template<typename T>
struct div_binary_op {
    static T apply(const T& lhs, const T& rhs){
        return lhs / rhs;
    }
};


#endif
