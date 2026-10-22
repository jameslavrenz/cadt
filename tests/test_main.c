#include "test.h"

int g_failed = 0;
int g_passed = 0;

int main(void)
{
    CHECK(strcmp(cadt_version(), "1.0.0") == 0);
    CHECK(cadt_status_str(CADT_OK) != nullptr);
    CHECK(cadt_status_str(CADT_ERR_NOMEM) != nullptr);
    CHECK(cadt_hash_bytes("a", 1) != cadt_hash_bytes("b", 1));

    test_vector();
    test_deque();
    test_list();
    test_forward_list();
    test_array();
    test_adapters();
    test_set();
    test_map();
    test_unordered();
    test_string();
    test_bitset();
    test_optional();
    test_algo();
    test_span();

    printf("%d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
