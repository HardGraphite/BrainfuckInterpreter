INTERPRETER = ../${BF_INTERPRETER}
TEST_SRC = ${wildcard *.bf}

.PHONY: test
test: ${INTERPRETER}
	${foreach SRC, ${TEST_SRC}, \
		@echo -e "\x1b[34mtesting" ${SRC} "...\x1b[0m"; \
		${INTERPRETER} ${SRC} ; \
		echo -e "\x1b[34mexit code is $$?\x1b[0m.\n"; \
	}
