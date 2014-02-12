# nanojson
---
Copyright © 2014 Giemsa/MithrilWorks
  
　jsonをC++の構造体にマッピングするﾒﾒﾀｧな闇ライブラリ。  
**実装が圧倒的に適当なので使用の際は要注意!**

## 使い方
nanojson.hをインクルード

	#include <iostream>
	#include <vector>
	#include "nanojson.h"

	// nanojson::objectを継承
	struct Person : public nanojson::object<Person>
	{
		// def(型名, メンバ名);
		def(std::string, name);
		def(int, age);
	};

	struct JSONSample : public nanojson::object<JSONSample>
	{
		def(std::string, hoge);
		def(std::vector<Person>, list);
	};

	int main(int argc, const char * argv[])
	{
		JSONSample json;
		nanojson::reader reader("test.json");

		json = reader.parse<JSONSample>();
		std::cout << "hoge = " << json.hoge << std::endl;
		for(std::vector<Person>::iterator it = json.list.begin(); it != json.list.end(); ++it)
			std::cout << " name: " << it->name << ", age: " << it->age << std::endl;

		return 0;
	}

　`def(型名, メンバ名)`のように指定することで、該当するJSONの要素を自動的に読み込みます。def以外で定義したメンバは読み込み対象になりません。

	struct JSONSample : public nanojson::object<JSONSample>
	{
		def(std::string, hoge); // 読み込み対象
		int x; // 読み込み対象外
	};

## 使えるデータ型
　構造体のメンバには、以下の型のみ使用可能です。それ以外の型を指定すると死にます。

* ポインタ型 (*1)
* int (*2)
* double
* std::string
* std::vector<T> (*3)
* nanojson::object<T>

\*1 ポインタ型をメンバに持つことができますが、JSON側では`null`が指定される必要があります。  
\*2 `std::numeric_limits<T>::is_integer`が`true`の整数ならばマッピング可能です。  
\*3 C++なので、配列の要素は単一の型である必要があります。またTは上記の型のいずれかである必要があります。

## リファレンス的な
### nanojson::object\<T\>
　JSONのオブジェクト一つを表すクラス。Tには自分自身のクラスを指定してください。

	struct Person : public nanojson::object<Person> { }

### nanojson::reader
　JSONパーサです。使い方はmain.cppを参照してください。

### nanojson::exception
　例外クラスです。何かエラーが発生すると飛んできます。

* const char *getMessage()
	* エラーメッセージを取得します。
* int getLine()
	* エラー行数を取得します。
* getFileName()
	* エラーが発生したファイル名が返ります。もちろん"nanojson.h"です!(意味なし)
* getFuncName()
	* エラーが発生した関数名が返ります。

### defマクロ
　構造体のメンバを定義するマクロです。このマクロを使用して追加されたメンバは、JSONの値がマッピングされる対象となります。

	struct Person : public nanojson::object<Person>
	{
		def(std::string, name);
		def(int, age);
	};

## テスト環境
* Xcode 4.6.2(Apple LLVM compiler 4.2)

## 動作環境
　C++03がコンパイルできるC++コンパイラ

## 留意事項
* 実装が適当です。あとで頑張って直します(あとで)。
* nanojson名前空間にある、アンダーバーで始まるクラスや関数等は内部で使用しているものです。使用しないでください。
* 現状では、defを使用した構造体には余計なメンバ(構造体)が追加されます。追加される構造体自体は0なのですが、C++では規格上メンバを持たない構造体は1バイトになり、更にアライメントで4バイトくらいまで引き上げられるので、結果としてサイズが大きくなるけど愛さえあれば関係ないよね!

## よくありそうな質問
Q\. なんで作ったの？  
A\. メタメタしたコードが書きたかったからです。  
  
Q\. バグってるんだけど？  
A\. すんません。  

Q\. ○○できないの？  
A\. できません。すんません。

Q\. いろいろひどいんだけど  
A\. すんません。

## ライセンス
nanojsonはlibpng/zibライセンス下で頒布されています。  
This software is released under the zlib License, see LICENSE.txt.

### PicoJSON
nanojsonのJSONパース部分には[PicoJSON](https://github.com/kazuho/picojson)を使用しています。  
nanojsonのリポジトリにはPicoJSONが同梱されていますが、最新版であるとは限りません。  
  
PicoJSON - Copyright © 2009-2010 Cybozu Labs, Inc. Copyright © 2011 Kazuho Oku  
licensed under the new BSD License

