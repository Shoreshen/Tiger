# Variables =====================================================================================
PHONY 		= 
CFLAGS		= -g -m64
CH1_FILES	= $(shell ls ./ch1/*.c)
# Run ===========================================================================================
./ch1/ch1.out:$(CH1_FILES)
	gcc $(CFLAGS) ./ch1/*.c -o ./ch1/ch1.out
run_ch1:./ch1/ch1.out
	./ch1/ch1.out

PHONY += ch1
# Clean =========================================================================================
clean:
	rm ./ch1/*.out
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