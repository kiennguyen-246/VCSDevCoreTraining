run:
	g++ -std=c++23 -g -municode -Wall main.cpp -o grepMT.exe
	./grepMT click text1.txt text2.txt text3.txt text4.txt text5.txt text6.txt text7.txt text8.txt text9.txt text10.txt

run_no_threads:
	g++ -std=c++23 -g -municode -Wall noThreads.cpp -o grepMT.exe
	./grepMT click text1.txt text2.txt text3.txt text4.txt text5.txt text6.txt text7.txt text8.txt text9.txt text10.txt

run_Win_threads:
	g++ -std=c++23 -g -municode -Wall WinThreads.cpp -o grepMT.exe
	./grepMT click text1.txt text2.txt text3.txt text4.txt text5.txt text6.txt text7.txt text8.txt text9.txt text10.txt

run_spam:
	g++ -std=c++23 -g -municode -Wall spam.cpp -o spam.exe
	./spam text.txt 10

run_test:
	del test.out
	g++ -std=c++23 -g -municode -Wall test.cpp -o test.exe
	./test