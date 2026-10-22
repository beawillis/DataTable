#ifndef DATATABLE_TEST_ASSERT_HPP
#define DATATABLE_TEST_ASSERT_HPP

// Assertions are the test checks, so keep them active in Release builds too.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

#endif // DATATABLE_TEST_ASSERT_HPP
