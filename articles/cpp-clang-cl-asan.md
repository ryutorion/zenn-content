---
title: "Clang-CL + ASAN"
emoji: "✡️"
type: "tech"
topics: [ "cpp" ]
published: true
---

この記事では，Visual StudioでClangとAddress Sanitizer (以下ASAN) を使う方法について解説します．

メモリ関連のバグを検出するための機能として，Address Sanitizer，略してASANというものがあります．  
Microsoft Visual Studioにもこの機能が存在しており，基本的なことについては公式のドキュメントやモノリスソフト様の解説記事を読むと理解できます．

- [AddressSanitizer (Microsoft Learn)](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-180)
- [メモリバグ検出ツールのAddressSanitizerについて (モノリスソフト TECH BLOG)](https://www.monolithsoft.co.jp/techblog/articles/000507.html)

ここで，Visual StudioではC/C++のコンパイラとしてcl.exe以外に，clang-cl.exe(実質Clang)を選ぶことができます．
プロジェクト設定の全般にある「プラットフォームツールセット」を「LLVM (clang-cl)」に切り替えることで利用できます．

![](/images/clang-cl-asan-002.png)

プラットフォームツールセットに「LLVM (clang-cl)」が無い場合、Visual Studio Installerで「個別のコンポーネント」からLLVM関連の2つを追加すると出てきます．

![](/images/clang-cl-asan-003.png)

同じコードに対して，プラットフォームによってはcl.exeとclangを切り替えて使っている場合，片方でしか通らないコードを書いてしまって，後から気付く，なんてこともあります．  
可能なら，コンパイラは統一したいものです．

しかし，Clang-CLではcl.exe向けの一部の設定が必ずしも反映されません．ASANもその一つです．  
ClangそのものにはASANの機能があるので，これはあくまでもVisual Studioの設定の問題です．

Clang-CLでASANを使うには，プロジェクトのプロパティから「C/C++」の「コマンドライン」を選び，「追加のオプション」に「-fsanitize=address」を追加します．

![](/images/clang-cl-asan-004.png)

プロジェクト設定のほとんどがデフォルト値の場合，更にランタイムライブラリの変更が必要です．
「C/C++」の「コード生成」にある「ランタイムライブラリ」をデバッグ無しの「マルチスレッドDLL (/MD)」か「マルチスレッド (/MT)」にする必要があります．
ここでは「マルチスレッドDLL (/MD)」にしておきます．

![](/images/clang-cl-asan-005.png)

[LLVMのClang-CLについてのドキュメント](https://clang.llvm.org/docs/UsersManual.html#finding-clang-runtime-libraries)に書いてあることですが，追加でライブラリを指定する必要があります．
ここでは，#pragmaを利用して追加してみます．

```cpp
#ifdef __clang__
#pragma comment(lib, "clang_rt.asan_dynamic-x86_64.lib")
#pragma comment(lib, "clang_rt.asan_dynamic_runtime_thunk-x86_64.lib")
#endif
```

ドキュメントには1つ目しか書いていませんが，実際には2つ目も必要でした．「/MT」を選んだ場合は適宜書き換えてください．

これで終わりではなく，プリプロセッサに「_DEBUG」が定義してあると「/MD」にした関係でDebugビルドのリンクができません．
プロパティの「C/C++」の「プリプロセッサ」から「プリプロセッサの定義」を編集し，_DEBUGを消しましょう．

![](/images/clang-cl-asan-006.png)

これでようやくClang-CLでASANを試すことができる，と思いきや，まだ問題があります．

ASANそのものは有効にならないものの，プロパティのASANを有効にする設定は入れておかないとdllが見つからずに実行できないのです．

![](/images/clang-cl-asan-001.png)

これでようやく本当にASANが利用できるようになります．

ただし，このexeを配布するとdllが無いので起動できなかったりします．その場合は，vcxprojの「EnableASAN」の後に「CopyAsanBinariesToOutDir」をtrueで追加してやると出力フォルダに一緒にコピーされるようになるので，それを一緒に渡せば実行できるようになります．

```xml
<EnableASAN>true</EnableASAN>
<CopyAsanBinariesToOutDir>true</CopyAsanBinariesToOutDir>
```

実は，元々この記事を書こうと思ったきっかけは「stl_asan.lib」を要求されるというもので，「__msvc_sanitizer_annotate_container.hpp」にpragmaで強制的にリンクしていることが原因だったのですが，テンプレートから生成したプロジェクトでは発生しませんでした．
何かしらの設定が影響している影響しているのだと思いますが，取り合えず「_DISABLE_STRING_ANNOTATION」と「_DISABLE_VECTOR_ANNOTATION」をプリプロセッサの定義に追加してやれば回避できそうです．

とにかく，いくつかの問題を乗り越えたので，ようやくASANを使う環境が整いました．後はバグを修正するだけです．
そう，バグ修正が待っているのです．
年末までに何とかしたいものです．

## 参考文献

- [AddressSanitizer (Microsoft Learn)](https://learn.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-180)
- [メモリバグ検出ツールのAddressSanitizerについて (モノリスソフト TECH BLOG)](https://www.monolithsoft.co.jp/techblog/articles/000507.html)
- [Clang Compiler User's Manual](https://clang.llvm.org/docs/UsersManual.html)
