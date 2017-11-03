CC      = g++
CFLAGS  = -g
OFLAGS  = -O3 
WFLAGS  = -Wall -Wextra -std=c++11 -pedantic
#DBFLAGS = -g  -pg # debugflag
#LDFLAGS =  -lm  -lstdc++



TARGETDIR = RUN

OBJ =	\
	main.o			\
	lgq1d.o			\



lg:  $(OBJ)
	$(CC) $(OFLAGS) $(DBFLAGS) $(CFLAGS) $(WFLAGS) -o LG $(OBJ) $(LDFLAGS)
#	mv $(OBJ) src/


%.o: %.cpp
	$(CC) $(OFLAGS) $(DBFLAGS) $(CFLAGS) $(WFLAGS) -c $< -o $@

style:
	. /etc/profile.d/modules.sh; \
	module load llvm; \
	clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4}" -i *.cpp *.h

.PHONY: clean
clean:
	rm $(OBJ) LG -rf

