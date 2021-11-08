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
# Clean =========================================================================================
clean:
	-rm *.out
	-rm ./ch2/*.yy.c
	-rm ./ch3/*.yy.c ./ch3/*.tab.*
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