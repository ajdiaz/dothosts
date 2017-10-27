BINS = libdothosts.so test
LDFLAGS = -ldl

libdothosts.so := dothosts.o

CFLAGS_test = -DUSE_TEST
test := test.o

