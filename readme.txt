1. 使用之程式語言：C++
2. 使用之編譯器：GNU g++
3. 各檔案說明：
	group26.ppt : 專題簡報
	intersection_manager.cpp : 程式原始碼
	im : 程式執行檔
	Makefile : 程式編譯指令檔
	readme.txt : 程式報告

4. 編譯方式說明：
	 請在程式目錄下，鍵入make指令，即可完成編譯，
	 ( Makefile下的 Optimize 指令為 -O3 )
	 在程式目錄下會產生一個名為 em 的執行檔

5. 執行、使用方式說明：

   編譯完成後，在檔案目錄下會產生一個 im 的執行檔
   執行檔的命令格式為：
   ./im [input file name] [output file name]
   例如：要讀取 input_1.txt 中的資料，並將結果輸出至 output_1.txt
   則在命令提示下鍵入
   ./im input_1.txt output_1.txt

6. 執行結果說明（說明執行結果的觀看方法，及解釋各項數據等）：
	 程式執行時會在輸出檔輸出平均等候回數儘可能小的車輛安排。
	 輸出檔將會出現在同一目錄，並覆蓋同名檔案的內容。
	 例如：執行 ./im input_1.txt output_1.txt 後，
	 程式會在 output_1.txt 中輸出
	 N: 00 1S 1E
	 E: 1W 00 00
	 S: 00 00 00
	 W: 00 00 00
	 
	 螢幕上會印出最小總等候回數，以及程式探索過的回合數量。例如：
	 total waiting time: 2				-- 3輛車共等候2回
	 total rounds explored: 3			-- 程式共探索了3種不同的回合
	 
	 若未指定輸入、輸出或指定的檔名不正確，則直接結束程式。