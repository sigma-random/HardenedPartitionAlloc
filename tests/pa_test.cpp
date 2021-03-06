// This serves as an example of how to use the various
// C/C++ APIs that ship with Hardened PartitionAlloc
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "../PartitionAlloc.h"

class MyClass : public PartitionBackedBase {
  public:
	MyClass() {
		ptr = NULL;
		idx = 0;
	}

	~MyClass() { }

	void setPtr(char *s) {
		ptr = s;
	}
	char *getPtr() {
		return ptr;
	}

	void setIdx(int i) {
		idx = i;
	}

	int getIdx() {
		return idx;
	}

  private:
	char *ptr;
	int idx;
};

typedef struct MyClassProxy {
	MyClassProxy() {
		m = new MyClass();
	}

	~MyClassProxy() {
		delete m;
	}

	MyClass *m;

    MyClass *operator->() {
        check_partition_pointer(m);
        return m;
    }

} MyClassProxy;


void run_test() {
	// PartitionAlloc API test with global root
	PartitionAllocatorGeneric my_partition;
	my_partition.init();
	void *p = partitionAllocGeneric(my_partition.root(), 16);
	partitionFreeGeneric(my_partition.root(), p);
	my_partition.shutdown();

	for(int i = 0; i < 512; i++) {
		p = partition_malloc_sz(64);
		ASSERT(p);
		partition_free_sz(p);
	}

	for(int i = 0; i < 512; i++) {
		p = partition_malloc_sz(128);
		ASSERT(p);
		partition_free_sz(p);
	}

	for(int i = 0; i < 512; i++) {
		p = partition_malloc_sz(256);
		ASSERT(p);
		partition_free_sz(p);
	}

	for(int i = 0; i < 32; i++) {
		p = partition_malloc_sz(512);
		ASSERT(p);
		partition_free_sz(p);
	}

	p = partition_malloc_string(128);
	ASSERT(p);
	check_partition_pointer(p);
	p = partition_realloc_string(p, 128);
	ASSERT(p);
	partition_free_string(p);

	p = partition_malloc(512);
	ASSERT(p);
	p = partition_realloc(p, 550);
	ASSERT(p);
	partition_free(p);

	// Create a new MyClass which inherits from PartitionBackedBase
	// which overloads the new operator
	MyClass *mc = new MyClass();
	ASSERT(mc);
	mc->setIdx(1234);
	check_partition_pointer(mc);
	delete mc;

	void *gp = new_generic_partition();
	p = generic_partition_alloc(gp, 128);
	ASSERT(p);

	// Create a proxy and call a method, which
	// will trigger a call to check_partition_pointer
	MyClassProxy j;
	j->setIdx(100);

	p = generic_partition_realloc(gp, p, 256);
	ASSERT(p);
	generic_partition_free(gp, p);
	delete_generic_partition(gp);
}

int main(int argc, char *argv[]) {
	// Initialize the C API by calling _init() which will
	// make sure all generic partitions are initialized
	partitionalloc_init();

	// Run all tests
	run_test();

	// Shutdown all generic partitions
	partitionalloc_shutdown();

	return 0;
}
