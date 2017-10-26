CC      = mpic++
CFLAGS  = -g #-I${HOME}/trng-4.15/
OFLAGS  = -O3 # O3 ist nicht viel schneller und prod. Compilerwarnungen
GFLAGS  = -g
WFLAGS  = -Wall -Wextra -std=c++11
#DBFLAGS = -g  -pg # debugflag
#LDFLAGS =  -lm  -lstdc++ -ltrng4



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

