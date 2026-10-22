# cadt — C Abstract Data Types

cadt is a C23 library of generic containers and algorithms in the spirit of the
C++ standard library. C has no templates, so each container stores values
described by a `cadt_type`: the element size, and optional copy, destroy,
compare, and hash callbacks.

| cadt | C++ counterpart |
| --- | --- |
| `cadt_vector` | `std::vector` |
| `cadt_deque` | `std::deque` |
| `cadt_list` | `std::list` |
| `cadt_flist` | `std::forward_list` |
| `cadt_array` | `std::array` |
| `cadt_stack` | `std::stack` |
| `cadt_queue` | `std::queue` |
| `cadt_pqueue` | `std::priority_queue` |
| `cadt_set`, `cadt_multiset` | `std::set`, `std::multiset` |
| `cadt_map`, `cadt_multimap` | `std::map`, `std::multimap` |
| `cadt_uset`, `cadt_umultiset` | `std::unordered_set`, `std::unordered_multiset` |
| `cadt_umap`, `cadt_umultimap` | `std::unordered_map`, `std::unordered_multimap` |
| `cadt_string` | `std::string` |
| `cadt_bitset` | `std::bitset` |
| `cadt_optional` | `std::optional` |
| `cadt_span` | `std::span` |
| `cadt_sort`, `cadt_lower_bound`, … | `<algorithm>` |

Ordered sets and maps are red-black trees. Unordered containers are chained
hash tables. The priority queue is a binary heap. `cadt_sort` is an introsort;
`cadt_stable_sort` is a mergesort.

## Build

cadt requires ISO C23: GCC 14, Clang 18, or newer. The CMake build needs CMake 3.21 or newer, which is the first release that accepts `CMAKE_C_STANDARD 23`.

```sh
make
make test
```

`make test` builds with AddressSanitizer and UndefinedBehaviorSanitizer,
runs the unit tests, and runs the examples. A static library is produced at
`build/libcadt.a`.

CMake works as well:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Install headers and the static library with `make install` or the CMake
install target. The default prefix is `/usr/local`.

## Example

```c
#include "cadt.h"
#include <stdio.h>

static int cmp_int(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

int main(void)
{
    cadt_type type = {.size = sizeof(int), .compare = cmp_int};
    cadt_vector *vec = cadt_vector_create(&type, nullptr);
    int i;

    for (i = 0; i < 5; i++) {
        cadt_vector_push_back(vec, &i);
    }
    cadt_sort(cadt_vector_data(vec), cadt_vector_size(vec), sizeof(int),
              cmp_int, nullptr);
    for (i = 0; i < 5; i++) {
        printf("%d\n", *(int *)cadt_vector_at(vec, (size_t)i));
    }
    cadt_vector_destroy(vec);
    return 0;
}
```

`examples/demo.c` walks several containers. `examples/word_count.c` counts
words on standard input with an unordered map of owned strings.

## Ownership

Containers copy elements in and own those copies.

* A `nullptr` copy callback copies bytes with `memcpy`. Use that for plain values.
* A copy callback that allocates must have a matching destroy callback. The
  container calls destroy when it drops an element. Pop and erase can instead
  move the element out to the caller, and then destroy is not called.
* Ordered containers and the priority queue require `compare`. Byte order is
  not numeric order. `compare` returns negative, zero, or positive, as `memcmp`
  does.
* A `nullptr` hash callback hashes the raw key bytes with FNV-1a. Equal keys must
  hash equal.
* Pass `nullptr` as the allocator to use `malloc` / `realloc` / `free`. A custom
  `cadt_allocator` must outlive every container created with it.

Mutations that grow a vector or deque invalidate outstanding element pointers.
List, set, and map iterators stay valid except for the erased element. Rehash
invalidates unordered-container iterators.

Functions that can fail return a `cadt_status`. `cadt_status_str` names the
code. Destroying `nullptr` is a no-op.

## Layout

Public declarations live in `include/cadt/`. `include/cadt.h` includes all of
them. Implementation files are in `src/`. The red-black tree and hash table
are internal.

## License

MIT. See [LICENSE](LICENSE).
