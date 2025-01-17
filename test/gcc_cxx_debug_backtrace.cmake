# Copyright (c) 2025 Pyarelal Knowles, MIT License

set(GCC_DEBUG_BACKTRACE_SUPPORTED false)

# Try C++20 with no extra libraries
if(NOT GCC_DEBUG_BACKTRACE_SUPPORTED)
  try_compile(
    DEBUG_BACKTRACE_JUSTWORKS SOURCE_FROM_CONTENT
    stdc++_libbacktrace_test.cpp
    "#include <vector>\nint main() { std::vector<int> v{0}; return v[0]; }"
    COMPILE_DEFINITIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_BACKTRACE
                        CXX_STANDARD 20 CXX_STANDARD_REQUIRED true)
  if(DEBUG_BACKTRACE_JUSTWORKS)
    set(GCC_DEBUG_BACKTRACE_SUPPORTED true)
    set(GCC_DEBUG_BACKTRACE_LIBRARY)
    set(GCC_DEBUG_BACKTRACE_CXX_FEATURE)
  endif()
endif()

# Try C++23 with no extra libraries
if(NOT GCC_DEBUG_BACKTRACE_SUPPORTED)
  try_compile(
    DEBUG_BACKTRACE_NEEDCXX23 SOURCE_FROM_CONTENT
    stdc++_libbacktrace_test.cpp
    "#include <vector>\nint main() { std::vector<int> v{0}; return v[0]; }"
    COMPILE_DEFINITIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_BACKTRACE
                        CXX_STANDARD 23 CXX_STANDARD_REQUIRED true)
  if(DEBUG_BACKTRACE_NEEDCXX23)
    set(GCC_DEBUG_BACKTRACE_SUPPORTED true)
    set(GCC_DEBUG_BACKTRACE_LIBRARY)
    set(GCC_DEBUG_BACKTRACE_CXX_FEATURE cxx_std_23)
  endif()
endif()

# Try with libstdc++exp
if(NOT GCC_DEBUG_BACKTRACE_SUPPORTED)
  try_compile(
    DEBUG_BACKTRACE_NEEDSTDEXP SOURCE_FROM_CONTENT
    stdc++_libbacktrace_test.cpp
    "#include <vector>\nint main() { std::vector<int> v{0}; return v[0]; }"
    COMPILE_DEFINITIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_BACKTRACE CXX_STANDARD
                        20 CXX_STANDARD_REQUIRED true
    LINK_LIBRARIES stdc++exp)
  if(DEBUG_BACKTRACE_NEEDSTDEXP)
    set(GCC_DEBUG_BACKTRACE_SUPPORTED true)
    set(GCC_DEBUG_BACKTRACE_LIBRARY stdc++exp)
    set(GCC_DEBUG_BACKTRACE_CXX_FEATURE)
  endif()
endif()

# Try with libstdc++_libbacktrace
if(NOT GCC_DEBUG_BACKTRACE_SUPPORTED)
  try_compile(
    DEBUG_BACKTRACE_NEEDLIBBACKTRACE SOURCE_FROM_CONTENT
    stdc++_libbacktrace_test.cpp
    "#include <vector>\nint main() { std::vector<int> v{0}; return v[0]; }"
    COMPILE_DEFINITIONS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_BACKTRACE CXX_STANDARD
                        20 CXX_STANDARD_REQUIRED true
    LINK_LIBRARIES stdc++_libbacktrace)
  if(DEBUG_BACKTRACE_NEEDLIBBACKTRACE)
    set(GCC_DEBUG_BACKTRACE_SUPPORTED true)
    set(GCC_DEBUG_BACKTRACE_LIBRARY stdc++_libbacktrace)
    set(GCC_DEBUG_BACKTRACE_CXX_FEATURE)
  endif()
endif()

# Issue a warning if none of the above worked
if(NOT GCC_DEBUG_BACKTRACE_SUPPORTED)
  message(
    WARNING "No working try_compile configs with _GLIBCXX_DEBUG_BACKTRACE found"
  )
endif()
