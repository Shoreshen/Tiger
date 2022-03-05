# $@  表示目标文件
# $^  表示所有的依赖文件
# $<  表示第一个依赖文件
# $?  表示比目标还要新的依赖文件列表
# Variables =====================================================================================
PHONY 		= 
CFLAGS		= -g -m64
CH1_FILES	= $(shell ls ./ch1/*.c)
BISON_FLAG	:= -d -Wcounterexamples
# ch1 ===========================================================================================
ch1.out:/ch1/*.c
	gcc $(CFLAGS) ./ch1/*.c -o $@
run_ch1:ch1.out
	./$<

PHONY += ch1
# ch2 ===========================================================================================
./ch2/tiger.yy.c:./ch2/tiger.l
	flex --outfile=$@ $<
ch2.out:./ch2/tiger.yy.c
	gcc $(CFLAGS) ./ch2/*.c -o $@
run_ch2:ch2.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch3 ===========================================================================================
./ch3/tiger.tab.c ./ch3/tiger.tab.h: ./ch3/tiger.y
	bison -d -Wcounterexamples -o ./ch3/tiger.tab.c $<
./ch3/tiger.yy.c:./ch3/tiger.tab.h ./ch3/tiger.l
	flex --outfile=$@ ./ch3/tiger.l
ch3.out:./ch3/tiger.yy.c ./ch3/tiger.tab.c ./ch3/tiger.tab.h ./ch3/*.c
	gcc $(CFLAGS) ./ch3/*.c -o $@
run_ch3:ch3.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch4 ===========================================================================================
./ch4/tiger.tab.c ./ch4/tiger.tab.h: ./ch4/tiger.y
	bison -d -Wcounterexamples -o ./ch4/tiger.tab.c $<
./ch4/tiger.yy.c:./ch4/tiger.tab.h ./ch4/tiger.l
	flex --outfile=$@ ./ch4/tiger.l
ch4.out:./ch4/tiger.yy.c ./ch4/tiger.tab.c ./ch4/tiger.tab.h ./ch4/*.c
	gcc $(CFLAGS) ./ch4/*.c -o $@
run_ch4:ch4.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch5 ===========================================================================================
./ch5/tiger.tab.c ./ch5/tiger.tab.h: ./ch5/tiger.y
	bison -d -Wcounterexamples -o ./ch5/tiger.tab.c $<
./ch5/tiger.yy.c:./ch5/tiger.tab.h ./ch5/tiger.l
	flex --outfile=$@ ./ch5/tiger.l
ch5.out:./ch5/tiger.yy.c ./ch5/tiger.tab.c ./ch5/tiger.tab.h ./ch5/*.c
	gcc $(CFLAGS) ./ch5/*.c -o $@
run_ch5:ch5.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch6 ===========================================================================================
./ch6/tiger.tab.c ./ch6/tiger.tab.h: ./ch6/tiger.y
	bison -d -Wcounterexamples -o ./ch6/tiger.tab.c $<
./ch6/tiger.yy.c:./ch6/tiger.tab.h ./ch6/tiger.l
	flex --outfile=$@ ./ch6/tiger.l
ch6.out:./ch6/tiger.yy.c ./ch6/tiger.tab.c ./ch6/tiger.tab.h ./ch6/*.c
	gcc $(CFLAGS) ./ch6/*.c -o $@
run_ch6:ch6.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch7 ===========================================================================================
./ch7/tiger.tab.c ./ch7/tiger.tab.h: ./ch7/tiger.y
	bison -d -Wcounterexamples -o ./ch7/tiger.tab.c $<
./ch7/tiger.yy.c:./ch7/tiger.tab.h ./ch7/tiger.l
	flex --outfile=$@ ./ch7/tiger.l
ch7.out:./ch7/tiger.yy.c ./ch7/tiger.tab.c ./ch7/tiger.tab.h ./ch7/*.c
	gcc $(CFLAGS) ./ch7/*.c -o $@
run_ch7:ch7.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch8 ===========================================================================================
./ch8/tiger.tab.c ./ch8/tiger.tab.h: ./ch8/tiger.y
	bison -d -Wcounterexamples -o ./ch8/tiger.tab.c $<
./ch8/tiger.yy.c:./ch8/tiger.tab.h ./ch8/tiger.l
	flex --outfile=$@ ./ch8/tiger.l
ch8.out:./ch8/tiger.yy.c ./ch8/tiger.tab.c ./ch8/tiger.tab.h ./ch8/*.c
	gcc $(CFLAGS) ./ch8/*.c -o $@
run_ch8:ch8.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# ch9 ===========================================================================================
./ch9/tiger.tab.c ./ch9/tiger.tab.h: ./ch9/tiger.y
	bison -d -Wcounterexamples -o ./ch9/tiger.tab.c $<
./ch9/tiger.yy.c:./ch9/tiger.tab.h ./ch9/tiger.l
	flex --outfile=$@ ./ch9/tiger.l
ch9.out:./ch9/tiger.yy.c ./ch9/tiger.tab.c ./ch9/tiger.tab.h ./ch9/*.c
	gcc $(CFLAGS) ./ch9/*.c -o $@
run_ch9:ch9.out
	@echo "Please type in file names: "; \
	read file; \
	./$< $$file
# Clean =========================================================================================
clean:
	-rm *.out \
	    ./ch2/*.yy.c \
	    ./ch3/*.yy.c ./ch3/*.tab.* \
	    ./ch4/*.yy.c ./ch4/*.tab.* \
	    ./ch5/*.yy.c ./ch5/*.tab.* \
	    ./ch6/*.yy.c ./ch6/*.tab.* \
	    ./ch7/*.yy.c ./ch7/*.tab.* \
	    ./ch8/*.yy.c ./ch8/*.tab.* \
		./ch9/*.yy.c ./ch9/*.tab.*
PHONY += clean
# GitHub ========================================================================================
sub_pull:
	git submodule foreach --recursive 'git pull'
commit: clean
	git add -A
	@echo "Please type in commit comment: "; \
	read comment; \
	git commit -m"$$comment"
sync: sub_pull commit 
	git push -u origin master

PHONY += commit sync
# PHONY =========================================================================================
.PHONY: $(PHONY)