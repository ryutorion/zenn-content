---
title: "C++ overload pattern - オーバーロードパターン"
emoji: "✡️"
type: "tech"
topics: [ "cpp" ]
published: true
---

# オーバーロードパターン

[2 Lines Of Code and 3 C++17 Features - The overload Pattern](https://www.cppstories.com/2019/02/2lines3featuresoverload.html/)という記事を見つけて，初めてオーバーロードパターンという言葉を耳にしました．
気になったので調べてみると，[stack overflowにあった質問](https://stackoverflow.com/questions/66890356/c-overload-pattern-call-resolution-with-mutable-lambda)では，良く知られたパターンとあります．

しかし，日本語で検索してみても通常のオーバーロードが引っ掛かるばかりで日本語での記事は無さそうだったのでまとめてみました．もし日本語では別の名前で普及しているよ，という話があれば教えていただけると幸いです．

:::message
2023/12/1
yohhoyさんに日本語でも名前はついてないけれど紹介されている記事があると紹介していただいたので，参考資料に追加しています．
:::

簡単に言うと，複数のlambdaをオーバーロードとしてまとめるためのパターンのようです．

```cpp
template <class... Ts> struct overload : Ts... { using Ts::operator()...; }
// C++17では次の型推論のガイドが必要
template <class... Ts> overload(Ts...) -> overload<Ts...>;
```

これは次のように使います．

```cpp
#include <iostream>

using namespace std;

template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

int main()
{
    auto print = overload {
        [](int)    { cout << "int" << endl; },
        [](double) { cout << "double" << endl; }
    };
    print(1);
    print(1.0);
}
```
[Compiler Explorerへのリンク](https://godbolt.org/z/6rMn3oxse)

lambdaがoperator()を実装したクラスを生成するので，それを継承したクラスでそのoperator()を使えるようにしてオーバーロードとしてまとめる，というパターンのようです．

これを何に使うのかというと，型安全でunionの代わりになるように作られたstd::variantと，そのvariantが保持している値にアクセスするstd::visitを使う際によく使われるようです．

```cpp
#include <iostream>
#include <variant>

using namespace std;

template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

int main()
{
    using value_t = std::variant<int, double>;

    value_t v1 = 1;
    value_t v2 = 2.0;

    auto print = overload {
        [](int)    { cout << "int" << endl; },
        [](double) { cout << "double" << endl; }
    };
    visit(print, v1);
    visit(print, v2);
}
```

[Compiler Explorerへのリンク](https://godbolt.org/z/YnKejfb35)

std::variantを使う機会が無かったのですが，なかなか面白そうなので，職場で使えそうな部分が無いかなって探してしまいそうです．

# 参考資料
- [2 Lines Of Code and 3 C++17 Features - The overload Pattern](https://www.cppstories.com/2019/02/2lines3featuresoverload.html/)
- [C++ overload pattern : call resolution with mutable lambda (stack overflow)](https://stackoverflow.com/questions/66890356/c-overload-pattern-call-resolution-with-mutable-lambda)
- [Smart Tricks with Parameter Packs and Fold Expressions](https://www.modernescpp.com/index.php/smart-tricks-with-fold-expressions/)
- [Visiting a std::variant with the Overload Pattern](https://www.modernescpp.com/index.php/visiting-a-std-variant-with-the-overload-pattern/)
- [Overload pattern - what's wrong with it and how not to use it?](https://elegant-cpp.com/overload-pattern-whats-wrong-with-it-and-how-not-to-use-it/)
- [Visiting a std::variant safely](https://andreasfertig.blog/2023/07/visiting-a-stdvariant-safely/)

- [ラムダ式を継承](https://osyo-manga.hatenadiary.org/entry/20121224/1356320541)
- [ラムダ式をオーバーロードする](https://kenkyu-note.hatenablog.com/entry/2019/09/01/040017)
- [ラムダ式のオーバーロード](https://yohhoy.hatenadiary.jp/entry/20200715/p1)
