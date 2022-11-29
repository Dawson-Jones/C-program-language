TARGET := mytbf
TARGET_C := ${TARGET:=.c}
TARGET_O := ${TARGET:=.o}

LIBTARGET := $(join lib, $(TARGET))

STATIC_TARGET = ${LIBTARGET:=.a}
SHARED_TARGET = ${LIBTARGET:=.so}

all: $(SHARED_TARGET) $(STATIC_TARGET) $(TARGET_O)

CFLAGS += -Wall

$(STATIC_TARGET): $(TARGET_O)
	ar -crv $@ $<

$(SHARED_TARGET): $(TARGET_O)
	gcc $< -shared -o $@

$(TARGET_O): $(TARGET_C)
	gcc $^ -fPIC $(CFLAGS) -c -o $@

clean:
	rm -rf *.o $(STATIC_TARGET) $(SHARED_TARGET)