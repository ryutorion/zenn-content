---
title: "ゲームプログラミングのための数学 - ベクトル"
emoji: "✡️"
topics: [ "cpp", "math" ]
published: true
type: "tech"
---

# はじめに

プログラマの視点で言えば，ベクトルは$n$個の浮動小数点数型のメンバ変数を持つ
構造体になります．$n$個のメンバ変数は，$x$，$y$，$z$のような名前を付けることもあれば，
配列にすることもあります．

それぞれのメンバ変数を成分(component)と言います．
成分の数が$n$の場合，$n$次元(dimension)のベクトルと言います．
つまり，3D(3-dimension)のベクトルは成分が3つのベクトル，ということになります．

ゲームプログラミングで扱うベクトルは，成分が2つ，3つ，4つの場合が一般的です．
ここでは，3次元ベクトルを取り上げて，C++のコードを交えつつ説明してみます．

:::message alert
ここでの実装は，こんな実装もあり得るというだけで，正解というわけではありません．
また，C++をある程度理解している前提とします．
:::

成分が3つなので，データ構造は次のようになります．

```cpp
struct Vector3
{
    float x;
    float y;
    float z;
};
```

一般的にはメンバ変数はprivateにすべきなのです．
しかし，ベクトルの場合は数学的に定義が決まっていて，隠ぺいする意味がありません．
いちいちアクセッサを経由する方が面倒です．

変更するとすれば，floatをdoubleにするくらいでしょうか．

全てpublicで良いので，classにもせず，structで実装します．

# コンストラクタ

まずは，デフォルトコンストラクタを実装します．

```cpp
struct Vector3
{
    Vector3() noexcept = default;
};
```

分かりやすくするために，こんな感じで既に出た実装は省略して，
メンバ関数であればVecotr3の中に，それ以外はVector3を書かずにコードを書きます．

ここでは，デフォルトコンストラクタは何もしないことにします．
また，何もしないので例外も出ないということでnoexceptも指定しておきます．

デフォルトコンストラクタの挙動は，各成分を0で初期化するか，
何もしないかで判断が分かれるところです．

0で初期化し忘れていて，メモリ上に偶然あった値を参照してしまい，
結果としてバグになってしまう，ということもよくあります．

しかし，大量のVector3のデータを扱う場合に，
どうせ読み込んだデータで上書きするのに0初期化が入るとすると，
結構な量の無駄な処理をすることになります．

そのため，ここでは何もしないことを選びました．
例えば，有名なUnreal Engine 4にあるFVectorも，
デフォルトコンストラクタでは何もしないことを選択しています．

次に，各成分の値を渡して，それぞれの成分を初期化するコンストラクタを実装します．

```cpp
struct Vector3
{
    constexpr Vector3(float vx, float vy, float vz) noexcept
        : x(vx)
        , y(vy)
        , z(vz)
    {}
};
```

今のところ，constexprにすることで圧倒的なメリットがある，というわけではないのですが，
できるなら今のうちから対応しておいた方が良いだろう，という理由でconstexprにしています．
単純にfloatの値を代入するだけなので例外も起きないはずなので，noexceptも指定します．

最後に，各成分を同じ値で初期化するコンストラクタを用意します．
例えば，0で初期化したい場合に3回も0を書くのは面倒です．
そういった場合に重宝します．

```cpp
struct Vector3
{
    explicit constexpr Vector3(float v) noexcept
        : x(v)
        , y(v)
        , z(v)
    {}
};
```

explicitを忘れてはいけません．
そうしないと，意図せずfloatの値がVector3に変換されるかもしれません．

# 逆ベクトル

ベクトル$v$と逆方向のベクトルを表すのに，$-v$と書きます．
$v=(v_x,v_y,v_z)$とすると，$-v=(-v_x,-v_y,-v_z)$となります．

これを実装すると，次のようになります．
```cpp
inline constexpr const Vector3 operator-(const Vector3 & v) noexcept
{
    return Vector3(-v.x, -v.y, -v.z);
}
```

# 加算

ベクトル$u=(u_x,u_y,u_z)$とベクトル$v=(v_x,v_y,v_z)$の加算は，
成分別の和を成分として持つベクトルになります．
つまり，$u + v = (u_x + v_x, u_y + v_y, u_z + v_z)$です．

これを実装すると，次のようになります．
```cpp
struct Vector3
{
    Vector3 & operator+=(const Vector3 & v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }
};

inline constexpr const Vector3 operator+(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
```

加算代入は，通常の変数への加算代入と同じように変数への参照が返るようにするため，
メンバ変数として実装します．
加算は，組み込み型による加算と同じように，変更できない一時オブジェクトを返すだけなので，
通常の関数として実装します．

constexprやconst，noexceptを忘れないようにします．

# 減算

減算の実装については，加算の演算子を減算の演算子に置き換えるだけなので，
説明は省きます．

```cpp
struct Vector3
{
    Vector3 & operator-=(const Vector3 & v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }
};

inline constexpr const Vector3 operator-(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
```

# 乗算

ベクトルの乗算には色々な種類がありますが，ここではスカラーとの積を考えます．
ベクトル$v = (v_x,v_y,v_z)$に対して，スカラー$s$があったとすると，
$s * v = (s * v_x, s * v_y, s * v_z)$，$v * s = (v_x * s, v_y * s, v_z * s)$のように，
スカラーの積はそれぞれの成分にスカラーを掛けたものになります．

これを実装すると，次のようになります．
```cpp
struct Vector3
{
    Vector3 & operator*=(const float s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;

        return *this;
    }
};

inline constexpr const Vector3 operator*(const Vector3 & v, const float s) noexcept
{
    return Vector3(v.x * s, v.y * s, v.z * s);
}

inline constexpr const Vector3 operator*(const float s, const Vector3 & v) noexcept
{
    return Vector3(s * v.x, s * v.y, s * v.z);
}
```

紙の上では，スカラーは右にも左にも掛けられます．
同じことをプログラム上で実現しようとすると，仮引数の順序を考慮して，
2種類用意する必要があります．

また，成分別に積ができると便利なことが多々あるので，成分別の積も実装します．
```cpp
struct Vector3
{
    Vector3 & operator*=(const Vector3 & v) noexcept
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }
};

inline constexpr const Vector3 operator*(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}
```

# 除算

```cpp
struct Vector3
{
    Vector3 & operator/=(const float s)
    {
        return *this *= (1.0f / s);
    }
};

inline constexpr const Vector3 operator/(const Vector3 & v, const float s)
{
    return v * (1.0f / s);
}
```

# ドット積

あるベクトル$\mathbf{u}$とあるベクトル$\mathbf{v}$の成分同士の積の和を内積，
またはドット積と言います．

ドット積という理由は，$\mathbf{u} \cdot \mathbf{v}$のように，ドットを使って表すからです．

式で書くと，次のようになります．

$$\mathbf{u} \cdot \mathbf{v} = u_x * v_x + u_y * v_y + u_z * v_z$$

```cpp
struct Vector3
{
    constexpr float dot(const Vector3 & v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z;
    }
};
```

# 長さ

あるベクトル$\mathbf{v}$の長さを$|\mathbf{v}|$と書きます．

ベクトルの長さは，それぞれの成分の2乗の和の平方根になります．
つまり，ベクトル$\mathbf{v} = (v_x,v_y,v_z)$があったとすると，
次のような式になります．

$$|\mathbf{v}|=\sqrt{v_x * v_x + v_y * v_y + v_z * v_z}$$

これは，ドット積を使って次のような式で表せます．

$$|\mathbf{v}|=\sqrt{\mathbf{v} \cdot \mathbf{v}}$$

そのため，ベクトルの長さを返すメンバ関数の実装は次のようになります．

```cpp
#include <cmath>

struct Vector3
{
    float length() const
    {
        return std::sqrt(dot(*this));
    }
};
```

std::sqrtはconstexprでないため，必然的にlength関数もconstexprではなくなります．

# ドット積と長さと角度

あるベクトル$\mathbf{u}$と$\mathbf{v}$の間の角度を$\theta$とすると，次の式が成り立ちます．

$$\mathbf{u} \cdot \mathbf{v} = |\mathbf{u}||\mathbf{v}|\cos\theta$$

ここから，$cos\theta$について整理すると，次のような式が導けます．

$$\cos\theta = \frac{\mathbf{u} \cdot \mathbf{v}}{|\mathbf{u}||\mathbf{v}|}$$

更に，$\cos\theta$の値$x$から$\theta$を求める$\cos^{-1}x$を利用すると，次のような式が求まる．

$$\theta = \cos^{-1}\frac{\mathbf{u} \cdot \mathbf{v}}{|\mathbf{u}||\mathbf{v}|}$$

これを実際のコードに落とし込むと，次のようになります．
```cpp
float theta = acos(u.dot(v) / (u.length() * v.length()));
```

acos($\theta$)は$\cos^{-1}\theta$に対応する関数です．

# 単位ベクトル

長さが1のベクトルを単位ベクトル(unit vector)と言います．
また，あるベクトル$\mathbf{u}$を次のような式で単位ベクトル$\mathbf{\hat{u}}$にすることを，
正規化(normalize)と言います．

$$\mathbf{\hat{u}} = \frac{\mathbf{u}}{|\mathbf{u}|}$$

単位ベクトルを返すメンバ関数の名前をunitとすると，次のような実装になります．

```cpp
struct Vector3
{
    inline const Vector3 unit() const;
};

inline const Vector3 Vector3::unit() const
{
    return *this / length();
}
```

一般的には，単位ベクトルを返す関数はnormalizeという名前になります．
しかし，normalizedという名前が使われることもあります．
また，変数を書き換えて単位ベクトルに変えてしまうような場合もあります．

この分かりにくさを避けるために，最近はunitという関数名を使うようにしています．

なお，他のメンバ関数と違い，構造体の外に定義を書いているのは，除算を利用するために，
実装を除算の後に回す必要があるからです．

また，constexprでないlength関数で割る関係上，unit関数もconstexprにはできません．

# クロス積

3次元ベクトルの場合，あるベクトル$\mathbf{u}$とあるベクトル$\mathbf{v}$について，
両方に直交するベクトル$\mathbf{w}$を次のような実装で求めることができます．

```cpp
struct Vector3
{
    constexpr const Vector3 cross(const Vector3 & v) const noexcept
    {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};
```

なお，このベクトルの向きは2つのベクトルに直交していますが，大きさについては次の式が成り立ちます．

$$|\mathbf{u}\times\mathbf{v}|=|\mathbf{u}||\mathbf{v}|\sin\theta$$

数学的に正しいかどうかは検討していないのですが，私は次のような覚え方をしています．

まず，$x$軸，$y$軸，$z$軸を表す単位ベクトル$\mathbf{x}$，$\mathbf{y}$，$\mathbf{z}$があるとします．
すると，次の式が成り立ちます．

$$
\mathbf{x} = \mathbf{y} \times \mathbf{z}\\
\mathbf{y} = \mathbf{z} \times \mathbf{x}\\
\mathbf{z} = \mathbf{x} \times \mathbf{y}
$$

つまり，$\mathbf{x} \rArr \mathbf{y} \rArr \mathbf{z} \rArr \mathbf{x}$という順番を覚えると，
ある2つのベクトルのクロス積で次の順番のベクトルが求まるわけです．

なお，順番を遡る場合，求まるベクトルは$-\mathbf{v}$のように逆ベクトルになります．

さて，ここで$x$成分を見てみると，$y$と$z$の積とその逆の積の減算になっています．
$y$成分は，$z$と$x$です．この流れは，先ほどのクロス積の順序と同じですね．
念のために$z$成分も見てみると，$x$と$y$ですね．

このようにパターンさえ覚えてしまえば，細かいことは分からなくても，
検索しなくてもクロス積を実装できるようになります．

# プロジェクション
:::message
そのうち追記されます．
:::

# リジェクション
:::message
そのうち追記されます．
:::

# 反射
:::message
そのうち追記されます．
:::

# 屈折
:::message
そのうち追記されます．
:::

# ソースコード全文
:::message
そのうち追記されます．
:::

# 今後の課題

- SIMD(Single Instruction Multiple Data)を使った高速化
- 単体テストの追加

# 参考文献

- [実例で学ぶゲーム3D数学](https://www.oreilly.co.jp/books/9784873113777/)
- Mathematics for 3D Game Programming and Computer Graphics Third Edition
- [Foundations of Game Engine Development Volume 1: Mathematics](https://foundationsofgameenginedev.com/)
- [DirectXMath](https://github.com/microsoft/DirectXMath)
- [glm](https://github.com/g-truc/glm)
- [Unreal Engine 4](https://github.com/EpicGames/UnrealEngine)
