set(clang_tidy_flags_warnings
--warnings-as-errors=*
-cppcoreguidelines-avoid-magic-numbers
-readability-magic-numbers
-altera-struct-pack-align
-cppcoreguidelines-non-private-member-variables-in-classes
-misc-non-private-member-variables-in-classes
-abseil-string-find-str-contains
)

set(clang_tidy_flags_checks
--checks=*
-cppcoreguidelines-pro-type-vararg
-fuchsia-default-arguments-calls
-hicpp-vararg
-llvm-include-order
-fuchsia-overloaded-operator
-cert-err58-cpp
-modernize-use-trailing-return-type
-llvmlibc-*
-cppcoreguidelines-avoid-goto
-hicpp-avoid-goto
-fuchsia-default-arguments-declarations
)

string(JOIN "," clang_tidy_flags_warnings ${clang_tidy_flags_warnings})
string(JOIN "," clang_tidy_flags_checks ${clang_tidy_flags_checks})

set(TOOLING_CLANG_TIDY_PARAMS "${clang_tidy_flags_warnings};${clang_tidy_flags_checks}")
