
CC = gcc
CFLAGS = -pthread -O2 -g -Wall -Wextra
CFLAGS_OPT = -pthread -O3 -march=native -Wall -Wextra
LIBS = -lm

SOURCES = my_rwlock.c my_rand.c pth_ll_rwl_custom.c
HEADERS = my_rwlock.h my_rand.h timer.h

CUSTOM_TARGET = pth_ll_custom
STDLIB_TARGET = pth_ll_stdlib
ALL_TARGETS = $(CUSTOM_TARGET) $(STDLIB_TARGET)


all: $(ALL_TARGETS)

# Версия с пользовательской реализацией RWLock
$(CUSTOM_TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DUSE_CUSTOM -o $@ $(SOURCES) $(LIBS)
	@echo "✓ Custom RWLock built successfully: $@"

# Версия со стандартной pthread_rwlock_t
$(STDLIB_TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LIBS)
	@echo "✓ Standard pthread_rwlock_t built successfully: $@"

# Оптимизированные версии
custom-opt: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS_OPT) -DUSE_CUSTOM -o $(CUSTOM_TARGET)_opt $(SOURCES) $(LIBS)
	@echo "✓ Optimized custom RWLock built: $(CUSTOM_TARGET)_opt"

stdlib-opt: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS_OPT) -o $(STDLIB_TARGET)_opt $(SOURCES) $(LIBS)
	@echo "✓ Optimized standard pthread_rwlock_t built: $(STDLIB_TARGET)_opt"

custom: $(CUSTOM_TARGET)
stdlib: $(STDLIB_TARGET)


clean:
	rm -f $(ALL_TARGETS) $(CUSTOM_TARGET)_opt $(STDLIB_TARGET)_opt
	rm -f *.o core
	@echo "✓ Cleaned"


test-small:
	@echo "=== Running small test (4 threads, 10000 ops) ==="
	@echo "100\n10000\n0.8\n0.1\n" | ./$(CUSTOM_TARGET) 4 || true
	@echo ""
	@echo "=== Same test with stdlib ==="
	@echo "100\n10000\n0.8\n0.1\n" | ./$(STDLIB_TARGET) 4 || true

test-medium:
	@echo "=== Running medium test (4 threads, 100000 ops) ==="
	@echo "500\n100000\n0.8\n0.1\n" | ./$(CUSTOM_TARGET) 4 || true
	@echo ""
	@echo "=== Same test with stdlib ==="
	@echo "500\n100000\n0.8\n0.1\n" | ./$(STDLIB_TARGET) 4 || true

test-read-heavy:
	@echo "=== Read-heavy workload (90% reads, 5% inserts) ==="
	@echo "500\n100000\n0.9\n0.05\n" | ./$(CUSTOM_TARGET) 4 || true
	@echo ""
	@echo "=== Same with stdlib ==="
	@echo "500\n100000\n0.9\n0.05\n" | ./$(STDLIB_TARGET) 4 || true

test-balanced:
	@echo "=== Balanced workload (33% each operation) ==="
	@echo "500\n100000\n0.33\n0.33\n" | ./$(CUSTOM_TARGET) 4 || true
	@echo ""
	@echo "=== Same with stdlib ==="
	@echo "500\n100000\n0.33\n0.33\n" | ./$(STDLIB_TARGET) 4 || true


benchmark: all
	@echo "========================================="
	@echo "RWLock Comprehensive Benchmark"
	@echo "========================================="
	@echo ""
	@make test-small
	@echo ""
	@echo "========================================="
	@echo ""
	@make test-medium
	@echo ""
	@echo "========================================="
	@echo ""
	@make test-read-heavy
	@echo ""
	@echo "========================================="
	@echo ""
	@make test-balanced


help:
	@echo "Available targets:"
	@echo "  make              - build both versions"
	@echo "  make custom       - build custom RWLock only"
	@echo "  make stdlib       - build stdlib version only"
	@echo "  make custom-opt   - build custom RWLock optimized"
	@echo "  make stdlib-opt   - build stdlib version optimized"
	@echo "  make test-small   - run small test"
	@echo "  make test-medium  - run medium test"
	@echo "  make test-read-heavy - run read-heavy workload"
	@echo "  make test-balanced - run balanced workload"
	@echo "  make benchmark    - run full benchmark suite"
	@echo "  make clean        - remove compiled binaries"
	@echo "  make help         - show this help"

.PHONY: all custom stdlib custom-opt stdlib-opt clean test-small test-medium test-read-heavy test-balanced benchmark help
