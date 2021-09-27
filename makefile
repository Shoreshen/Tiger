# Variables =====================================================================================
PHONY 		= 
CFLAGS		= -g
# Run ===========================================================================================
ch1:
	gcc $(CFLAGS) ./ch1/*.c -o ./ch1/ch1.out

PHONY += ch1
# Clean =========================================================================================
clean:
	rm ./ch1/*.out
PHONY += clean
# GitHub ========================================================================================
commit: clean
	git add -A
	@echo "Please type in commit comment: "; \
	read comment; \
	git commit -m"$$comment"
sync: commit 
	git push -u origin master

PHONY += commit sync
# PHONY =========================================================================================
.PHONY: $(PHONY)