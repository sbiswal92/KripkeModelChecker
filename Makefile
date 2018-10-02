all: mctool

DEPS=model.h
OBJS=parser.o model.o

%.o: %.cpp $(DEPS)
	g++ -ggdb -Wall -c -o $@ $<

mctool: $(OBJS)
	g++ -o $@ $^

.PHONY: clean 

clean:
	rm -f mctool *.o

tar:
	tar czvf cpp.tgz .
