# playSDwav
micropythonでSDカードに保存したwavファイルを再生します。以下のサイトを参考にしました。
>http://electroniqueamateur.blogspot.com/2022/08/jouer-de-la-musique-fichiers-wav-avec.html

上記のサイトのコードをそのまま利用する場合、System Volume Informationフォルダをwavファイルとして再生しようとしてエラーを出します。直接ファイル名を指定して下さい。  
再生時間が長いwavファイルは
~~~
WAV sub chunk 2 ID not found
~~~
このようなエラーが出ます。
