CFLAGS=-g -O3 -Wall -Wextra -Isrc -DNDEBUG $(OPTFLAGS)
ifneq "$(CC)" "clang"
	CFLAGS += -rdynamic
endif
LIBS= -lm $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libpalindrome.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET)

dev: CFLAGS=-g2 -pg -Wall -Wextra -Isrc $(OPTFLAGS)
dev: all

cov: CFLAGS=-g -O0 -Wall -Wextra -Isrc -fprofile-arcs -ftest-coverage
cov: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
		ar rcs $@ $(OBJECTS)
		ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
		$(CC) -shared -o $@ $(OBJECTS) $(LIBS)

build:
		@mkdir -p build
#		@mkdir -p bin

.PHONY: test
test: LDLIBS += $(SO_TARGET)
test: $(TESTS)
		sh ./tests/runtests.sh

.PHONY: test-cov
test-cov: LDLIBS += $(TARGET) -fprofile-arcs -lm
test-cov: $(TESTS)
		sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --leak-check=full --log-file=tests/valgrind-%p.log" $(MAKE) test

clean:
		rm -rf build bin $(OBJECTS) $(TESTS)
		rm -f tests/tests.log
		rm -f tests/valgrind-*.log
		find . -name "*.gc*" -exec rm {} \;
		rm -rf `find . -name "*.dSYM" -print`

install: all
		install -d $(DESTDIR)/$(PREFIX)/lib/
		install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/
