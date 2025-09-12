CC := g++
CFLAGS := -g -O0 -std=c++17
LDFLAGS :=
LIBS := -lpthread -lmysqlcppconnx -lsodium -lcrypto

SRC := $(wildcard *.cpp)
INCLUDE := -I/usr/include/mysql-cppconn -Inlohmann
INCLUDE += -Ijwt-cpp/include

OBJ := $(patsubst %.cpp, %.o, $(SRC))

target := TodoApp

all: $(target)
	@echo "Build Done"

$(target) : $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

%.o : %.cpp %.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

httpserver.o : httpserver.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

.PHONY: clean

clean:
	-rm -rf $(target) $(OBJ)
