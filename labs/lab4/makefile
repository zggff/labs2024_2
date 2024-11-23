HEADERS=$(dir $(wildcard 0*/)) $(dir $(wildcard 1*/))

build: 
	@for folder in ${HEADERS}; do 			 	\
		if [[ -a "$$folder/makefile" ]]; then	\
			echo "Building" $${folder%/};		\
			make -C $$folder build;				\
		fi 										\
	done

test: 
	@for folder in ${HEADERS}; do 			 	\
		if [[ -a "$$folder/makefile" ]]; then	\
			echo "Testing" $${folder%/};		\
			make -C $$folder test;				\
		fi 										\
	done

clean: 
	@for folder in ${HEADERS}; do 			 	\
		if [[ -a "$$folder/makefile" ]]; then	\
			echo "Cleaning" $${folder%/};		\
			make -C $$folder clean;				\
		fi 										\
	done
		
