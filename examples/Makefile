subdirs=$(shell find . -mindepth 1 -maxdepth 1 -type d)

clean:
	@for d in $(subdirs); do 	\
		echo $$d; 				\
		make -C $$d $@;  		\
		done