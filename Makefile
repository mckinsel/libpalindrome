CFLAGS=-g -O3 -Wall -Wextra -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS= $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libpalindrome.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET) tests

dev: CFLAGS=-g2 -pg -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
		ar rcs $@ $(OBJECTS)
		ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
		$(CC) -shared -o $@ $(OBJECTS)

build:
		@mkdir -p build
#		@mkdir -p bin

.PHONY: tests
tests: LDLIBS += $(SO_TARGET)
tests: $(TESTS)
		sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --leak-check=full --log-file=tests/valgrind-%p.log" $(MAKE)

clean:
		rm -rf build bin $(OBJECTS) $(TESTS)
		rm -f tests/tests.log
		rm -f tests/valgrind-*.log
		find . -name "*.gc*" -exec rm {} \;
		rm -rf `find . -name "*.dSYM" -print`

install: all
		install -d $(DESTDIR)/$(PREFIX)/lib/
		install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/
