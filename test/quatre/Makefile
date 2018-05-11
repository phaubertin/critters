TARGETS     = tree-1 tree-2 tree-3 tree-4 tree-5

include		= ../../include
src			= ../../src
under_test	= $(src)

CFLAGS      = -DQRT_CONFIG_TREE_KEY_TYPE=double -I$(include) -Wall -ansi -pedantic -Werror=implicit -Werror=implicit-function-declaration -Werror=uninitialized -Werror=return-type
LDFLAGS     = 

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	-rm -f *.o $(TARGETS)

.PHONY: run
run: $(TARGETS)
	for f in $(TARGETS) ; do \
		./$$f ; \
	done

.PHONY: run-%
run-%: %
	./$<


tree-1: tree-1.o $(under_test)/tree.o

tree-2: tree-2.o $(under_test)/tree.o

tree-3: tree-3.o $(under_test)/tree.o

tree-4: tree-4.o $(under_test)/tree.o

tree-5: tree-5.o $(under_test)/tree.o

$(under_test)/%.o:
	make -C $(under_test) $(notdir $@)


