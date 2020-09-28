---
title: "ゲームプログラミングのための数学 - 行列"
emoji: "✡️"
topics: [ "cpp", "math" ]
published: true
type: "tech"
---

# はじめに

:::message alert
この記事は未完成で，時折追記されたり，修正されたりします．
:::

プログラマの視点で言えば，行列(matrix)は2次元配列です．
横方向を行(row)，縦方向を列(column)と言います．

数学では，行や列は1から数えることが多いようですが，プログラマ向けなので，
ここでは0から数えることにします．

ゲームプログラミングで扱う行列は，2行2列，3行3列，4行4列が一般的です．
ここでは，特によく使われる4行4列の行列を取り上げて，
C++のコードを交えつつ説明してみます．

また，行と列の数が同じ行列を正方行列と言います．

:::message alert
ここでの実装は，こんな実装もあり得るというだけで，正解というわけではありません．
また，C++をある程度理解している前提とします．
:::

データ構造は，次のようになります．

```cpp
class Matrix4x4
{
private:
    float mV[4][4];
};
```

$r$行$c$列目の値はmV[r][c]でアクセスします．数式で書く場合，$m_{r,c}$のように書きます．
また，その値を成分(element)とも言います．

:::message alert
ここでは，行x列という順で書いていますが，GLSLでは列x行の順になってたりします．
どちらを先にするか，というのは決まってないので，新しい環境では確認した方が良いでしょう．
:::

[ゲームプログラミングのための数学 - ベクトル](https://zenn.dev/ryutorion/articles/game-math-vector)を読んだ方は，メンバ変数はpublicで良いのでは? と
思うかもしれません．
ただ，行列の場合は注意にも書いたように行と列の順序が決まっていなかったり，
成分の名前がベクトルほど共通認識となっているものが無いため，privateになっています．

# コンストラクタ

まずは，デフォルトコンストラクタを実装します．

簡単にするために，既に出てきた実装は省略して書いてあります．
メンバ関数はクラスの中に，通常の関数はクラスの外に書きます．

```cpp
class Matrix4x4
{
public:
    Matrix4x4() noexcept = default;
};
```

デフォルトコンストラクタでは何もしません．
配列を用意しておいて，後からデータを読み込んで初期化をする，というような場合に，
何かしらの初期化を必ずする，というのはまぁまぁなコストになります．
特にゲームプログラムのように大量に行列を扱う場合，その影響は無視できません．

次に，各成分を指定して初期化するコンストラクタを書きます．

```cpp
class Matrix4x4
{
public:
    constexpr Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    ) noexcept
        : mV {
            m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33
        }
    {}
};
```

C++11より前だと，メンバ関数の初期化時に配列の初期化ができなかったので，
こんな書き方はできなかったんですが，今ではこんな風に書けるようになりました．

# 加算

行列同士の加算をする場合，成分同士の加算になります．

つまり，行列$A$と$B$を足して$C$を作る場合，$c_{r,c} = a_{r,c}+b_{r,c}$となります．
ここで注意しないといけないのが，行列の加算は行と列の数が同じ場合のみ実行できる
ということです．4行4列同士の加算はできますが，4行3列と3行4列の行列の加算はできません．

では，実装してみます．

```cpp
class Matrix4x4
{
public:
    Matrix4x4 & operator+=(const Matrix4x4 & m) noexcept
    {
        mV[0][0] += m.mV[0][0];
        mV[0][1] += m.mV[0][1];
        mV[0][2] += m.mV[0][2];
        mV[0][3] += m.mV[0][3];

        mV[1][0] += m.mV[1][0];
        mV[1][1] += m.mV[1][1];
        mV[1][2] += m.mV[1][2];
        mV[1][3] += m.mV[1][3];

        mV[2][0] += m.mV[2][0];
        mV[2][1] += m.mV[2][1];
        mV[2][2] += m.mV[2][2];
        mV[2][3] += m.mV[2][3];

        mV[3][0] += m.mV[3][0];
        mV[3][1] += m.mV[3][1];
        mV[3][2] += m.mV[3][2];
        mV[3][3] += m.mV[3][3];

        return *this;
    }

    constexpr const Matrix4x4 operator+(const Matrix4x4 & m) const noexcept
    {
        return Matrix4x4(
            mV[0][0] + m.mV[0][0],
            mV[0][1] + m.mV[0][1],
            mV[0][2] + m.mV[0][2],
            mV[0][3] + m.mV[0][3],

            mV[1][0] + m.mV[1][0],
            mV[1][1] + m.mV[1][1],
            mV[1][2] + m.mV[1][2],
            mV[1][3] + m.mV[1][3],

            mV[2][0] + m.mV[2][0],
            mV[2][1] + m.mV[2][1],
            mV[2][2] + m.mV[2][2],
            mV[2][3] + m.mV[2][3],

            mV[3][0] + m.mV[3][0],
            mV[3][1] + m.mV[3][1],
            mV[3][2] + m.mV[3][2],
            mV[3][3] + m.mV[3][3]
        );
    }
};
```

単純に成分同士を足すだけなのですが，constexprやconst，noexceptなどは忘れがちです．

また，ループにしたほうが良いのでは，と思うかもしれませんが，最初にも書いたように，
ゲームプログラミングで登場する行列は行と列の数が決まっているものが多く，
あちこちで使われるため，ループを展開して少しでも速度を稼ぐのが一般的です．

# 減算

減算は，加算と同じなので説明は省きます．

```cpp
class Matrix4x4
{
public:
    constexpr Matrix4x4 & operator-=(const Matrix4x4 & m) noexcept
    {
        mV[0][0] -= m.mV[0][0];
        mV[0][1] -= m.mV[0][1];
        mV[0][2] -= m.mV[0][2];
        mV[0][3] -= m.mV[0][3];

        mV[1][0] -= m.mV[1][0];
        mV[1][1] -= m.mV[1][1];
        mV[1][2] -= m.mV[1][2];
        mV[1][3] -= m.mV[1][3];

        mV[2][0] -= m.mV[2][0];
        mV[2][1] -= m.mV[2][1];
        mV[2][2] -= m.mV[2][2];
        mV[2][3] -= m.mV[2][3];

        mV[3][0] -= m.mV[3][0];
        mV[3][1] -= m.mV[3][1];
        mV[3][2] -= m.mV[3][2];
        mV[3][3] -= m.mV[3][3];

        return *this;
    }

    constexpr const Matrix4x4 operator-(const Matrix4x4 & m) const noexcept
    {
        return Matrix4x4(
            mV[0][0] - m.mV[0][0],
            mV[0][1] - m.mV[0][1],
            mV[0][2] - m.mV[0][2],
            mV[0][3] - m.mV[0][3],

            mV[1][0] - m.mV[1][0],
            mV[1][1] - m.mV[1][1],
            mV[1][2] - m.mV[1][2],
            mV[1][3] - m.mV[1][3],

            mV[2][0] - m.mV[2][0],
            mV[2][1] - m.mV[2][1],
            mV[2][2] - m.mV[2][2],
            mV[2][3] - m.mV[2][3],

            mV[3][0] - m.mV[3][0],
            mV[3][1] - m.mV[3][1],
            mV[3][2] - m.mV[3][2],
            mV[3][3] - m.mV[3][3]
        );
    }
};
```

# 乗算

行列の乗算には，行列と数値の積と，行列同士の積が考えられます．

行列$M$に対して数値$s$を掛ける場合，$sM$のように書き，
それぞれの成分に$s$を掛けます．つまり，$N = sM$とすると，$n_{r,c}=sm_{r,c}$となります．

```cpp
class Matrix4x4
{
public:
    Matrix4x4 & operator*=(const float s)
    {
        mV[0][0] *= s;
        mV[0][1] *= s;
        mV[0][2] *= s;
        mV[0][3] *= s;

        mV[1][0] *= s;
        mV[1][1] *= s;
        mV[1][2] *= s;
        mV[1][3] *= s;

        mV[2][0] *= s;
        mV[2][1] *= s;
        mV[2][2] *= s;
        mV[2][3] *= s;

        mV[3][0] *= s;
        mV[3][1] *= s;
        mV[3][2] *= s;
        mV[3][3] *= s;

        return *this;
    }

    constexpr const Matrix4x4 operator*(const float s) const noexcept
    {
        return Matrix4x4(
            mV[0][0] * s,
            mV[0][1] * s,
            mV[0][2] * s,
            mV[0][3] * s,
            mV[1][0] * s,
            mV[1][1] * s,
            mV[1][2] * s,
            mV[1][3] * s,
            mV[2][0] * s,
            mV[2][1] * s,
            mV[2][2] * s,
            mV[2][3] * s,
            mV[3][0] * s,
            mV[3][1] * s,
            mV[3][2] * s,
            mV[3][3] * s
        );
    }
};

inline constexpr const Matrix4x4 operator*(const float s, const Matrix4x4 & m) noexcept
{
    return m * s;
}
```

成分へのアクセスはまだ用意されていないので，右から数値を掛ける演算子はメンバ関数にして，
左から掛ける演算子はそちらを利用して実装しています．

次に，行列同士の乗算について見ていきます．
行列$X$，$Y$，$Z$について$Z = XY$とすると，$z_{r,c}$は$X$の$r$行ベクトルと$Y$の$c$列ベクトルの
ドット積，つまり成分同士の積の和になります．

つまり，次の図のような計算になります．

:::message
画像を追加
:::

もう少し数式で書いてみます．ここでは，4行4列の行列同士の積を考えます．

$$z_{r,c} = \sum^{3}_{k=0}x_{r,k}y_{k,c}$$

行列同士の積の定義から，積の左側の列数と右側の行数は同じという前提になります．

では，実装を見てみましょう．

```cpp
class Matrix4x4
{
public:
    constexpr const Matrix4x4 operator*(const Matrix4x4 & m) const noexcept
    {
        return Matrix4x4(
            mV[0][0] * m.mV[0][0] +
            mV[0][1] * m.mV[1][0] +
            mV[0][2] * m.mV[2][0] +
            mV[0][3] * m.mV[3][0],
            mV[0][0] * m.mV[0][1] +
            mV[0][1] * m.mV[1][1] +
            mV[0][2] * m.mV[2][1] +
            mV[0][3] * m.mV[3][1],
            mV[0][0] * m.mV[0][2] +
            mV[0][1] * m.mV[1][2] +
            mV[0][2] * m.mV[2][2] +
            mV[0][3] * m.mV[3][2],
            mV[0][0] * m.mV[0][3] +
            mV[0][1] * m.mV[1][3] +
            mV[0][2] * m.mV[2][3] +
            mV[0][3] * m.mV[3][3],

            mV[1][0] * m.mV[0][0] +
            mV[1][1] * m.mV[1][0] +
            mV[1][2] * m.mV[2][0] +
            mV[1][3] * m.mV[3][0],
            mV[1][0] * m.mV[0][1] +
            mV[1][1] * m.mV[1][1] +
            mV[1][2] * m.mV[2][1] +
            mV[1][3] * m.mV[3][1],
            mV[1][0] * m.mV[0][2] +
            mV[1][1] * m.mV[1][2] +
            mV[1][2] * m.mV[2][2] +
            mV[1][3] * m.mV[3][2],
            mV[1][0] * m.mV[0][3] +
            mV[1][1] * m.mV[1][3] +
            mV[1][2] * m.mV[2][3] +
            mV[1][3] * m.mV[3][3],

            mV[2][0] * m.mV[0][0] +
            mV[2][1] * m.mV[1][0] +
            mV[2][2] * m.mV[2][0] +
            mV[2][3] * m.mV[3][0],
            mV[2][0] * m.mV[0][1] +
            mV[2][1] * m.mV[1][1] +
            mV[2][2] * m.mV[2][1] +
            mV[2][3] * m.mV[3][1],
            mV[2][0] * m.mV[0][2] +
            mV[2][1] * m.mV[1][2] +
            mV[2][2] * m.mV[2][2] +
            mV[2][3] * m.mV[3][2],
            mV[2][0] * m.mV[0][3] +
            mV[2][1] * m.mV[1][3] +
            mV[2][2] * m.mV[2][3] +
            mV[2][3] * m.mV[3][3],

            mV[3][0] * m.mV[0][0] +
            mV[3][1] * m.mV[1][0] +
            mV[3][2] * m.mV[2][0] +
            mV[3][3] * m.mV[3][0],
            mV[3][0] * m.mV[0][1] +
            mV[3][1] * m.mV[1][1] +
            mV[3][2] * m.mV[2][1] +
            mV[3][3] * m.mV[3][1],
            mV[3][0] * m.mV[0][2] +
            mV[3][1] * m.mV[1][2] +
            mV[3][2] * m.mV[2][2] +
            mV[3][3] * m.mV[3][2],
            mV[3][0] * m.mV[0][3] +
            mV[3][1] * m.mV[1][3] +
            mV[3][2] * m.mV[2][3] +
            mV[3][3] * m.mV[3][3]
        );
    }

    Matrix4x4 & operator*=(const Matrix4x4 & m) noexcept
    {
        *this = *this * m;

        return *this;
    }
};
```

長いですね．加算のところでも書きましたが，よく呼ばれる処理なので，
ループは用いずに書いています．

そして，乗算代入は通常の乗算を使って実装しています．
これは，式を見ると分かるように，$m_{0,0}$の新しい値が決まる後にも
古い値$m_{0,0}$の値を使う必要があるように，各成分を新しい値に置き換えられるのは，
その行の成分の計算がすべて終わった後，ということになります．
つまり，若干実装が面倒になるので，こんな感じの実装にしています．

:::message alert
行列同士の積は，その仕組み上，$AB$と$BA$が等しいとは限りません．
:::

# 転置

行列$M$の成分の行と列を入れ替えた行列を転置行列と言い，$M^T$と書きます．
つまり，$m^T_{r,c}=m_{c,r}$となります．

実装は，行と列を入れ替えるだけなのでシンプルです．

```cpp
class Matrix4x4
{
public:
    constexpr const Matrix4x4 transpose() const noexcept
    {
        return Matrix4x4(
            m[0][0], m[1][0], m[2][0], m[3][0],
            m[0][1], m[1][1], m[2][1], m[3][1],
            m[0][2], m[1][2], m[2][2], m[3][2],
            m[0][3], m[1][3], m[2][3], m[3][3]
        );
    }
};
```

## 転置と行列同士の乗算の関係

転置と行列同士の乗算については，次の関係が成り立ちます．

$$(AB)^T = B^TA^T$$

ここでは，$x$行$y$列の行列$A$と$y$行$z$列の行列$B$で考えてみます．
$C = AB$とすると，$C$の成分$c_{i,j}$は次のようになります．

$$c_{i,j}=\sum^{y-1}_{k=0}a_{i,k}b_{k,j}$$

また，$C^T=B^TA^T$とすると，$C^T$の成分$c^T{i,j}$は次のようになります．

$$c^T_{i,j}=\sum^{y-1}_{k=0}b^T_{i,k}a^T_{k,j}=\sum^{y-1}_{k=0}a_{j,k}b_{k,i}=c_{j,i}$$

よって，$(AB)^T=B^TA^T$であることが分かりました．
これは，行列をメモリに格納する際にどのように格納するか，といった話と関係してきます．

# 単位行列(Identity Matrix)

次のように対角成分が1で，それ以外は0の行列$I$を単位行列と言います．

$$
I = \left(\begin{matrix}
1 & 0 & 0 & 0\\
0 & 1 & 0 & 0\\
0 & 0 & 1 & 0\\
0 & 0 & 0 & 1
\end{matrix}\right)
$$

# 行列式(determinant)

行と列が同じ数の正方行列では，行列式という値を計算することができます．
この行列式を使うことで，この後に登場する逆行列を求めることができます．
ある行列$M$の行列式は$\mathrm{det}A$や$|A|$と書きます．

ただ，この行列式の計算は行と列の数が増えるほどに面倒になります．

次のコードは，Unreal Engine 4の行列式を求めるコードから，
成分1つ分の計算式を引用したものです．

```cpp
inv[0]  =  s5 * s10 * s15 - s5 * s11 * s14 - s9 * s6 * s15 + s9 * s7 * s14 + s13 * s6 * s11 - s13 * s7 * s10;
```

長いですね．正直なところ，これを正しく書く自信はありません．
本来は，数学的にどういう意味があって云々にも触れるべきなのでしょうが，
ここでは計算手順だけ示すことにします．

## 小行列式(minor)

ある正方行列$M$の$i$行と$j$列を取り除いた正方行列の行列式を小行列式と言い，$M_{i,j}$と書きます．

2行2列の場合の小行列式を考えてみましょう．

$$
M = \left(\begin{matrix}
1 & 2\\
3&4
\end{matrix}\right)
$$

$$
\begin{array}{l}
M_{0,0}=|4|=4\\
M_{0,1}=|3|=3\\
M_{1,0}=|2|=2\\
M_{1,1}=|1|=1
\end{array}
$$

3行3列以上の場合，2行2列の小行列式を求めるために，次に説明する余因子が必要になります．

## 余因子(cofactor)

小行列式$M_{i,j}$に$(-1)^{i+j}$を掛けたものを余因子と言います．
$$C_{i,j}=(-1)^{i+j}M_{i,j}$$

この余因子を使って，ある行に沿って成分$m_{i,j}$と余因子$C_{i,j}$を掛けた和が行列式になります．
また，列に沿って計算しても同じ結果になります．
これを，余因子展開(cofactor expansion)やラプラス展開(Laplace expansion)と言います．

$$\mathrm{det}M=\sum_{k=0}^{n}m_{k,j}C_{k,j}=\sum_{k=0}^{n}m_{i,k}C_{i,k}$$

2行2列の例を見てみましょう．

$$M = \left(\begin{matrix}1 & 2\\3&4\end{matrix}\right)$$
$$
\begin{array}{l}
C_{0,0}=(-1)^{0+0}M_{0,0}=4\\
C_{0,1}=(-1)^{0+1}M_{0,1}=-3\\
C_{1,0}=(-1)^{1+0}M_{1,0}=-2\\
|M| = 1*C_{0,0}+2*C_{0,1}=1*4+2*(-3)=-2\\
|M| = 1*C_{0,0}+3*C_{1,0}=1*4+3*(-2)=-2
\end{array}
$$

2行2列の場合，簡単なので行列式の求め方は公式にもなっています．

$$|M|=m_{0,0}m_{1,1}-m_{0,1}m_{1,0}$$

3行3列も計算してみましょう．

$$
M = \left(\begin{matrix}
1 & 2 & 3\\
4 & 5 & 6\\
7 & 8 & 9
\end{matrix}\right)
$$

$$
\begin{array}{l}
|C_{0,0}| = (-1)^{0+0}\left|\begin{matrix}
5 & 6\\
8 & 9
\end{matrix}\right| = 5*9-6*8=-3\\
|C_{0,1}| = (-1)^{0+1}\left|\begin{matrix}
4 & 6 \\
7 & 9
\end{matrix}\right|=-(4*9-6*7)=6\\
|C_{0,2}| = (-1)^{0+2}\left|\begin{matrix}
4 & 5 \\
7 & 8
\end{matrix}\right|=4*8-5*7=-3\\
|M|=1*C_{0,1}+2*C_{0,1}+3*C_{0,2}=1*(-3)+2*6+3*(-3)=0
\end{array}
$$

おっと，0になってしまいました．

## 実装

4行4列の行列式を求めるには，3行3列，2行2列の行列式が必要になるので，
その部分だけ実装します．

```cpp
class Matrix2x2
{
public:
    constexpr Matrix2x2(
        float m00, float m01,
        float m10, float m11
    )
        : mV {
            m00, m01,
            m10, m11
        }
    {}

    constexpr float determinant() const noexcept
    {
        return mV[0][0] * mV[1][1] - mV[0][1] * mV[1][0];
    }
private:
    float mV[2][2];
};

class Matrix3x3
{
public:
    constexpr Matrix3x3(
        float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m10, float m11, float m12
    )
        : mV {
            m00, m01, m02,
            m10, m11, m12,
            m20, m21, m22
        }
    {}

    constexpr float determinant() const noexcept
    {
        float c00 = mV[0][0] * Matrix2x2(
            mV[1][1], mV[1][2],
            mV[2][1], mV[2][2]
        ).determinant();

        float c01 = -mV[0][1] * Matrix2x2(
            mV[1][0], mV[1][2],
            mV[2][0], mV[2][2]
        ).determinant();

        float c02 = mV[0][2] * Matrix2x2(
            mV[1][0], mV[1][1],
            mV[2][0], mV[2][1]
        ).determinant();

        return c00 + c01 + c02;
    }
private:
    float mV[3][3];
};

class Matrix4x4
{
public:
    constexpr float determinant() const noexcept
    {
        float c00 = mV[0][0] * Matrix3x3(
            mV[1][1], mV[1][2], mV[1][3],
            mV[2][1], mV[2][2], mV[2][3],
            mV[3][1], mV[3][2], mV[3][3]
        ).determinant();

        float c01 = -mV[0][1] * Matrix3x3(
            mV[1][0], mV[1][2], mV[1][3],
            mV[2][0], mV[2][2], mV[2][3],
            mV[3][0], mV[3][2], mV[3][3]
        ).determinant();

        float c02 = mV[0][2] * Matrix3x3(
            mV[1][0], mV[1][1], mV[1][3],
            mV[2][0], mV[2][1], mV[2][3],
            mV[3][0], mV[3][1], mV[3][3]
        ).determinant();

        float c03 = -mV[0][3] * Matrix3x3(
            mV[1][0], mV[1][1], mV[1][2],
            mV[2][0], mV[2][1], mV[2][2],
            mV[3][0], mV[3][1], mV[3][2]
        ).determinant();

        return c00 + c01 + c02 + c03;
    }
};
```

# 逆行列

ある行列$M$に掛けると単位行列になるような行列を逆行列と言い，$M^{-1}$のように書きます．
$$MM^{-1} = I$$

この逆行列は，余因子$C_{i,j}$を用いて次のように掛けます．

$$
M^{-1}=\frac{1}{|M|}\left(\begin{matrix}
C_{0,0}&
C_{1,0}&
C_{2,0}&
C_{3,0}\\
C_{0,1}&
C_{1,1}&
C_{2,1}&
C_{3,1}\\
C_{0,2}&
C_{1,2}&
C_{2,2}&
C_{3,2}\\
C_{0,3}&
C_{1,3}&
C_{2,3}&
C_{3,3}
\end{matrix}\right)
$$

:::message alert
余因子の並びは行と列が入れ替わった転置の状態なことに注意しましょう．
:::

2行2列の場合を計算してみましょう．

$$
M=\left(\begin{matrix}
1 & 2\\
3 & 4
\end{matrix}\right)
$$

$$
\begin{array}{l}
M^{-1}=\frac{1}{-2}\left(\begin{matrix}
4 & -2\\
-3 & 1
\end{matrix}\right)=
\left(\begin{matrix}
-2 & 1\\
\frac{3}{2} & -\frac{1}{2}
\end{matrix}\right)
\end{array}
$$

本当に逆行列になっているのか確認してみましょう．

$$
MM^{-1}=\left(\begin{matrix}
1 & 2\\
3 & 4
\end{matrix}\right)
\left(\begin{matrix}
-2 & 1\\
\frac{3}{2} & -\frac{1}{2}
\end{matrix}\right)=
\left(\begin{matrix}
1*(-2)+2*\frac{3}{2} & 1*1+2*\frac{-1}{2}\\
3*(-2)+4*\frac{3}{2} & 3*1+4*\frac{-1}{2}
\end{matrix}\right)=
\left(\begin{matrix}
1 & 0\\
0 & 1
\end{matrix}\right)
$$

3行3列も見てみましょう，と言いたいところですが，例に出した3行3列の行列式が0でした．
そう，除算をする関係で，行列式が0の場合は逆行列が求まらないのです．

逆行列を求める実装は次のようになります．

```cpp
class Matrix4x4
{
public:
    constexpr const Matrix4x4 inverse() const
    {
        float c00 = mV[0][0] * Matrix3x3(
            mV[1][1], mV[1][2], mV[1][3],
            mV[2][1], mV[2][2], mV[2][3],
            mV[3][1], mV[3][2], mV[3][3]
        ).determinant();
        float c01 = -mV[0][1] * Matrix3x3(
            mV[1][0], mV[1][2], mV[1][3],
            mV[2][0], mV[2][2], mV[2][3],
            mV[3][0], mV[3][2], mV[3][3]
        ).determinant();
        float c02 = mV[0][2] * Matrix3x3(
            mV[1][0], mV[1][1], mV[1][3],
            mV[2][0], mV[2][1], mV[2][3],
            mV[3][0], mV[3][1], mV[3][3]
        ).determinant();
        float c03 = -mV[0][3] * Matrix3x3(
            mV[1][0], mV[1][1], mV[1][2],
            mV[2][0], mV[2][1], mV[2][2],
            mV[3][0], mV[3][1], mV[3][2]
        ).determinant();

        float c10 = -mV[1][0] * Matrix3x3(
            mV[0][1], mV[0][2], mV[0][3],
            mV[2][1], mV[2][2], mV[2][3],
            mV[3][1], mV[3][2], mV[3][3]
        ).determinant();
        float c11 = mV[1][1] * Matrix3x3(
            mV[0][0], mV[0][2], mV[0][3],
            mV[2][0], mV[2][2], mV[2][3],
            mV[3][0], mV[3][2], mV[3][3]
        ).determinant();
        float c12 = -mV[1][2] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][3],
            mV[2][0], mV[2][1], mV[2][3],
            mV[3][0], mV[3][1], mV[3][3]
        ).determinant();
        float c13 = mV[1][3] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][2],
            mV[2][0], mV[2][1], mV[2][2],
            mV[3][0], mV[3][1], mV[3][2]
        ).determinant();

        float c20 = mV[2][0] * Matrix3x3(
            mV[0][1], mV[0][2], mV[0][3],
            mV[1][1], mV[1][2], mV[1][3],
            mV[3][1], mV[3][2], mV[3][3]
        ).determinant();
        float c21 = -mV[2][1] * Matrix3x3(
            mV[0][0], mV[0][2], mV[0][3],
            mV[1][0], mV[1][2], mV[1][3],
            mV[3][0], mV[3][2], mV[3][3]
        ).determinant();
        float c22 = mV[2][2] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][3],
            mV[1][0], mV[1][1], mV[1][3],
            mV[3][0], mV[3][1], mV[3][3]
        ).determinant();
        float c23 = -mV[2][3] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][2],
            mV[1][0], mV[1][1], mV[1][2],
            mV[3][0], mV[3][1], mV[3][2]
        ).determinant();

        float c30 = -mV[3][0] * Matrix3x3(
            mV[0][1], mV[0][2], mV[0][3],
            mV[1][1], mV[1][2], mV[1][3],
            mV[2][1], mV[2][2], mV[2][3]
        ).determinant();
        float c31 = mV[3][1] * Matrix3x3(
            mV[0][0], mV[0][2], mV[0][3],
            mV[1][0], mV[1][2], mV[1][3],
            mV[2][0], mV[2][2], mV[2][3]
        ).determinant();
        float c32 = -mV[3][2] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][3],
            mV[1][0], mV[1][1], mV[1][3],
            mV[2][0], mV[2][1], mV[2][3]
        ).determinant();
        float c33 = mV[3][3] * Matrix3x3(
            mV[0][0], mV[0][1], mV[0][2],
            mV[1][0], mV[1][1], mV[1][2],
            mV[2][0], mV[2][1], mV[2][2]
        ).determinant();

        float rd = 1.0f / (c00 + c01 + c02 + c03);

        return Matrix4x4(
            c00 * rd, c10 * rd, c20 * rd, c30 * rd,
            c01 * rd, c11 * rd, c21 * rd, c31 * rd,
            c02 * rd, c12 * rd, c22 * rd, c32 * rd,
            c03 * rd, c13 * rd, c23 * rd, c33 * rd
        );
    }
};
```

# 行列とベクトルの乗算

$n$次のベクトルを1行$n$列の行列，または$n$行1列の行列とみなすと，
行列とベクトルの乗算ができます．
このとき，1行$n$列の行列を行ベクトル，$n$行1列の行列を列ベクトルと言ったりもします．

3行3列の行列$A$と3次の列ベクトル$\mathbf{v}$，$\mathbf{b}$があったとして，次のような形を線形変換と言います．

$$A\mathbf{v} + \mathbf{b}$$

成分での計算を見てみると，次のようになります．

$$
\left(\begin{matrix}
a_{0,0}v_x + a_{0,1}v_y + a_{0,2}v_z + b_x\\
a_{1,0}v_x + a_{1,1}v_y + a_{1,2}v_z + b_y\\
a_{2,0}v_x + a_{2,1}v_y + a_{2,2}v_z + b_z
\end{matrix}\right)
$$

ここで，行列$A$を次のような4次行列にしてみます．

$$
A = \left(\begin{matrix}
a_{0,0} & a_{0,1} & a_{0,2} & b_x\\
a_{1,0} & a_{1,1} & a_{1,2} & b_y\\
a_{2,0} & a_{2,1} & a_{2,2} & b_z\\
0 & 0 & 0 & 1\\
\end{matrix}\right)
$$

そして，列ベクトル$\mathbf{v}$は次のような4次ベクトルにしてみます．

$$
\mathbf{v} = \left(\begin{matrix}
v_x\\
v_y\\
v_z\\
1
\end{matrix}\right)
$$

すると，$A\mathbf{v}$は次のようになります．

$$
A\mathbf{v}=\left(\begin{matrix}
a_{0,0}v_x + a_{0,1}v_y + a_{0,2}v_z + b_x\\
a_{1,0}v_x + a_{1,1}v_y + a_{1,2}v_z + b_y\\
a_{2,0}v_x + a_{2,1}v_y + a_{2,2}v_z + b_z\\
1
\end{matrix}\right)
$$

つまり，4行4列の行列を使うことで3次のベクトルへの乗算と加算を表せるのです．
このことを利用して，ベクトルの変形を色々と表現できるのですが，
記事が長くなりすぎたので，変形については別の記事に分けます．

# 成分へのアクセス

通常，行列を実装する場合はそれぞれの成分へのアクセスできるように
アクセッサを実装すると思いますが，ここでは敢えて実装しませんでした．

ゲームプログラミングでは，行列をベクトルの変形に使うことが多いので，
すべての成分がそろって意味を持っています．
そのため，下手に部分的に変更されても困ります．

# 追加の実装

4行4列の逆行列の計算では，3行3列以下の行列を利用して実装していますが，
実際には高速化のために数式を展開していることが多いです．
正しい実装が確認できたら，数式を展開してみるのも良いでしょう．

# 参考文献

- [プログラミングのための線形代数](https://www.ohmsha.co.jp/book/9784274065781/)
- [行列プログラマー Pythonプログラムで学ぶ線形代数](https://www.oreilly.co.jp/books/9784873117775/)
- [実例で学ぶゲーム3D数学](https://www.oreilly.co.jp/books/9784873113777/)
- Mathematics for 3D Game Programming and Computer Graphics Third Edition
- [Foundations of Game Engine Development Volume 1: Mathematics](https://foundationsofgameenginedev.com/)
- [DirectXMath](https://github.com/microsoft/DirectXMath)
- [glm](https://github.com/g-truc/glm)
- [Unreal Engine 4(FMatrix)](https://docs.unrealengine.com/en-US/API/Runtime/Core/Math/FMatrix/index.html)

# 謝辞

@[tweet](https://twitter.com/nico_shindannin/status/1310376329410301952)

# ソースコード全文


