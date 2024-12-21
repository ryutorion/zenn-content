---
title: "std::functionの実装を読む - Microsoft STL編"
emoji: "✡️"
type: "tech"
topics: [ "cpp" ]
published: true
---

## はじめに
とある事情でstd::functionのようなクラスを作りたくなったので，そもそも既存の実装はどうなっているのか，というのを調べてみました．
この記事では，MicrosoftのSTL([https://github.com/microsoft/STL](https://github.com/microsoft/STL))を読んで理解したことをまとめています．

なお，ここでの仕様はC++17をベースとしています．

## C++標準のインターフェース

std::functionのインターフェースは仕様で次のように定義されています．

```cpp
namespace std
{
    template<class> class function; // not defined

    template<class R, class... ArgTypes>
    class function<R(ArgTypes...)>
    {
    public:
        using result_type = R;

        function() noexcept;

        function(nullptr_t) noexcept;

        function(const function&);

        function(function&&);

        template<class F>
        function(F);
        
        function& operator=(const function&);

        function& operator=(function&&);

        function& operator=(nullptr_t) noexcept;

        template<class F>
        function& operator=(F&&);

        template<class F>
        function& operator=(reference_wrapper<F>) noexcept;

        ~function();

        void swap(function&) noexcept;

        explicit operator bool() const noexcept;

        R operator()(ArgTypes...) const;

        const type_info& target_type() const noexcept;

        template<class T> T* target() noexcept;

        template<class T> const T* target() const noexcept;
    };

    template<class R, class... ArgTypes>
    function(R(*)(ArgTypes...)) -> function<R(ArgTypes...)>;

    template<class F>
    function(F) -> function<see below >;

    template <class R, class... ArgTypes>
    bool operator==(const function<R(ArgTypes...)>&, nullptr_t) noexcept;

    template <class R, class... ArgTypes>
    bool operator==(nullptr_t, const function<R(ArgTypes...)>&) noexcept;

    template <class R, class... ArgTypes>
    bool operator!=(const function<R(ArgTypes...)>&, nullptr_t) noexcept;

    template <class R, class... ArgTypes>
    bool operator!=(nullptr_t, const function<R(ArgTypes...)>&) noexcept;

    template <class R, class... ArgTypes>
    void swap(function<R(ArgTypes...)>&, function<R(ArgTypes...)>&) noexcept;
}
```

## Microsoft STLのインターフェース

[Microsoft STL](https://github.com/microsoft/STL)の実装は次のようになっています．

```cpp
namespace std
{
    template <class _Fty>
    class function : public _Get_function_impl<_Fty>::type
    {
        // ...
    };
}
```

この_Get_function_impl<_Fty>は，_Ftyが関数かどうかを判定するようです．
大雑把には，関数以外，関数(noexcept無し)，関数(noexcept有り)の3つに分かれていました．

```cpp
// 関数でない場合の実装
template <class _Tx>
struct _Get_function_impl {
    static_assert(_Always_false<_Tx>, "std::function only accepts function types as template arguments.");
};

// 関数向けの実装
#define _GET_FUNCTION_IMPL(CALL_OPT, X1, X2, X3)                                                  \
    template <class _Ret, class... _Types>                                                        \
    struct _Get_function_impl<_Ret CALL_OPT(_Types...)> { /* determine type from argument list */ \
        using type = _Func_class<_Ret, _Types...>;                                                \
    };

_NON_MEMBER_CALL(_GET_FUNCTION_IMPL, X1, X2, X3)
#undef _GET_FUNCTION_IMPL

// C++17でnoexceptが関数の型の一部になった関係で
// 機能テストマクロでチェックした上でnoexceptな関数を弾いている
// ref. https://cpprefjp.github.io/lang/cpp17/exception_spec_be_part_of_the_type_system.html
#ifdef __cpp_noexcept_function_type
#define _GET_FUNCTION_IMPL_NOEXCEPT(CALL_OPT, X1, X2, X3)                                                         \
    template <class _Ret, class... _Types>                                                                        \
    struct _Get_function_impl<_Ret CALL_OPT(_Types...) noexcept> {                                                \
        static_assert(                                                                                            \
            _Always_false<_Ret>, "std::function does not accept noexcept function types as template arguments."); \
    };
_NON_MEMBER_CALL(_GET_FUNCTION_IMPL_NOEXCEPT, X1, X2, X3)
#undef _GET_FUNCTION_IMPL_NOEXCEPT
#endif // defined(__cpp_noexcept_function_type)
```

_GET_FUNCTION_IMPLは，CALL_OPTのみ利用し，それ以外は無視するような関数形式マクロになっています．
その中では，関数の場合の_Get_function_implの特殊化を行っています．
_GET_FUNCTION_IMPL_NOEXCEPTも同様で，こちらは関数にnoexceptが付いている場合の特殊化に対応しています．
コメントにも書いたように，C++17からの仕様変更に伴うもののようです．

```cpp
#define _NON_MEMBER_CALL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT) \
    _EMIT_CDECL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT)          \
    _EMIT_CLRCALL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT)        \
    _EMIT_FASTCALL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT)       \
    _EMIT_STDCALL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT)        \
    _EMIT_VECTORCALL(FUNC, CV_OPT, REF_OPT, NOEXCEPT_OPT)
```

それぞれ[実引数の渡し方に関する規約](https://learn.microsoft.com/ja-jp/cpp/cpp/argument-passing-and-naming-conventions?view=msvc-170)にそって関数を分類します．最終的に，次のようになるようです．

```cpp
// 常に定義される
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __cdelc (Types...)> { /* ... */ };
// コンパイル時に/clrを指定した場合にのみ定義される
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __clrcall (Types...)> { /* ... */ };
// /clrが指定されていなくてx86の場合に定義される (x64およびARMでは無し)
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __fastcall (Types...)> { /* ... */ };
// x86の場合にのみ定義される
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __stdcall (Types...)> { /* ... */ };
// /clrの場合は定義されない
// x86の場合はSSE2以上が利用できる場合に定義される
// X64かつARM64ECで無い場合にも定義される
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __vectorcall (Types...)> { /* ... */ };
```

つまり，呼び出し規約によって型に一致するかが変わるので分けたうえで，全てを同じtypeに落とし込んでいるわけです．

```cpp
template <class _Ret, class... _Types>
struct _Get_function_impl<_Ret __cdelc (Types...)>
 {
    using type = _Func_class<_Ret, _Types...>;
 };
```

次に，この_Func_classが何なのか見てみます．

```cpp
template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
    // ...
};
```

どうやら，今度は_Arg_typesというクラスを継承しています．まずはこれを見てみましょう．

```
template <class... _Types>
struct _Arg_types {}; // sizeof...(_Types)が1か2の場合にargument_typeなどを提供

// _Typesが1つの場合にはargument_typeを提供
template <class _Ty1>
struct _Arg_types<_Ty1> {
    using _ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty1;
};

// _Typesが2つの場合にはfirst_argument_type/second_argument_typeを提供
template <class _Ty1, class _Ty2>
struct _Arg_types<_Ty1, _Ty2> {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS  = _Ty1;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty2;
};
```

どうやら，C++17から非推奨でC++20からは消えるargument_type/first_argument_type/second_argument_typeを定義するためのクラスのようです．

## コンストラクタ

```cpp
    // 標準と同じ    
    function() noexcept {}

    // 標準と同じ
    function(nullptr_t) noexcept {}

    // 標準と同じコピーコンストラクタ
    function(const function& _Right) {
        this->_Reset_copy(_Right);
    }

    // 標準のいくつかをまとめている
    // function(function&& f);
    // template <class F> function(F f);
    template <class _Fx, typename _Mybase::template _Enable_if_callable_t<_Fx, function> = 0>
    function(_Fx&& _Func) {
        this->_Reset(_STD forward<_Fx>(_Func));
    }

// C++17以降消える
#if _HAS_FUNCTION_ALLOCATOR_SUPPORT
    template <class _Alloc>
    function(allocator_arg_t, const _Alloc&) noexcept {}

    template <class _Alloc>
    function(allocator_arg_t, const _Alloc&, nullptr_t) noexcept {}

    template <class _Alloc>
    function(allocator_arg_t, const _Alloc& _Ax, const function& _Right) {
        this->_Reset_alloc(_Right, _Ax);
    }

    template <class _Fx, class _Alloc, typename _Mybase::template _Enable_if_callable_t<_Fx, function> = 0>
    function(allocator_arg_t, const _Alloc& _Ax, _Fx&& _Func) {
        this->_Reset_alloc(_STD forward<_Fx>(_Func), _Ax);
    }
#endif // _HAS_FUNCTION_ALLOCATOR_SUPPORT
```

C++17では削除されたアロケータ指定タイプは無視するとして，まずは任意の関数を受け取るコンストラクタを見てみます．

```cpp
template <class _Fty>
class function : public _Get_function_impl<_Fty>::type
{
    using _Mybase = typename _Get_function_impl<_Fty>::type;

public:
    template <class _Fx, typename _Mybase::template _Enable_if_callable_t<_Fx, function> = 0>
    function(_Fx&& _Func) {
        this->_Reset(_STD forward<_Fx>(_Func));
    }
};
```

まず，_Enable_if_callable_tを確認します．

```cpp
template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
protected:
    template <class _Fx, class _Function>
    using _Enable_if_callable_t = enable_if_t<
        conjunction_v<
            negation<
                is_same<_Remove_cvref_t<_Fx>, _Function>
            >,
            _Is_invocable_r<_Ret, decay_t<_Fx>&, _Types...>
        >,
        int
    >;
};
```

_Functionにはfunctionが設定されているので，_Fxからvolatile/constや参照を外して同じ型の関数を受け取るfunctionだった場合はis_sameがtrueになり，negationで反転してfalseになり，conjunctionはfalseになります．よって，enable_if_tは存在しません．

次に，_Is_invocable_rは，decay_t<_Fx>&と_Types...で関数呼び出しが可能で，その返り値の型が_Retに変換できる場合にtrueになります．
つまり，function型以外で呼び出し可能な型の場合にenable_if_tが存在する，というSFINAEになっているわけです．

続いて，Resetの中身を見てみます．Resetは，_Func_classに定義されています．

```cpp
template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
    template <class _Fx>
    void _Reset(_Fx&& _Val) { // _Valのコピーを格納する
        if (!_STD _Test_callable(_Val)) { // メンバーポインタ/関数ポインタ/std::functionがnull
            return; // 既に空
        }

        using _Impl = _Func_impl_no_alloc<decay_t<_Fx>, _Ret, _Types...>;
        if constexpr (_Is_large<_Impl>) {
            // _Valを動的割り当て
            _Set(_STD _Global_new<_Impl>(_STD forward<_Fx>(_Val)));
        } else {
            // store _Val in-situ (in-situはラテン語でその場)
            // その場に_Valを格納する
            _Set(::new (static_cast<void*>(&_Mystorage)) _Impl(_STD forward<_Fx>(_Val)));
        }
    }
};
```

まずは，_STDは標準ライブラリの名前空間 std をラップしています．そして，_Test_callable(_Val)について見てみます．

```cpp
template <class _Ty>
bool _Test_callable(const _Ty& _Arg) noexcept { // determine whether std::function must store _Arg
    if constexpr (_Testable_callable_v<_Ty>) {
        return !!_Arg;
    } else {
        return true;
    }
}
```

実行時は常にtrueなので，_Resetの最初のreturnに到達することはありません．
_Testable_callbable_v<_Ty>がtrueの場合，!!_Argによって，空の場合にfalseに変換された結果，returnに到達します．

ここで気になるのが，`既に空`，というコメントです．
全体を見ると，_Resetが呼ばれるのはコンストラクタか，代入演算子で一度空にしてからなので，このようなコメントなのでしょう．

で，この_Testable_callable_vはというと，次のような実装になっていました．

```cpp
template <class _Ty>
constexpr bool _Testable_callable_v = disjunction_v<
    is_pointer<_Ty>,
    _Is_specialization<_Ty, function>,
    is_member_pointer<_Ty>
>;
```

既に呼び出し可能な型であることは分かっているので，後はポインタか，function型の特殊化か，メンバーポインタかだけチェックしているようです．

続いて，_Func_impl_no_allocを見てみます．

```cpp
using _Impl = _Func_impl_no_alloc<decay_t<_Fx>, _Ret, _Types...>;

template <class _Callable, class _Rx, class... _Types>
class _Func_impl_no_alloc final : public _Func_base<_Rx, _Types...>
{
    // ...
};
```

どうやら，更に_Func_baseというクラスを見る必要がありそうです．

```cpp
template <class _Rx, class... _Types>
class __declspec(novtable) _Func_base { // 実装型のための抽象基底
    // ...
};
```

[novtable](https://learn.microsoft.com/ja-jp/cpp/cpp/novtable?view=msvc-170)は，どうやらインターフェースクラスに付けるとvtableの生成をしないようにして，コードサイズを削減してくれるものだそうです．vtableポインタそのものを消すわけではないようなので注意が必要なようです．これ以上は脱線するので触れません．

ただ，この_Func_base，C++17で消えるアロケータ指定有りのfunctionのために，_Func_impl_no_allocと_Func_implを分けるためにあるようで，C++17以降は無くても良いのでは，という気がします．

さて，次はこの_Func_impl_no_allocのサイズが大きい場合に動的割り当てに入るようです．

```cpp
using _Impl = _Func_impl_no_alloc<decay_t<_Fx>, _Ret, _Types...>;
if constexpr (_Is_large<_Impl>) {
    // _Valを動的割り当て
    _Set(_STD _Global_new<_Impl>(_STD forward<_Fx>(_Val)));
}
```

ただ，何を基準に大きいと判断しているのでしょう?
_Implのサイズが何を基準に決まるのかと，_Is_large<_Impl>が何と比較しているのか見てみます．

```cpp
// std::functionとstd::anyのポインタの数 (std::stringのデバッグビルドより3つくらい多い)
// 64ビット環境では8
_INLINE_VAR constexpr int _Small_object_num_ptrs = 6 + 16 / sizeof(void*);

// 64ビット環境では7 * 8で56
_INLINE_VAR constexpr size_t _Space_size = (_Small_object_num_ptrs - 1) * sizeof(void*);

template <class _Impl> // _Implが動的割り当てを必要とするか決める
constexpr bool _Is_large =
    sizeof(_Impl) > _Space_size                 // (64ビット環境なら)56バイト以上か
    || alignof(_Impl) > alignof(max_align_t)    // アライメントが8バイト以上か
    || !_Impl::_Nothrow_move::value;            // _Nothrow_moveじゃないか
```

最後の_Nothrow_moveは更に中身を見てみます．

```cpp
template <class _Callable, class _Rx, class... _Types>
class _Func_impl_no_alloc final : public _Func_base<_Rx, _Types...>
 {
public:
    using _Nothrow_move = is_nothrow_move_constructible<_Callable>;
 };
```

単純にmoveコンストラクタがnoexceptで無ければ，ということのようです．
ちなみに，`is_nothrow_move_constructible_v<int()>==0`のように関数型を直接渡すと常に割り当てが必要ってことになるので，途中に何度か登場したdecayが使われているようです．

では，_Implのサイズが56バイトを超えるときというのはどういう場合かというと，例えば64ビット環境でstd::vectorを2つほどキャプチャするとそれくらいのサイズになります．

```cpp
std::function<void()> func = [
	a = std::vector<int>(1000),
	b = std::vector<int>(1000)
]()
{};
```

そして，56バイトな理由は後で出てきます．

_Global_newはnewと見なせばよいようです．何故forwardなのかについては，パーフェクトフォワードなどを検索してみてください．

```cpp
_Set(_STD _Global_new<_Impl>(_STD forward<_Fx>(_Val)));

template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
private:
    using result_type = _Ret;
    using _Ptrt = _Func_base<_Ret, _Types...>;

    union _Storage { // 小さいオブジェクトのためのストレージ (basic_stringは小さい)
        max_align_t _Dummy1; // 最大アライメントのため
        char _Dummy2[_Space_size]; // エイリアシング許可のため
        _Ptrt* _Ptrs[_Small_object_num_ptrs]; // _Ptrs[_Small_object_num_ptrs - 1]は予約されている
    };

    _Storage _Mystorage;

    void _Set(_Ptrt* _Ptr) noexcept { // オブジェクトへのポインタを格納する
        // _Mystorage._Ptrsの最後に保存する
        _Mystorage._Ptrs[_Small_object_num_ptrs - 1] = _Ptr;
    }
};
```

_Setは，_Storageという共用体の_Ptrtのポインタ配列の最後にポインタを書き込むだけのようです．
_Ptrtは_Func_base型のようですが，C++17以降は実質_Func_impl_no_alloc型ですね．

続いて，サイズが小さい場合の側を見てみると，_Mystorageの先頭にplacement newで_Func_impl_no_alloc型を構築して，
その先頭アドレスを_Mystorageの最後に格納しています．

```cpp
// store _Val in-situ (in-situはラテン語でその場)
// その場に_Valを格納する
_Set(::new (static_cast<void*>(&_Mystorage)) _Impl(_STD forward<_Fx>(_Val)));
```

つまり，56バイトよりも大きい場合はnewを使うとなっていたのは，_Storageの_Ptrsの最後はポインタの格納に使って，その前の56バイトに構築するからだったのです．

さて，ここまでで，実際に関数や関数オブジェクト，lambdaを保持しているのは_Func_impl_no_allocということになるので，そちらがどうやって関数を保持しているのか見てみます．

```cpp
template <class _Callable, class _Rx, class... _Types>
class _Func_impl_no_alloc final : public _Func_base<_Rx, _Types...>
{
public:
    template <class _Other, enable_if_t<!is_same_v<_Func_impl_no_alloc, decay_t<_Other>>, int> = 0>
    explicit _Func_impl_no_alloc(_Other&& _Val) : _Callee(_STD forward<_Other>(_Val)) {}

private:
    _Callable _Callee;
};
```

_enable_if_tの部分は，先ほど似たような書き方を見ました．_Func_impl_no_allocそのものを渡されるケースを避けているようです．

あとは，_Calleeに値を保存しているだけですが，_Calleeは渡された関数ポインタまたは関数オブジェクトかlambdaなので，実質そのまま保存しているだけになります．

イメージとしては次のような感じです．

```cpp
// 56バイト以下の場合 _Ptrs[0]から_Ptrs[6]までの範囲に関数ポインタまたは関数オブジェクトやlambdaが格納される
// |- _Storage -|
// |- _Ptrs[0] -| <--|
// |- _Ptrs[1] -|    |
// |- _Ptrs[2] -|    |
// |- _Ptrs[3] -|    |
// |- _Ptrs[4] -|    |
// |- _Ptrs[5] -|    |
// |- _Ptrs[6] -|    |
// |- _Ptrs[7] -| ---|
// |------------|
//
// 56バイトより大きい場合，他の領域を指している
// |-- _Storage --|
// |-- _Ptrs[0] --|
// |-- _Ptrs[1] --|
// |-- _Ptrs[2] --|
// |-- _Ptrs[3] --|
// |-- _Ptrs[4] --|
// |-- _Ptrs[5] --|
// |-- _Ptrs[6] --|
// |-- _Ptrs[7] --| ---|
// |--------------|    |
//                     |
// |- _Func_impl -| <--|
```

## 関数呼び出し

次に，格納した関数を呼び出す部分を見てみます．関数の呼び出しは，functionではなく，継承元の_Func_classの方に実装されています．

```cpp
template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
public:
    _Ret operator()(_Types... _Args) const {
        if (_Empty()) {
            _Xbad_function_call();
        }
        const auto _Impl = _Getimpl();
        return _Impl->_Do_call(_STD forward<_Types>(_Args)...);
    }
};
```

ここで_Emptyと_Getimplは次のような実装になっていました．

```cpp
template <class _Ret, class... _Types>
class _Func_class : public _Arg_types<_Types...>
{
protected:
    bool _Empty() const noexcept {
        return !_Getimpl();
    }

    using _Ptrt = _Func_base<_Ret, _Types...>;
    _Ptrt* _Getimpl() const noexcept { // オブジェクトへのポインタを取得
        return _Mystorage._Ptrs[_Small_object_num_ptrs - 1];
    }
};
```

_Emptyは単純にポインタがnullかどうかをチェックしているだけで，_Getimplは_Storageの最後に格納している_Func_impl_no_allocへのポインタを返しているだけです．

というわけで，あとは_Do_callをチェックすれば関数呼び出しに到達できそうです．

```cpp
template <class _Rx, class... _Types>
class __declspec(novtable) _Func_base { // 実装型のための抽象基底
public:
    virtual _Rx _Do_call(_Types&&...) = 0;
};

template <class _Callable, class _Rx, class... _Types>
class _Func_impl_no_alloc final : public _Func_base<_Rx, _Types...>
{
private:
    _Rx _Do_call(_Types&&... _Args) override { // call wrapped function
        if constexpr (is_void_v<_Rx>) {
            (void) _STD invoke(_Callee, _STD forward<_Types>(_Args)...);
        } else {
            return _STD invoke(_Callee, _STD forward<_Types>(_Args)...);
        }
    }
};
```

_Func_baseにあるpublicな純粋仮想関数_Do_callを，_Func_impl_no_allocではprivateでオーバーライドしています．
privateでオーバーライドしても呼び出せることって意外と知らなかったりします．

で，中では戻り値の型がvoidかどうかをチェックして，voidの場合はreturnを呼び出さないようにした上で，std::invokeを呼び出しています．

std::invokeの中まで追うと若干長くなりそうなので，functionに関して読むのはここまでにしました．

## まとめ

Microsoft STLのstd::functionは，関数ポインタおよび，64bit環境では56バイトまでの関数(ただしアライメントが8より大きい場合は除く)の場合，
メモリ割り当てなしで使えるものの，std::functionのオブジェクトそのものは64バイトになることが分かりました．

ゲーム開発の場合，少しでもメモリの使用量を減らしたかったりするので，例えばサイズ指定可能なfixed_size_functioonとかあっても良いんじゃないかな，と．

また，C++17より前の実装を維持するために仮想関数になっている部分を消せば，実は少し早くなったりしないだろうか，という気がします．
この辺は互換性を維持するためでもあるでしょうから，大変ですね．
ちらっとC++23から入るmove_only_functionの実装を見てみたところ，そちらは仮想関数を使わない実装になっているようでした．

そのうち時間が出来たら，move_only_functionの実装を読んで理解したことや，Clang/GCCの実装も読んでまとめたいと思います．