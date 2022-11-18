#pragma once

#include "lua.h"
#include <type_traits>
#include <tuple>
#include <cassert>
#include "meta/LambdaUtil.h"

namespace luash {
    template<class T> struct type_identity : std::enable_if<true, T> {};

    struct sentence_t {};
    struct expression_t {};
    struct volatile_t : expression_t {};
    struct identity_t : volatile_t {};
    struct local_base_t : expression_t {};
    struct function_arg_base_t : expression_t {};
    struct assign_base_t : sentence_t {};

    namespace detail {
        // so sad that we dont have c++17 fold expression
        template<std::size_t...I> std::true_type conjunction_impl(std::index_sequence<I...>, decltype(I, std::true_type())...);
        std::false_type conjunction_impl(...);
        template<template<class> class UnaryFn, class...T> struct all_of : decltype(conjunction_impl(std::index_sequence_for<T...>(), typename UnaryFn<T>::type()...)) {};
    }

    template<class T> struct is_sentence : std::is_base_of<sentence_t, T> {};
    template<class T> struct is_expression : std::is_base_of<expression_t, T> {};
    template<class...T> struct is_sentences : detail::all_of<is_sentence, T...> {};
    template<class...T> struct is_expressions : detail::all_of<is_expression, T...> {};
    template<class T> struct is_identity : std::is_base_of<identity_t, T> {};
    template<class T> struct is_volatile : std::is_base_of<volatile_t, T> {};
    template<class T> struct is_local : std::is_base_of<local_base_t, T> {};
    template<class...T> struct is_locals : detail::all_of<is_local, T...> {};
    template<class T> struct is_function_arg : std::is_base_of<function_arg_base_t, T> {};
    template<class...T> struct is_function_args : detail::all_of<is_function_arg, T...> {};
    template<class T> struct is_assign_stmt : std::is_base_of<assign_base_t, T> {};
    template<class...T> struct is_assign_stmts : detail::all_of<is_assign_stmt, T...> {};

    struct constant_t : expression_t {};
    template<long long N> struct integer_constant_t : constant_t {};
    template<bool N> struct boolean_constant_t : constant_t {};
    template<char...> struct string_constant_t : constant_t {};
    struct nil_constant_t : constant_t {};

    template <char...C> constexpr string_constant_t<C...> _s = {};
    template<long long N> constexpr integer_constant_t<N> _i = {};
    template<bool B> constexpr boolean_constant_t<B> _b = {};
    constexpr nil_constant_t _nil;
    constexpr boolean_constant_t<true> _true;
    constexpr boolean_constant_t<false> _false;

    template<class T, class K> struct index_result;
    template<class F, class...A> struct call_result;

    struct newtable_t;

    template<class E, class A> struct if_then_end_t;
    template<class E, class A, class B> struct if_then_else_end_t;

    struct return_void_t;
    template<class...E> struct return_value_t;

    struct global_t;
    struct registry_t;
    template<int N> struct upvalue_t;
    template<int N> struct local_t;
    template<class...E> struct sequence_t;
    template<class V, class E> struct assign_t : assign_base_t {};;

    template<class T, class K, class V> using newindex_t = assign_t<index_result<T, K>, V>;

    template<class T, class K> struct index_result;
    template<class F, class...A> struct call_result;
    template<class T, class K, class...A> using member_call_result = call_result<index_result<T, K>, T, A...>;

    namespace incomplete
    {
        struct incomplete_t {};
        struct incomplete_sentence_t : incomplete_t {};

        template<class T, class K> struct member_closure_t : incomplete_t
        {
            constexpr auto operator()() const -> member_call_result<T, K> { return {}; }
            template<class...A2> constexpr auto operator()(A2...) const -> typename std::enable_if<is_expressions<A2...>::value, member_call_result<T, K, A2...>>::type { return {}; }
        };
    }

    template<class D, class B> struct callable_expression_t : B
    {
        // MSVC intellisence dont want empty para pack, so satisfy it.
        constexpr auto operator()() const -> call_result<D> { return {}; }
        template<class...A2> constexpr auto operator()(A2...) const -> typename std::enable_if<is_expressions<A2...>::value, call_result<D, A2...>>::type { return {}; }
    };
    template<class D, class B> struct indexable_expression_t : B
    {
        template<class K2> constexpr auto operator[](K2) const -> typename std::enable_if<is_expression<K2>::value, index_result<D, K2>>::type { return {}; }

        // quick path for member call
        template<class K2> constexpr auto operator->*(K2) const -> typename std::enable_if<is_expression<K2>::value, incomplete::member_closure_t<D, K2>>::type { return {}; }
    };
    template<class D, class B> struct common_expression_t : callable_expression_t<D, indexable_expression_t<D, B>> {};

    template<class T, class K> struct index_result : common_expression_t<index_result<T, K>, volatile_t> {
        template<class V> constexpr auto operator=(V) const -> typename std::enable_if<is_expression<V>::value, newindex_t<T, K, V>>::type { return {}; }
    };

    template<class F, class...A> struct call_result : common_expression_t<call_result<F, A...>, volatile_t>, sentence_t {};

    struct placeholder_expression : common_expression_t<placeholder_expression, volatile_t> {};
    template<class E> using placeholder_indexing = index_result<placeholder_expression, E>;
    template<class...Initializers> struct newtable_with_assign_initalizers : indexable_expression_t<newtable_with_assign_initalizers<Initializers...>, identity_t> {};
    template<class...Initializers> struct newtable_with_array_initalizers : indexable_expression_t<newtable_with_array_initalizers<Initializers...>, identity_t> {};

    struct newtable_t : indexable_expression_t<newtable_t, identity_t> {
        template<class...Initializers> constexpr auto operator()(Initializers...) const
        -> typename std::enable_if<is_assign_stmts<Initializers...>::value, newtable_with_assign_initalizers<Initializers...>>::type { return {}; }
        template<class...Initializers> constexpr auto operator()(Initializers...) const
        -> typename std::enable_if<is_expressions<Initializers...>::value, newtable_with_array_initalizers<Initializers...>>::type { return {}; }
    };
    constexpr newtable_t _newtable;

    template<int N> struct upvalue_t : common_expression_t<upvalue_t<N>, expression_t> {};
    template<> struct upvalue_t<0>; // undefined

    struct global_t : indexable_expression_t<global_t, constant_t> {};
    struct registry_t : indexable_expression_t<registry_t, constant_t> {};
    struct empty_sentence_t : sentence_t {};
    template<class...E> struct sequence_t : sentence_t {};

    struct ipairs_t : callable_expression_t<ipairs_t, expression_t> {};
    struct pairs_t : callable_expression_t<pairs_t, expression_t> {};

    template<int OP, class A, class B> struct binary_op_t : common_expression_t<binary_op_t<OP, A, B>, volatile_t> {};
    template<int OP, class A> struct unary_op_t : common_expression_t<unary_op_t<OP, A>, volatile_t> {};
    template<int OP, bool INV, class A, class B> struct compare_op_t : common_expression_t<compare_op_t<OP, INV, A, B>, volatile_t> {};

    //template<class A, class B> constexpr auto operator + (A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, binary_op_t<LUA_OPADD, A, B>>::type { return {}; }
#define LUAEXP_BINARY_OP_GEN(R, OP) template<class A, class B> constexpr auto operator R (A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, binary_op_t<OP, A, B>>::type { return {}; }
#define LUAEXP_UNARY_OP_GEN(R, OP) template<class A> constexpr auto operator R (A) -> typename std::enable_if<is_expression<A>::value, unary_op_t<OP, A>>::type { return {}; }
#define LUAEXP_COMPARE_OP_GEN(R, OP, INV) template<class A, class B> constexpr auto operator R (A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, compare_op_t<OP, INV, A, B>>::type { return {}; }
    LUAEXP_BINARY_OP_GEN(+, LUA_OPADD)
    LUAEXP_BINARY_OP_GEN(-, LUA_OPSUB)
    LUAEXP_BINARY_OP_GEN(*, LUA_OPMUL)
    LUAEXP_BINARY_OP_GEN(/, LUA_OPIDIV)
    LUAEXP_BINARY_OP_GEN(%, LUA_OPMOD)
    LUAEXP_BINARY_OP_GEN(&, LUA_OPBAND)
    LUAEXP_BINARY_OP_GEN(|, LUA_OPBOR)
    LUAEXP_BINARY_OP_GEN(^, LUA_OPBXOR)
    LUAEXP_BINARY_OP_GEN(<<, LUA_OPSHL)
    LUAEXP_BINARY_OP_GEN(>>, LUA_OPSHR)
    LUAEXP_UNARY_OP_GEN(-, LUA_OPUNM)
    LUAEXP_UNARY_OP_GEN(~, LUA_OPBNOT)
    LUAEXP_COMPARE_OP_GEN(==, LUA_OPEQ, false)
    LUAEXP_COMPARE_OP_GEN(<, LUA_OPLT, false)
    LUAEXP_COMPARE_OP_GEN(<=, LUA_OPLE, false)
    LUAEXP_COMPARE_OP_GEN(!=, LUA_OPEQ, true)
    LUAEXP_COMPARE_OP_GEN(>=, LUA_OPLT, true)
    LUAEXP_COMPARE_OP_GEN(>, LUA_OPLE, true)
#undef LUAEXP_OP_GEN
#undef LUAEXP_UNARY_OP_GEN
#undef LUAEXP_COMPARE_OP_GEN

    template<class A, class B> constexpr auto _pow(A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, binary_op_t<LUA_OPPOW, A, B>>::type { return {}; }
    template<class A, class B> constexpr auto _fmod(A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, binary_op_t<LUA_OPDIV, A, B>>::type { return {}; }

    template<class A, class B> struct and_op_t : common_expression_t<and_op_t<A, B>, volatile_t> {};
    template<class A, class B> struct or_op_t : common_expression_t<or_op_t<A, B>, volatile_t> {};
    template<class A> struct not_op_t : common_expression_t<not_op_t<A>, volatile_t> {};
    template<class A> struct len_op_t : common_expression_t<len_op_t<A>, volatile_t> {};

    template<class A, class B> constexpr auto operator && (A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, and_op_t<A, B>>::type { return {}; }
    template<class A, class B> constexpr auto operator || (A, B) -> typename std::enable_if<is_expression<A>::value && is_expression<B>::value, or_op_t<A, B>>::type { return {}; }
    template<class A> constexpr auto operator ! (A) -> typename std::enable_if<is_expression<A>::value, not_op_t<A>>::type { return {}; }
    template<class A> constexpr auto _len(A) -> typename std::enable_if<is_expression<A>::value, len_op_t<A>>::type { return {}; }

    template<class E, class A> struct if_then_end_t : sentence_t {};
    template<class E, class A, class B> struct if_then_else_end_t : sentence_t {};
    template<class E, class A> struct while_do_end_t : sentence_t {};
    template<class A, class I, class... V> struct for_in_do_end_t : sentence_t {};
    template<class A, class E> struct repeat_until_t : sentence_t {};

    namespace incomplete {
        template<class E, class A, class B> struct if_then_else_t : incomplete_sentence_t {
            if_then_else_end_t<E, A, B> _end;
        };

        template<class E, class A> struct if_then_t : incomplete_sentence_t {
            if_then_end_t<E, A> _end;
            constexpr auto _else() const -> if_then_else_t<E, A, empty_sentence_t> { return {}; }
            template<class B>
            constexpr auto _else(B) const -> typename std::enable_if<is_sentence<B>::value, if_then_else_t<E, A, B>>::type { return {}; }
            template<class...Bs>
            constexpr auto _else(Bs...) const -> typename std::enable_if<is_sentences<Bs...>::value, if_then_else_t<E, A, sequence_t<Bs...>>>::type { return {}; }
        };
        template<class E> struct if_t : incomplete_sentence_t {
            constexpr auto _then() const -> if_then_t<E, empty_sentence_t> { return {}; }
            template<class A>
            constexpr auto _then(A) const -> typename std::enable_if<is_sentence<A>::value, if_then_t<E, A>>::type { return {}; }
            template<class...As>
            constexpr auto _then(As...) const -> typename std::enable_if<is_sentences<As...>::value, if_then_t<E, sequence_t<As...>>>::type { return {}; }
        };

        template<class E, class A> struct while_do_t : incomplete_sentence_t {
            while_do_end_t<E, A> _end;
        };

        template<class E> struct while_t : incomplete_sentence_t {
            constexpr auto _do() const -> while_do_t<empty_sentence_t, E> { return {}; }
            template <class A>
            constexpr auto _do(A) const -> typename std::enable_if<is_sentence<A>::value, while_do_t<A, E>>::type { return {}; }
            template <class...As>
            constexpr auto _do(As...) const -> typename std::enable_if<is_sentences<As...>::value, while_do_t<sequence_t<As...>, E>>::type { return {}; }
        };

        template <class A, class I, class... V> struct for_in_do_t : incomplete_sentence_t {
            for_in_do_end_t<A, I, V...> _end;
        };

        template <class I, class... V> struct for_in_t : incomplete_sentence_t {
            constexpr auto _do() const -> for_in_do_t<empty_sentence_t, I, V...> { return {}; }
            template <class A>
            constexpr auto _do(A) const -> typename std::enable_if<is_sentence<A>::value, for_in_do_t<A, I, V...>>::type { return {}; }
            template <class...As>
            constexpr auto _do(As...) const -> typename std::enable_if<is_sentences<As...>::value, for_in_do_t<sequence_t<As...>, I, V...>>::type { return {}; }
        };

        template <class... V> struct for_t : incomplete_sentence_t {
            template <class I>
            constexpr auto _in(I) const -> typename std::enable_if<is_expression<I>::value, for_in_t<I, V...>>::type { return {}; }
        };

        template<class A> struct repeat_t : incomplete_sentence_t {
            template<class E>
            constexpr auto _until(E) const -> typename std::enable_if<is_volatile<E>::value, repeat_until_t<A, E>>::type { return {}; }
        };
    }

    template<class E> constexpr auto _if(E) -> typename std::enable_if<is_expression<E>::value, incomplete::if_t<E>>::type { return {}; }
    template<class E> constexpr auto _while(E) -> typename std::enable_if<is_volatile<E>::value, incomplete::while_t<E>>::type { return {}; }
    template<class...V> constexpr auto _for(V...) -> typename std::enable_if<is_locals<V...>::value, incomplete::for_t<V...>>::type { return {}; }
    constexpr auto _repeat() -> incomplete::repeat_t<empty_sentence_t> { return {}; }
    template <class A> constexpr auto _repeat(A) -> typename std::enable_if<is_sentence<A>::value, incomplete::repeat_t<A>>::type { return {}; }
    template <class...As> constexpr auto _repeat(As...) -> typename std::enable_if<is_sentences<As...>::value, incomplete::repeat_t<sequence_t<As...>>>::type { return {}; }

    template<class...E> struct return_value_t : sentence_t {};
    struct return_void_t : sentence_t {
        template<class...E> auto operator()(E...) const -> typename std::enable_if<is_expressions<E...>::value, return_value_t<E...>>::type { return {}; }
    };
    constexpr return_void_t _return;

    template<int N, class E> using local_assign_t = assign_t<local_t<N>, E>;
    template<int N> struct local_t : common_expression_t<local_t<N>, local_base_t> {
        template<class E> constexpr auto operator=(E) const -> typename std::enable_if<is_expression<E>::value, local_assign_t<N, E>>::type { return {}; }
    };
    template<> struct local_t<0>; // undefined

    template<int N, class E> struct local_decl_init_t : sentence_t {};
    template<int N> struct local_decl_t : sentence_t
    {
        template<class E> constexpr auto operator=(E) const -> typename std::enable_if<is_expression<E>::value, local_decl_init_t<N, E>>::type { return {}; }
    };
    template<int N>
    constexpr auto _local(local_t<N>)
    {
        return local_decl_t<N>();
    }
    template<int N> struct function_arg_t : common_expression_t<function_arg_t<N>, local_base_t> {
        // TODO : mutable
    };

    struct exception_handler_t : expression_t {};
    struct exception_t : expression_t {};
    constexpr exception_handler_t _debug_traceback;

    template<class Body, class...Args> struct function_arg_body_end_t : expression_t {};

    namespace incomplete
    {
        struct incomplete_expression_t {};
        template<class Body, class...Args> struct function_arg_body_t : incomplete_expression_t
        {
            function_arg_body_end_t<Body, Args...> _end;
        };

        template<class...Args> struct function_arg_t : incomplete_expression_t
        {
            template <class A> constexpr auto operator()(A) const -> typename std::enable_if<is_sentence<A>::value, function_arg_body_t<A, Args...>>::type { return {}; }
            template <class...As> constexpr auto operator()(As...) const -> typename std::enable_if<is_sentences<As...>::value, function_arg_body_t<sequence_t<As...>, Args...>>::type { return {}; }
        };
    }
    constexpr auto _function() -> incomplete::function_arg_t<> { return {}; }
    template<class...E> constexpr auto _function(E...) -> typename std::enable_if<is_function_args<E...>::value, incomplete::function_arg_t<E...>>::type { return {}; }

    // rotate to left-fold
    template<class L, class R>
    constexpr auto operator,(L, R) -> typename std::enable_if<is_sentence<L>::value&& is_sentence<R>::value, sequence_t<L, R>>::type { return {}; }
    template<class...E1, class R2>
    constexpr auto operator,(sequence_t<E1...>, R2) -> typename std::enable_if<is_sentence<R2>::value, sequence_t<E1..., R2>>::type { return {}; }
    template<class...E2, class L2>
    constexpr auto operator,(L2, sequence_t<E2...>) -> typename std::enable_if<is_sentence<L2>::value, sequence_t<L2, E2...>>::type { return {}; }
    template<class...E1, class...E2>
    constexpr auto operator,(sequence_t<E1...>, sequence_t<E2...>) -> sequence_t<E1..., E2...> { return {}; }

    // eval
    template<std::size_t N, class E> struct IndexedLuaStackValue {};
    template<class Seq, class...E> struct LuaStackValue;
    template<std::size_t...I, class...E> struct LuaStackValue<std::index_sequence<I...>, E...> : IndexedLuaStackValue<I + 1, E>... {};
    template<class...E> struct LuaExecStack : LuaStackValue<std::index_sequence_for<E...>, E...> {};
    using LuaEmptyStack = LuaExecStack<>;

    struct LuaReturnStack {
        lua_State* L;
        int esp;
        int return_num;
        bool handled;
        bool exception;

        LuaReturnStack(lua_State* L, int num, bool exception) : L(L), esp(lua_gettop(L)), return_num(num), handled(false), exception(exception) {}
        LuaReturnStack(const LuaReturnStack&) = delete;
        LuaReturnStack(LuaReturnStack&& other) : L(other.L), esp(other.esp), return_num(other.return_num), handled(false), exception(other.exception)
        {
            other.handled = true;
        }
        LuaReturnStack& operator=(const LuaReturnStack&) = delete;
        LuaReturnStack& operator=(LuaReturnStack&&) = delete;

        ~LuaReturnStack()
        {
            assert(handled);
        }
        int retain() &&
        {
            assert(!handled);
            handled = true;
            return return_num;
        }
        template<class> void release_impl(std::true_type)
        {
            lua_pop(L, return_num);
        }
        template<class T> T release_impl(std::false_type)
        {
            T result;
            Get(L, -return_num, result);
            lua_pop(L, return_num);

            return result;
        }
        template<class T = void>  T release() &&
        {
            assert(!handled);
            handled = true;

            assert(lua_gettop(L) == esp);

            return release_impl<T>(std::is_void<T>());
        }
    };

    struct NotFoundTag {
        using type = NotFoundTag;
    };
    struct MultiFoundTag {
        using type = MultiFoundTag;
    };
    struct DeletedTag {
        using type = DeletedTag;
    };
    namespace detail {

        template<class T> struct NotEqualsToFn {
            template<class E> struct type : std::integral_constant<bool, !std::is_same<T, E>::value> {};
        };

        template<class T, class E> struct LuaStackHasValueImpl;
        template<class...StackVals, class E> struct LuaStackHasValueImpl<LuaExecStack<StackVals...>, E> : all_of<NotEqualsToFn<E>::template type, StackVals...> {};

        template<class E> struct LuaStackFindImpl {
            template<std::size_t N> auto operator()(const IndexedLuaStackValue<N, E>&) const  -> std::integral_constant<std::size_t, N>;
            template<class T> auto operator()(const T &) const -> typename std::conditional<LuaStackHasValueImpl<T, E>::value, MultiFoundTag, NotFoundTag>::type;
        };

        template<std::size_t N> struct LuaStackAtImpl {
            template<class E> auto operator()(const IndexedLuaStackValue<N, E>&) const ->type_identity<E>;
            auto operator()(...) const -> NotFoundTag;
        };

        template<class T, class E> struct LuaStackPushImpl;
        template<class...StackVals, class E> struct LuaStackPushImpl<LuaExecStack<StackVals...>, E> : type_identity<LuaExecStack<StackVals..., E>> {};;
        template<class...StackVals> struct LuaStackPushImpl<LuaExecStack<StackVals...>, NotFoundTag>;
        template<class...StackVals> struct LuaStackPushImpl<LuaExecStack<StackVals...>, DeletedTag> : type_identity<LuaExecStack<StackVals...>> {};;

        template<std::size_t N, class E, class...StackVals> struct LuaStackFindFirstImpl2;
        template<std::size_t N, class E> struct LuaStackFindFirstImpl2<N, E> : NotFoundTag {};
        template<std::size_t N, class E, class...Rest> struct LuaStackFindFirstImpl2<N, E, E, Rest...> : std::integral_constant<std::size_t, N + 1> {};
        template<std::size_t N, class E, class First, class...Rest> struct LuaStackFindFirstImpl2<N, E, First, Rest...> : LuaStackFindFirstImpl2<N + 1, E, Rest...> {};

        template<class T, class E> struct LuaStackFindFirstImpl;
        template<class...StackVals, class E> struct LuaStackFindFirstImpl<LuaExecStack<StackVals...>, E> : LuaStackFindFirstImpl2<0, E, StackVals...> {};


        template<class T, class...E> struct LuaStackPushMultiImpl;
        template<class T, class First, class...Rest> struct LuaStackPushMultiImpl<T, First, Rest...> : LuaStackPushMultiImpl<typename LuaStackPushImpl<T, First>::type, Rest...> {};
        template<class T> struct LuaStackPushMultiImpl<T> : type_identity<T> {};

        template<class...OldStackVal> struct IsStackContainsImpl
        {
            template<class...DeltaStackVal>
            auto operator()(OldStackVal..., DeltaStackVal...) const -> std::true_type;

            auto operator()(...) const -> std::false_type;
        };

        template<class T> struct LuaStackSizeImpl;
        template<class...StackVals> struct LuaStackSizeImpl<LuaExecStack<StackVals...>> : std::integral_constant<std::size_t, sizeof...(StackVals)> {};
    }    // namespace detail
    template <class T, class E> struct LuaStackFind : decltype(detail::LuaStackFindImpl<E>()(T())) {};
    template<class T, class E> struct LuaStackFindFirst : detail::LuaStackFindFirstImpl<T, E> {};
    template<class T, class E> struct LuaStackHasValue : detail::LuaStackHasValueImpl<T, E> {};
    template<class T, std::size_t N> struct LuaStackAt : decltype(detail::LuaStackAtImpl<N>()(T())) {};
    template<class T, class E> struct LuaStackPush : detail::LuaStackPushImpl<T, E> {};
    template<class T, class...E> struct LuaStackPushMulti : detail::LuaStackPushMultiImpl<T, E...> {};
    template<class T> struct LuaStackSize : detail::LuaStackSizeImpl<T> {};

    template<class S1, class S2> struct IsStackContains;
    template<class...OldStackVal, class...NewStackVal> struct IsStackContains<LuaExecStack<OldStackVal...>, LuaExecStack<NewStackVal...>> : decltype(detail::IsStackContainsImpl<OldStackVal...>()(NewStackVal()...)) {};
    template<class...NewStackVal> struct IsStackContains<LuaExecStack<>, LuaExecStack<NewStackVal...>> : std::true_type {};

    struct LvmAbstractStack {

        template<class Stack> static constexpr int GetStackSize(Stack s)
        {
            return LuaStackSize<Stack>::value;
        }
        template<class...StackVal, class NewStackVal> static constexpr auto UpdateStackValPush(LuaExecStack<StackVal...>, NewStackVal e)
        {
            return LuaExecStack<StackVal..., NewStackVal>();
        }
        template<class Stack, std::size_t...I> static constexpr auto UpdateStackValResize(Stack, std::index_sequence<I...>)
        {
            return LuaExecStack<typename LuaStackAt<Stack, I + 1>::type...>();
        }
        template <std::size_t N, class... StackVal>
        static constexpr auto UpdateStackValPop(LuaExecStack<StackVal...> s, std::integral_constant<std::size_t, N> = {})
        {
            return UpdateStackValResize(s, std::make_index_sequence<sizeof...(StackVal) - N>());
        }
        template<class Stack, class...E> static constexpr auto UpdateStackValPushMulti(Stack, E...e)
        {
            return typename LuaStackPushMulti<Stack, E...>::type();
        }
    };

    struct LvmRunner : LvmAbstractStack {
        // deduced return type (aka auto) will crash poor clang++ on NDK
        using ReturnType = int;
        template <class Body, class... Args, class Stack, class NextFn, int ArgsNum = sizeof...(Args)>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, function_arg_body_end_t<Body, Args...> e, NextFn eip)
        {
            // TODO : handle local as upvalue
            lua_Debug ar = {};
            int upvaluenum = 0;
            if (lua_getstack(L, 0, &ar))
            {
                lua_getinfo(L, "u", &ar);
                // auto funcindex = ar.i_ci->func;
                upvaluenum = ar.nups;
            }
            for (int i = 1; i <= upvaluenum; ++i)
            {
                // TODO : mutable upvalue
                lua_pushvalue(L, lua_upvalueindex(i));
            }
            auto f = [](lua_State* L) -> int {
                lua_settop(L, ArgsNum);
                return RunSentence(L, LuaExecStack<Args...>(), Body(), [](lua_State *L, auto s1){ return ReturnVoid(L, s1); });
            };
            lua_pushcclosure(L, f, upvaluenum);
            return eip(L, UpdateStackValPush(s, e));
        }

        template<class T, class K, class Stack, class NextFn, std::size_t N>
        static ReturnType PushIndexResultToLuaStack(lua_State* L, Stack s, index_result<T, K> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            return PushSingleExpressionToLuaStack(L, s, K(), [](lua_State *L, auto s1) {
                lua_gettable(L,  N);
                return InvokeStatelessLambda<NextFn>(L, UpdateStackValPush(UpdateStackValPop<1>(s1), index_result<T, K>()));    // -1 + 1
            });
        }
        template<class T, char... K, class Stack, class NextFn, std::size_t N>
        static ReturnType PushIndexResultToLuaStack(lua_State* L, Stack s, index_result<T, string_constant_t<K...>> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            constexpr char str[] = {K..., '\0'};
            lua_getfield(L, N, str);
            return InvokeStatelessLambda<NextFn>(L, UpdateStackValPush(s, e));    // +1
        }
        template<class T, long long K, class Stack, class NextFn, std::size_t N>
        static ReturnType PushIndexResultToLuaStack(lua_State* L, Stack s, index_result<T, integer_constant_t<K>> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            lua_geti(L, N, K);
            return InvokeStatelessLambda<NextFn>(L, UpdateStackValPush(s, e));    // +1
        }
        template<class T, class K, class Stack, class NextFn>
        static ReturnType PushIndexResultToLuaStack(lua_State* L, Stack s, index_result<T, K> e, NextFn eip, NotFoundTag table_pos)
        {
            return PushSingleExpressionToLuaStack(L, s, T(), [](lua_State *L, auto s1) {
                return PushIndexResultToLuaStack(L, s1, index_result<T, K>(), ConstructStatelessLambda<NextFn>(), LuaStackFindFirst<decltype(s1), T>());
            });
        }
        template<class T, class K, class V, class Stack, class NextFn, std::size_t N>
        static ReturnType RunNewIndex(lua_State* L, Stack s, newindex_t<T, K, V> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            return PushMultiExpressionsToLuaStack(L, s, [](lua_State *L, auto s1) {
                lua_settable(L, N);
                return InvokeStatelessLambda<NextFn>(L, UpdateStackValPop<2>(s1));    // -2
            }, K(), V());
        }
        template <class T, char... K, class V, class Stack, class NextFn, std::size_t N>
        static ReturnType RunNewIndex(lua_State* L, Stack s, newindex_t<T, string_constant_t<K...>, V> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            return PushMultiExpressionsToLuaStack(L, s, [](lua_State* L, auto s1) {
                constexpr char str[] = {K..., '\0'};
                lua_setfield(L, N, str);
                return InvokeStatelessLambda<NextFn>(L, UpdateStackValPop<1>(s1));    // -1
            }, V());
        }
        template <class T, long long K, class V, class Stack, class NextFn, std::size_t N>
        static ReturnType RunNewIndex(lua_State* L, Stack s, newindex_t<T, integer_constant_t<K>, V> e, NextFn eip, std::integral_constant<std::size_t, N> table_pos)
        {
            return PushMultiExpressionsToLuaStack(L, s, [](lua_State* L, auto s1) {
                lua_seti(L, N, K);
                return InvokeStatelessLambda<NextFn>(L, UpdateStackValPop<1>(s1));    // -1
            }, V());
        }
        template<class T, class K, class V, class Stack, class NextFn>
        static ReturnType RunNewIndex(lua_State* L, Stack s, newindex_t<T, K, V> e, NextFn eip, NotFoundTag table_pos)
        {
            return PushSingleExpressionToLuaStack(L, s, T(), [](lua_State *L, auto s1) {
                return RunNewIndex(L, s1, newindex_t<T, K, V>(), ConstructStatelessLambda<NextFn>(), LuaStackFindFirst<decltype(s1), T>());
            });
        }

        template<int N, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, local_t<N> e, NextFn eip)
        {
            static_assert(sizeof(Stack) < 0, "local variable was not decleared");
        }

        template <int N, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, upvalue_t<N> e, NextFn eip)
        {
            lua_pushvalue(L, lua_upvalueindex(N));
            return eip(L, UpdateStackValPush(s, e));
        }
        template<class T, class K, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, index_result<T, K> e, NextFn eip)
        {
            return PushIndexResultToLuaStack(L, s, e, eip, LuaStackFindFirst<Stack, T>());
        }
        template <class F, class... A, class Stack, class NextFn, std::size_t ArgsNum = sizeof...(A)>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, call_result<F, A...> e, NextFn eip)
        {
            lua_checkstack(L, 1 + ArgsNum);
            return PushMultiExpressionsToLuaStack(L, s,
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      auto k = [](lua_State* L, int status, lua_KContext ctx) {
                                                          auto s1_ = decltype(s1)();
                                                          auto s2 = UpdateStackValPop<ArgsNum + 1>(s1_);
                                                          auto s3 = UpdateStackValPush(s2, call_result<F, A...>());
                                                          return InvokeStatelessLambda<NextFn>(L, s3);    // -(N+1) +1
                                                      };
                                                      lua_callk(L, ArgsNum, 1, lua_KContext(), k);
                                                      return k(L, LUA_OK, lua_KContext());
                                                  }, F(), A()...);
        }
        template <int OP, class A, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, unary_op_t<OP, A> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, A(), [](lua_State* L, auto s1)
            {
                auto s2 = UpdateStackValPop<1>(s1);
                auto s3 = UpdateStackValPush(s2, unary_op_t<OP, A>());
                lua_arith(L, OP);
                return InvokeStatelessLambda<NextFn>(L, s3);
            });
        }
        template <int OP, class A, class B, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, binary_op_t<OP, A, B> e, NextFn eip)
        {
            return PushMultiExpressionsToLuaStack(L, s,
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      auto s2 = UpdateStackValPop<2>(s1);
                                                      auto s3 = UpdateStackValPush(s2, binary_op_t<OP, A, B>());
                                                      lua_arith(L, OP);
                                                      return InvokeStatelessLambda<NextFn>(L, s3);
                                                  }, A(), B());
        }
        template <int OP, bool INV, class A, class B, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, compare_op_t<OP, INV, A, B> e, NextFn eip)
        {
            // TODO : quick path with if/for/while/do while
            return PushMultiExpressionsToLuaStack(L, s,
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      lua_pushboolean(L, lua_compare(L, -2 + INV, -1 - INV, OP));
                                                      auto s2 = UpdateStackValPush(s1, compare_op_t<OP, INV, A, B>());
                                                      return InvokeStatelessLambda<NextFn>(L, s2);
                                                  }, A(), B());
        }
        template <class A, class B, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, and_op_t<A, B> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, A(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      if (!lua_toboolean(L, -1))
                                                      {
                                                          auto s2 = UpdateStackValPush(UpdateStackValPop<1>(s1), and_op_t<A, B>());
                                                          return InvokeStatelessLambda<NextFn>(L, s2);
                                                      }
                                                      else
                                                      {
                                                          return PushSingleExpressionToLuaStack(L, s1, B(),
                                                                                                [](lua_State* L, auto s2)
                                                                                                {
                                                                                                    auto s3 = UpdateStackValPush(UpdateStackValPop<1>(s2), or_op_t<A, B>());
                                                                                                    return InvokeStatelessLambda<NextFn>(L, s3);
                                                                                                });
                                                      }
                                                  });
        }
        template <class A, class B, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, or_op_t<A, B> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, A(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      if (lua_toboolean(L, -1))
                                                      {
                                                          auto s2 = UpdateStackValPush(UpdateStackValPop<1>(s1), and_op_t<A, B>());
                                                          return InvokeStatelessLambda<NextFn>(L, s2);
                                                      }
                                                      else
                                                      {
                                                          return PushSingleExpressionToLuaStack(L, s1, B(),
                                                                                                [](lua_State* L, auto s2)
                                                                                                {
                                                                                                    auto s3 = UpdateStackValPush(UpdateStackValPop<1>(s2), or_op_t<A, B>());
                                                                                                    return InvokeStatelessLambda<NextFn>(L, s3);
                                                                                                });
                                                      }
                                                  });
        }
        template <class A, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, not_op_t<A> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, A(), [](lua_State* L, auto s1)
            {
                bool result = !lua_toboolean(L, -1);
                lua_pushboolean(L, result);
                auto s2 = UpdateStackValPush(s1, not_op_t<A>());
                return InvokeStatelessLambda<NextFn>(L, s2);
            });
        }
        template <class A, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, len_op_t<A> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, A(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      lua_len(L, -1);
                                                      auto s2 = UpdateStackValPush(s1, len_op_t<A>());
                                                      return InvokeStatelessLambda<NextFn>(L, s2);
                                                  });
        }
        template<long long N, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, integer_constant_t<N> e, NextFn eip)
        {
            lua_pushinteger(L, N);
            return eip(L, UpdateStackValPush(s, e));
        }
        template<bool N, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, boolean_constant_t<N> e, NextFn eip)
        {
            lua_pushboolean(L, N);
            return eip(L, UpdateStackValPush(s, e));
        }
        template<char...C, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, string_constant_t<C...> e, NextFn eip)
        {
            constexpr char str[] = { C..., '\0' };
            lua_pushlstring(L, str, sizeof...(C));
            return eip(L, UpdateStackValPush(s, e));
        }
        template<class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, nil_constant_t e, NextFn eip)
        {
            lua_pushnil(L);
            return eip(L, UpdateStackValPush(s, e));
        }
        template <class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, newtable_t e, NextFn eip)
        {
            lua_newtable(L);
            return eip(L, UpdateStackValPush(s, e)); // + 1
        }
        template <class Stack, class NextFn, class K, class V, class... Rest>
        static ReturnType SetNewTableWithMapInitalizer(lua_State* L, Stack s, NextFn, assign_t<placeholder_indexing<K>, V>, Rest...)
        {
            return PushMultiExpressionsToLuaStack(L, s, [](lua_State *L, auto s1) {
                lua_settable(L, -3);
                Stack s2 = UpdateStackValPop<2>(s1);
                return SetNewTableWithMapInitalizer(L, s2, ConstructStatelessLambda<NextFn>(), Rest()...);
            }, K(), V());
        }
        template <class Stack, class NextFn, char...K, class V, class... Rest>
        static ReturnType SetNewTableWithMapInitalizer(lua_State* L, Stack s, NextFn, assign_t<placeholder_indexing<string_constant_t<K...>>, V>,
                                                       Rest...)
        {
            return PushSingleExpressionToLuaStack(L, s, V(), [](lua_State *L, auto s1) {
                constexpr char str[] = {K..., '\0'};
                lua_setfield(L, -2, str);
                Stack s2 = UpdateStackValPop<1>(s1);
                return SetNewTableWithMapInitalizer(L, s2, ConstructStatelessLambda<NextFn>(), Rest()...);
            });
        }
        template <class Stack, class NextFn, long long K, class V, class... Rest>
        static ReturnType SetNewTableWithMapInitalizer(lua_State* L, Stack s, NextFn, assign_t<placeholder_indexing<integer_constant_t<K>>, V>, Rest...)
        {
            return PushSingleExpressionToLuaStack(L, s, V(), [](lua_State *L, auto s1) {
                lua_seti(L, -2, K);
                Stack s2 = UpdateStackValPop<1>(s1);
                return SetNewTableWithMapInitalizer(L, s2, ConstructStatelessLambda<NextFn>(), Rest()...);
            });
        }
        template <class Stack, class NextFn>
        static ReturnType SetNewTableWithMapInitalizer(lua_State* L, Stack s, NextFn eip)
        {
            return eip(L, s);
        }
        template <class Stack, class NextFn, class... Initializers>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, newtable_with_assign_initalizers<Initializers...> e, NextFn eip)
        {
            lua_createtable(L, 0, sizeof...(Initializers));
            auto s2 = UpdateStackValPush(s, e);
            return SetNewTableWithMapInitalizer(L, s2, eip, Initializers()...);
        }
        template <std::size_t N, class Stack, class NextFn, class First, class... Rest>
        static ReturnType SetNewTableWithArray(lua_State* L, Stack s, NextFn eip, First, Rest...)
        {
            return PushSingleExpressionToLuaStack(L, s, First(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      lua_seti(L, -2, N);
                                                      Stack s2 = UpdateStackValPop<1>(s1);
                                                      return SetNewTableWithArray<N + 1>(L, s2, ConstructStatelessLambda<NextFn>(), Rest()...);
                                                  });
        }
        template <std::size_t N, class Stack, class NextFn>
        static ReturnType SetNewTableWithArray(lua_State* L, Stack s, NextFn eip)
        {
            return eip(L, s);
        }
        template <class Stack, class NextFn, class... E>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, newtable_with_array_initalizers<E...> e, NextFn eip)
        {
            lua_createtable(L, sizeof...(E), 0);
            auto s2 = UpdateStackValPush(s, e);
            return SetNewTableWithArray<1>(L, s2, eip);
        }
        template<class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, global_t e, NextFn eip)
        {
            lua_pushglobaltable(L);
            return eip(L, UpdateStackValPush(s, e));
        }
        template <char... C, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, index_result<global_t, string_constant_t<C...>> e, NextFn eip)
        {
            constexpr char str[] = {C..., '\0'};
            lua_getglobal(L, str);
            return eip(L, UpdateStackValPush(s, e));
        }
        template <class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, registry_t e, NextFn eip)
        {
            lua_pushvalue(L, LUA_REGISTRYINDEX);
            return eip(L, UpdateStackValPush(s, e));
        }
        template <class Expression, class Stack, class NextFn, std::size_t N>
        static ReturnType PushSingleExpressionToLuaStackCopy(lua_State* L, Stack s, Expression e, NextFn eip, std::integral_constant<std::size_t, N> pos)
        {
            lua_pushvalue(L, pos);
            return eip(L, UpdateStackValPush(s, e));
        }

        template <class Expression, class Stack, class NextFn, std::size_t N>
        static ReturnType PushSingleExpressionToLuaStack(lua_State* L, Stack s, Expression e, NextFn eip, std::integral_constant<std::size_t, N> pos)
        {
            return PushSingleExpressionToLuaStackCopy(L, s, e, eip, pos);
        }

        template <class Expression, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStack(lua_State* L, Stack s, Expression e, NextFn eip, NotFoundTag)
        {
            return PushSingleExpressionToLuaStackNew(L, s, e, eip);
        }

        template<class Expression, class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStack(lua_State* L, Stack s, Expression e, NextFn eip)
        {
            static_assert(is_expression<Expression>::value, "e requires to be an expression");
            return PushSingleExpressionToLuaStack(L, s, e, eip, typename LuaStackFindFirst<Stack, Expression>::type());
        }

        template<class First, class Second, class...Rest, class Stack, class NextFn>
        static ReturnType PushMultiExpressionsToLuaStack(lua_State* L, Stack s, NextFn eip, First e, Second e2, Rest... rest)
        {
            return PushSingleExpressionToLuaStack(L, s, e, [](lua_State *L, auto s2) {
                return PushMultiExpressionsToLuaStack(L, s2, ConstructStatelessLambda<NextFn>(), Second(), Rest()...);
            });
        }
        template<class First, class Stack, class NextFn>
        static ReturnType PushMultiExpressionsToLuaStack(lua_State* L, Stack s, NextFn eip, First e)
        {
            return PushSingleExpressionToLuaStack(L, s, e, [](lua_State *L, auto s2) {
                return InvokeStatelessLambda<NextFn>(L, s2);
            });
        }
        template<class Stack, class NextFn>
        static ReturnType PushMultiExpressionsToLuaStack(lua_State* L, Stack s, NextFn eip)
        {
            return eip(L, s);
        }

        template <class Stack, class NextFn>
        static ReturnType PushSingleExpressionToLuaStackNew(lua_State* L, Stack s, exception_handler_t e, NextFn eip)
        {
            lua_getglobal(L, "debug");           // #1 = debug
            lua_getfield(L, -1, "traceback");    // #2 = debug.traceback
            lua_remove(L, -2);                   // remove 'debug'    // #1 = debug.traceback
            return eip(L, UpdateStackValPush(s, e));
        }

        template <class Stack>
        static auto ClearLuaStack(lua_State* L, Stack s)
        {
            int size = GetStackSize(s);
            lua_pop(L, size);
            return LuaEmptyStack();
        }

        template<class OldStack, class NewStack>
        static auto PopRecoverLuaStack(lua_State* L, OldStack olds, NewStack news)
        {
            static_assert(IsStackContains<OldStack, NewStack>::value, "...");
            lua_settop(L, GetStackSize(olds));
            return olds;
        }

        template<template <class> class Cond, class...OldStackVal> struct RemoveFromLuaStackIfImplBound : LvmAbstractStack
        {
            using OldStack = LuaExecStack<OldStackVal...>;

            template <class...DeltaStackVal, std::size_t...I>
            auto operator()(lua_State *L, LuaExecStack<OldStackVal..., DeltaStackVal...> news, std::index_sequence<I...>) const
            {
                bool to_be_deleted[] = { Cond<DeltaStackVal>::value... };
                for (int i = 1; i <= sizeof...(DeltaStackVal); ++i)
                {
                    if(to_be_deleted[sizeof...(DeltaStackVal) - i])
                        lua_remove(L, GetStackSize(news) - i);
                }
                return UpdateStackValPushMulti( OldStack(), typename std::conditional<Cond<DeltaStackVal>::value, DeletedTag, DeltaStackVal>::type()... );
            }
            template <class...DeltaStackVal>
            auto operator()(lua_State* L, LuaExecStack<OldStackVal..., DeltaStackVal...> news) const
            {
                return (*this)(L, news, std::index_sequence_for<DeltaStackVal...>());
            }
            auto operator()(lua_State* L, LuaExecStack<OldStackVal...> news) const
            {
                // nothing to do
                return news;
            }
        };

        template <template <class> class Cond, class...OldStackVal, class...NewStackVal>
        static auto RemoveFromLuaStackIfImpl(lua_State* L, LuaExecStack<OldStackVal...> olds, LuaExecStack<NewStackVal...> news)
        {
            return RemoveFromLuaStackIfImplBound<Cond, OldStackVal...>()(L, news);
        }
        template <template <class> class Cond, class OldStack, class NewStack>
        static auto RemoveFromLuaStackIf(lua_State* L, OldStack olds, NewStack news)
        {
            static_assert(IsStackContains<OldStack, NewStack>::value, "...");
            return RemoveFromLuaStackIfImpl<Cond>(L, olds, news);
        }

        template <class OldStack, class NewStack>
        static auto CleanVolatileLuaStack(lua_State* L, OldStack olds, NewStack news)
        {
            assert(GetStackSize(olds) == lua_gettop(L));
            auto s1 = RemoveFromLuaStackIf<is_volatile>(L, olds, news);
            assert(GetStackSize(news) == lua_gettop(L));
            return s1;
        }

        template<class T, class K, class V, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, newindex_t<T, K, V> e, NextFn eip)
        {
            return RunNewIndex(L, s, e, eip, typename LuaStackFindFirst<Stack, T>::type());
        }
        template <class T, char... K, class V, class Stack, class NextFn>
        static ReturnType RunNewIndex(lua_State* L, Stack s, newindex_t<global_t, string_constant_t<K...>, V> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, V(), [](lua_State* L, auto s1) {
                constexpr char str[] = {K..., '\0'};
                lua_setglobal(L, str);
                return InvokeStatelessLambda<NextFn>(L, UpdateStackValPop<1>(s1));    // -1
            });
        }
        template <class F, class... A, class Stack, class NextFn, std::size_t ArgsNum = sizeof...(A)>
        static ReturnType RunSentence(lua_State* L, Stack s, call_result<F, A...> e, NextFn eip)
        {
            lua_checkstack(L, 1 + ArgsNum);
            return PushMultiExpressionsToLuaStack(L, s,
                                                  [](lua_State *L, auto s1)
                                                  {
                                                      auto k = [](lua_State* L, int status, lua_KContext ctx)
                                                      {
                                                          auto s1_ = decltype(s1)();
                                                          auto s2 = UpdateStackValPop<ArgsNum + 1>(s1_);
                                                          return InvokeStatelessLambda<NextFn>(L, s2);    // -(N+1) +1
                                                      };
                                                      lua_callk(L, ArgsNum, 0, lua_KContext(), k);
                                                      return k(L, LUA_OK, lua_KContext());
                                                  }, F(), A()...);
        }

        template<class E, class A, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, if_then_end_t<E, A> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, E(), [](lua_State *L, auto s1)
            {
                return lua_toboolean(L, -1) ?
                       RunSentence(L, PopRecoverLuaStack(L, Stack(), s1), A(), [](lua_State *L, auto s2){ return InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(L, Stack(), s2)); }) :
                       InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(L, Stack(), s1));
            });
        }
        template<class E, class A, class B, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, if_then_else_end_t<E, A, B> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, E(), [](lua_State *L, auto s1)
            {
                return lua_toboolean(L, -1) ?
                       RunSentence(L, PopRecoverLuaStack(L, Stack(), s1), A(), [](lua_State *L, auto s2){ return InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(L, Stack(), s2)); }) :
                       RunSentence(L, PopRecoverLuaStack(L, Stack(), s1), B(), [](lua_State *L, auto s2){ return InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(L, Stack(), s2)); });
            });
        }

        template<class A, class E, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, while_do_end_t<E, A> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(s, E(), [](lua_State *L, auto s1)
            {
                return lua_toboolean(L, -1) ?
                       RunSentence(PopRecoverLuaStack(Stack(), s1), while_do_end_t<E, A>(), ConstructStatelessLambda<NextFn>()) :
                       InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(Stack(), s1));
            });
        }

        template<class A, class E, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, repeat_until_t<A, E> e, NextFn eip)
        {
            return RunSentence(s, A(), [](auto s1)
            {
                return PushSingleExpressionToLuaStack(PopRecoverLuaStack(Stack(), s1), E(), [](lua_State *L, auto s2)
                {
                    return lua_toboolean(L, -1) ?
                           RunSentence(PopRecoverLuaStack(Stack(), s2), repeat_until_t<A, E>(), ConstructStatelessLambda<NextFn>()) :
                           InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(Stack(), s2));
                });
            });
        }

        template<class E, class A, class V1, class V2, class Stack, class NextFn, std::size_t N>
        static ReturnType RunForEachMap(lua_State* L, Stack s, for_in_do_end_t<A, call_result<pairs_t, E>, V1, V2> e, NextFn eip, std::integral_constant<std::size_t, N> map_pos)
        {
            static_assert(std::is_same<typename LuaStackAt<Stack, N>::type, E>::value, "...");

            lua_pushnil(L);
            auto s1 = UpdateStackValPush(s, _nil);  // #1 = nil (first key)

            auto run = [](lua_State* L, auto s1, auto self) -> int
            {
                Stack s;
                if (lua_next(L,  N) != 0)    // throw
                {
                    auto s1_2 = UpdateStackValPush(UpdateStackValPop<1>(s1), V1());    // #1 = key
                    auto s2 = UpdateStackValPush(s1_2, V2());                          // #1 = key, #2 = value

                    return RunSentence(L, s2, A(),
                                       [](lua_State* L, auto s3)
                                       {
                                           auto s1_2_ = decltype(s1_2)();
                                           auto s1_3 = PopRecoverLuaStack(s1_2_, s3);    // #1 = key
                                           return InvokeStatelessLambda<decltype(self)>(L, s1_3);    // #1 = key
                                       });
                }
                else
                {
                    auto s1_2 = UpdateStackValPush(UpdateStackValPop<1>(s1), _nil);    // #2 = nil
                    return InvokeStatelessLambda<NextFn>(L, PopRecoverLuaStack(s, s1_2));
                }
            };

            return run(L, s1, run);
        }

        template <class E, class A, class V1, class V2, class Stack, class NextFn>
        static ReturnType RunForEachMap(lua_State* L, Stack s, for_in_do_end_t<A, call_result<pairs_t, E>, V1, V2> e, NextFn eip, NotFoundTag map_pos)
        {
            return PushSingleExpressionToLuaStack(L, s, E(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      return RunSentence(L, Stack(), for_in_do_end_t<A, call_result<pairs_t, E>, V1, V2>(), ConstructStatelessLambda<NextFn>());
                                                  });
        }

        template <class E, class A, class V1, class V2, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, for_in_do_end_t<A, call_result<pairs_t, E>, V1, V2> e, NextFn eip)
        {
            return RunForEachMap(L, s, e, eip, LuaStackFindFirst<Stack, E>());
        }

        template <class E, class A, class V1, class V2, class Stack, class NextFn, std::size_t N>
        static ReturnType RunForEachArray(lua_State* L, Stack s, for_in_do_end_t<A, call_result<ipairs_t, E>, V1, V2> e, NextFn eip, std::integral_constant<std::size_t, N> arr_pos)
        {
            static_assert(std::is_same<typename LuaStackAt<Stack, N>::type, E>::value, "...");

            lua_pushinteger(L, 0);
            auto s1 = UpdateStackValPush(s, V1()); // #1 = i

            auto run = [](lua_State* L, auto s1, auto self) -> int
            {
                lua_pushinteger(L, 1); // #2 = 1
                lua_arith(L, LUA_OPADD); // #1 = i + 1

                lua_pushvalue(L, -1); // #2 = i
                lua_gettable(L, N); // #2 = v
                auto s2 = UpdateStackValPush(s1, V2());

                if (lua_isnil(L, -1))
                {
                    // iter end
                    PopRecoverLuaStack(L, Stack(), s2);
                    return InvokeStatelessLambda<decltype(eip)>(L, Stack());
                }

                return RunSentence(L, s2, A(),
                                   [](lua_State* L, auto s3)
                                   {
                                       auto s1_ = decltype(s1)();
                                       s1_ = PopRecoverLuaStack(s1_, s3);    // #1 = i
                                       return InvokeStatelessLambda<decltype(self)>(L, s1_);
                                   });
            };

            return run(L, s1, run);
        }

        template <class E, class A, class V1, class V2, class Stack, class NextFn>
        static ReturnType RunForEachArray(lua_State* L, Stack s, for_in_do_end_t<A, call_result<ipairs_t, E>, V1, V2> e, NextFn eip, NotFoundTag arr_pos)
        {
            return PushSingleExpressionToLuaStack(L, s, E(),
                                                  [](lua_State* L, auto s1)
                                                  {
                                                      return RunSentence(L, Stack(), for_in_do_end_t<A, call_result<ipairs_t, E>, V1, V2>(), ConstructStatelessLambda<NextFn>());
                                                  });
        }

        template <class E, class A, class V1, class V2, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, for_in_do_end_t<A, call_result<ipairs_t, E>, V1, V2> e, NextFn eip)
        {
            return RunForEachArray(L, s, e, eip, LuaStackFindFirst<Stack, E>());
        }

        template<class First, class...Rest, class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, sequence_t<First, Rest...> e, NextFn eip)
        {
            return RunSentence(L, s, First(), [](lua_State *L, auto s1) {
                auto s2 = CleanVolatileLuaStack(L, Stack(), s1);
                return RunSentence(L, s2, sequence_t<Rest...>(), ConstructStatelessLambda<NextFn>());
            });
        }
        template<class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, sequence_t<> e, NextFn eip)
        {
            return eip(L, s);
        }
        template<class Stack>
        static ReturnType ReturnVoid(lua_State* L, Stack s)
        {
            return 0;
        }
        template<class Stack, class...E, int ReturnNum = sizeof...(E)>
        static ReturnType ReturnValue(lua_State* L, Stack s, E...e)
        {
            return PushMultiExpressionsToLuaStack(L, s, [](lua_State *L, auto s1) -> int
            {
                return ReturnNum;
            }, e...);
        }
        template<class Stack>
        static ReturnType ThrowException(lua_State* L, Stack s)
        {
            constexpr int size = LuaStackSize<Stack>::value;
            static_assert(LuaStackFindFirst<Stack, exception_t>::value == size && size > 1, "exception must be on top of stack");
            const char* message = lua_tostring(L, (LuaStackFindFirst<Stack, exception_t>::value));
            lua_insert(L, 1);
            lua_pop(L, size - 1);
            return lua_error(L);
        }
        template<class Stack, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, return_void_t e, NextFn eip)
        {
            // ignore eip
            return ReturnVoid(L, s);
        }
        template<class Stack, class...E, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, return_value_t<E...> e, NextFn eip)
        {
            // ignore eip
            return ReturnValue(L, s, E()...);
        }

        template <class Stack, int N, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, local_decl_t<N> e, NextFn eip)
        {
            lua_pushnil(L);
            auto s1 = UpdateStackValPush(s, local_t<N>());
            return eip(L, s1);
        }

        template <class Stack, int N, class E, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, local_decl_init_t<N, E> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, E(), [](lua_State *L, auto s1)
            {
                auto s2 = UpdateStackValPop<1>(s1);
                auto s3 = UpdateStackValPush(s2, local_t<N>());
                return InvokeStatelessLambda<NextFn>(L, s3);
            });
        }

        template <class Stack, int N, class E, class NextFn>
        static ReturnType RunSentence(lua_State* L, Stack s, local_assign_t<N, E> e, NextFn eip)
        {
            return PushSingleExpressionToLuaStack(L, s, E(), [](lua_State *L, auto s1)
            {
                using FindResult = typename LuaStackFindFirst<Stack, local_t<N>>::type;
                static_assert(!std::is_same<FindResult, NotFoundTag>::value, "local variable was not decleared");

                lua_replace(L, (FindResult::value));
                auto s2 = UpdateStackValPop<1>(s1);

                return InvokeStatelessLambda<NextFn>(L, s2);
            });
        }

        template<class Sentence>
        static ReturnType Run(lua_State *L)
        {
            static_assert(is_sentence<Sentence>::value, "e requires to be a sentence");
            return RunSentence(L, LuaEmptyStack(), Sentence(), [](lua_State *L, auto s1){ return ReturnVoid(L, s1); });
        }
    };

    template<class T>
    auto PushValue(lua_State* L, const T& a)->decltype(Push(L, a), int())
    {
        return Push(L, a), 0;
    }

    template<int N>
    int PushValue(lua_State* L, function_arg_t<N>)
    {
        return lua_pushvalue(L, N), 0;
    }

    template <class... BoundArgs>
    int PushValues(lua_State* L, const BoundArgs&... a)
    {
        lua_checkstack(L, sizeof...(a));
        int b[] = { PushValue(L, a)... };
        return sizeof...(a);
    }

    inline int PushValues(lua_State* L)
    {
        return 0;
    }

    template <class Sentence, class... BoundArgs>
    void PushExpLambda(lua_State* L, Sentence e, const BoundArgs&... a)
    {
        lua_pushcclosure(L, LvmRunner::Run<Sentence>, PushValues(L, a...));
    }

    template<class Sentence, class...BoundArgs>
    int eval_r(lua_State *L, Sentence e, BoundArgs &&...a)
    {
        int ebp = lua_gettop(L);
        PushExpLambda(L, e, a...);
        lua_call(L, 0, LUA_MULTRET);
        int esp = lua_gettop(L);
        return esp - ebp;
    }

    template <class Ret = void, class ExceptionHandler, class Sentence, class... BoundArgs>
    Ret xpeval(lua_State* L, const ExceptionHandler &eh, Sentence e, BoundArgs&&... a)
    {
        PushValue(L, eh); // #1 = eh
        PushExpLambda(L, e, a...); // #2 = f
        int status = lua_pcall(L, 0, 1, -2); // #2 = ret
        if (status != LUA_OK)
        {
            // TODO : handle error
            lua_pop(L, 2); // #0
            return Ret(); // run with error
        }
        lua_remove(L, -2);     // #1 = ret
        // warning : may panic on return value conv
        return LuaReturnStack(L, 1, false).release<Ret>();
    }

    template<class Ret = void, class Sentence, class...BoundArgs>
    Ret eval(lua_State* L, Sentence e, BoundArgs &&...a)
    {
        PushExpLambda(L, e, a...);
        lua_call(L, 0, 1);
        return LuaReturnStack(L, 1, false).release<Ret>();
    }

    // alias
#define UPVALUE_T_DEF(Z, N, DATA) constexpr upvalue_t<BOOST_PP_INC(N)> BOOST_PP_CAT(_, BOOST_PP_INC(N));
    BOOST_PP_REPEAT(100, UPVALUE_T_DEF, ~)
#undef UPVALUE_T_DEF

#define LOCAL_T_DEF(Z, N, DATA) constexpr local_t<BOOST_PP_INC(N)> BOOST_PP_CAT(v, BOOST_PP_INC(N));
    BOOST_PP_REPEAT(100, LOCAL_T_DEF, ~)
#undef LOCAL_T_DEF

#define ARG_T_DEF(Z, N, DATA) constexpr function_arg_t<BOOST_PP_INC(N)> BOOST_PP_CAT(a, BOOST_PP_INC(N));
    BOOST_PP_REPEAT(100, ARG_T_DEF, ~)
#undef ARG_T_DEF

    constexpr global_t _G;
    constexpr registry_t _REF;
    constexpr placeholder_expression _;

    constexpr auto _require = _G[(string_constant_t<'r','e','q','u','i','r','e'>())];

    constexpr ipairs_t _ipairs;
    constexpr pairs_t _pairs;

    namespace detail
    {
        template <typename In, typename Ret = luash::string_constant_t<>>
        struct str_filter;
        template <char I, char... In, char... Ret>
        struct str_filter<string_constant_t<I, In...>, string_constant_t<Ret...>> : str_filter<string_constant_t<In...>, string_constant_t<Ret..., I>> {};
        template <char... In, char... Ret>
        struct str_filter<string_constant_t<0, In...>, string_constant_t<Ret...>> : std::enable_if<true, string_constant_t<Ret...>> {};
        template <char... Ret>
        struct str_filter<string_constant_t<>, string_constant_t<Ret...>> : std::enable_if<true, string_constant_t<Ret...>> {};
    }

#define LUAEXP_STR_ARRAY_AT(i, S) (i < sizeof(S) ? S[i] : '\0')
#define LUAEXP_STR_ARRAY_ITER(Z, i, S) BOOST_PP_COMMA_IF(i) LUAEXP_STR_ARRAY_AT(i, S)
#define LUAEXP_STR_IMPL(S, N) typename luash::detail::str_filter<luash::string_constant_t<BOOST_PP_REPEAT(N, LUAEXP_STR_ARRAY_ITER, S)>>::type
#define _S(S) (LUAEXP_STR_IMPL(S, 100)())
    static_assert(std::is_base_of<string_constant_t<'t', 'e', 's', 't'>, decltype(_S("test"))>::value, "test");
}