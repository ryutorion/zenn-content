---
title: "C++/Swiftの互換処理を自力で実装する 001"
emoji: "✡️"
type: "tech"
topics: [ "cpp", "swift" ]
published: true
---

# 互換のためのヘッダ生成

Swiftは現在C++とのinteropを強化しているようです．  
ただ，その自動生成されているコードが若干複雑すぎる気がしたので，自分なりに整理してみました．

まず，[Swift公式ドキュメント](https://www.swift.org/documentation/cxx-interop/project-build-setup/#mixing-swift-and-c-using-other-build-systems)を参考に，
C++側でSwiftの関数名などを見るためのヘッダを生成してみます．

空のswiftファイルを用意して，swiftcを次のように呼び出します．

```
> touch hello.swift
> swiftc -sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -frontend -typecheck -cxx-interoperability-mode=default -emit-clang-header-path Hello-Swift.h hello.swift
```

sdkを指定しない場合，`error: unable to load standard library for target 'arm64-apple-macosx15.0'`のようなエラーが出ることがあります．

# ヘッダを読む

生成されたヘッダの中を見てみると，Xcodeの中，`/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/swiftToCxx/`にあるヘッダを参照しています．

更にそのヘッダの中を見てみます．例としてStringクラスのコンストラクタを見てみます．

```cpp
/// Constructs a Swift string from a C string.
SWIFT_INLINE_THUNK String(const char *cString) noexcept {
    if (!cString) {
        auto res = _impl::$sS2SycfC();
        memcpy(_getOpaquePointer(), &res, sizeof(res));
        return;
    }
    auto res = _impl::$sSS7cStringSSSPys4Int8VG_tcfC(cString);
    memcpy(_getOpaquePointer(), &res, sizeof(res));
}
```

`$sSS7cStringSSSPys4Int8VG_tcfC`という奇妙な識別子がありますが，これはマングリングされた名前です．  
swiftcではなくswiftにdemangleという実引数を渡して，クォーテーションで囲ったマングリング名を渡すと元の名前が分かります．  
クォーテーションで囲むのは$がコマンドラインの変数と勘違いされないようにです．

```
> swift demangle '$sSS7cStringSSSPys4Int8VG_tcfC'
$sSS7cStringSSSPys4Int8VG_tcfC ---> Swift.String.init(cString: Swift.UnsafePointer<Swift.Int8>) -> Swift.String
```

そう，マングリングした名前を直接呼び出しているだけなんです．

そして，あちこちでいくつかの型とそれに対するmemcpyが行われていましたが，整理した結果，実質は次のようなオブジェクトのやり取りでした．

```cpp
struct SwiftObject
{
    uint64_t _1;
    void* _2;
};
```

# 自力で実装してみる

そこでちょっと手抜きになりますが，まずhello.swiftを次のように書き換えます．

```swift
public func hello(message: String)
{
    print(message)
}
```

これを次のようにコンパイルしてオブジェクトファイルを用意します．

```
> swiftc -parse-as-library -emit-object hello.swift 
```

ライブラリとしてビルドしないとmainが含まれてしまいます．  
そして，nmでシンボルを確認します．

```
> nm hello.o
0000000000000000 T _$s5helloAA7messageySS_tF
```

定義した関数のマングリングされた名前は$s5helloAA7messageySS_tFのようです．

これらの情報を元に次のようにC++のコードを書きます．

```cpp
// main.cpp
#include <cstdint>
#include <iostream>

struct SwiftObject
{
    uint64_t _1;
    void* _2;
};


// SwiftのStringクラスを空で初期化する場合の関数
extern "C" SwiftObject $sS2SycfC() noexcept __attribute__((swiftcall));

// SwiftのStringクラスをCの文字列で初期化する場合の関数
extern "C" SwiftObject $sSS7cStringSSSPys4Int8VG_tcfC(const char*) noexcept __attribute__((swiftcall));

// Swiftの文字列を受け取るhello関数
extern "C" void $s5helloAA7messageySS_tF(SwiftObject) noexcept __attribute__((swiftcall));

namespace Swift
{
    class String
    {
        public:
            String(const char* s)
                : mSelf(s ? $sSS7cStringSSSPys4Int8VG_tcfC(s) : $sS2SycfC())
                {}

            SwiftObject mSelf;
    };
}

int main()
{
    Swift::String s("Hello, World");
    $s5helloAA7messageySS_tF(s.mSelf);
}
```

これを次のようにコンパイルして実行してみます．

```cpp
> clang++ -std=c++17 -o main main.cpp hello.o -lswiftCore
> ./main
Hello, World
```

無事実行できました．

# まとめ

今回は，Swiftの関数をC++から呼び出しました．次は逆方向も実現したいものです．

他にも色々と調べる必要があることがありそうです．

例えば，__attribute__((swiftcall))の効果だったり，asyncなどの機能に公式でも対応していないので，そういったところをどうするのかも調べる必要がありそうです．  
今回は，Swiftオブジェクトのリファレンスカウント操作も無視したので，実際にはメモリリークしているはずです．  
また，独自のアロケータを指定することもできるようなので，その辺も確認していく必要がありそうです．

来年はその辺も含めて調査したいところです．
