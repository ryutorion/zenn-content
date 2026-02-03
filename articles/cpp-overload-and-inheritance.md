---
title: "C++のオーバーロードと継承のあれこれ"
emoji: "✡️"
type: "tech"
topics: [ "cpp" ]
published: true
---

C++のオーバーロードと継承の関係でちょっと変なコードを書かざるを得なかったので，記録に残します．

```cpp
// https://godbolt.org/z/5vh4zMoPe
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
};

int main()
{
    Base b;
    b.f(0);   // => int32_t
    b.f(0.0); // => double
}
```

オーバーロードは，同じ名前だけれど実引数の型によって呼び出す関数を分けられる便利な機能です．
もちろん，次のように継承しても使えます．

```cpp
// https://godbolt.org/z/f1hjqdsa3
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
};

struct Derived : public Base
{
};

int main()
{
    Derived b;
    b.f(0);   // int32_t
    b.f(0.0); // => double
}
```

しかし，C++の場合，継承先でオーバーロードを増やすと，親のメンバーを呼べなくなります．

```cpp
// https://godbolt.org/z/6Wrr9MKex
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
};

struct Derived : public Base
{
    void f(float) { cout << "float" << endl; }
};

int main()
{
    Derived b;
    b.f(0);   // => float
    b.f(0.0); // => float
}
```

親のメンバーも呼べるようにするための方法として，usingで持ってくる方法があります．

```cpp
// https://godbolt.org/z/zhv6qo7dz
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
};

struct Derived : public Base
{
    using Base::f;
    void f(float) { cout << "float" << endl; }
};

int main()
{
    Derived b;
    b.f(0);    // => int32_t
    b.f(0.0);  // => double
    b.f(0.0f); // => float
}
```

しかし，privateなオーバーロードが混ざっていると，usingで持ってくる方法が使えなくなります．

```cpp
// https://godbolt.org/z/Ees781Ez3
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
private:
    void f(const char*) { cout << "const char*" << endl; }
};

struct Derived : public Base
{
    using Base::f;
    void f(float) { cout << "float" << endl; }
};

int main()
{
    Derived b;
    b.f(0);
    b.f(0.0);
    b.f(0.0f);
}
```

仕方なく，次のようにオーバーロードを全て用意して，継承元のメンバーを呼び出す形にしました．

```cpp
// https://godbolt.org/z/7WKx38svq
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
private:
    void f(const char*) { cout << "const char*" << endl; }
};

struct Derived : public Base
{
    void f(int32_t v) { Base::f(v); }
    void f(double v) { Base::f(v); }
    void f(float) { cout << "float" << endl; }
};

int main()
{
    Derived b;
    b.f(0);    // => int32_t
    b.f(0.0);  // => double
    b.f(0.0f); // => float
}
```

usingで持ってこれるものだけ持ってくる，みたいにしてくれれば楽なのにな，とは思ったものの，取り合えず目的は達成できました．
そもそもこんな書き方をするなと言われればそれまでですが，こうせざるを得ない事情があるのです．

何かもっと良い書き方があればコメントをお願いします．

2026/02/03 追記
[yohhoy生成からコメントいただきました](https://x.com/yohhoy/status/2018674363084497263?s=20)

variadic templateとforward使えば良いよね，と．  
確かにその通りですね．

```cpp
// https://godbolt.org/z/1KoYb4zxK
#include <cstdint>
#include <iostream>

using namespace std;

struct Base
{
    void f(int32_t) { cout << "int32_t" << endl; }
    void f(double) { cout << "double" << endl; }
private:
    void f(const char*) { cout << "const char*" << endl; }
};

struct Derived : public Base
{
    template <class... Args>
    auto f(Args&&... args)
    {
        Base::f(std::forward<Args>(args)...);
    }
//  using Base::f;
    void f(float) { cout << "float" << endl; }
};

int main()
{
    Derived b;
    b.f(0);
    b.f(0.0);
    b.f(0.0f);
//    b.f("");
}
```