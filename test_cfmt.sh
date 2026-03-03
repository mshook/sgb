#!/bin/sh
# TDD test harness for cfmt

PASS=0
FAIL=0

check() {
    name="$1"
    input="$2"
    expected="$3"
    actual=$(printf '%s\n' "$input" | ./cfmt 2>&1)
    if [ "$actual" = "$expected" ]; then
        echo "PASS: $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $name"
        echo "  --- expected ---"
        printf '%s\n' "$expected" | sed 's/^/  |/'
        echo "  --- actual ---"
        printf '%s\n' "$actual" | sed 's/^/  |/'
        FAIL=$((FAIL + 1))
    fi
}

# 1. Basic: body of a function gets indented
check "basic" \
'int main() {
int x = 5;
return x;
}' \
'int main() {
    int x = 5;
    return x;
}'

# 2. Nested braces
check "nested" \
'void f() {
if (a) {
if (b) {
c();
}
}
}' \
'void f() {
    if (a) {
        if (b) {
            c();
        }
    }
}'

# 3. } else { on one line
check "else" \
'if (x) {
a();
} else {
b();
}' \
'if (x) {
    a();
} else {
    b();
}'

# 4. Braces inside a string are not counted
check "string" \
'void f() {
printf("{hello}");
}' \
'void f() {
    printf("{hello}");
}'

# 5. Brace inside a line comment is not counted
check "line-comment" \
'void f() {
// { not a brace
x = 1;
}' \
'void f() {
    // { not a brace
    x = 1;
}'

# 6. Brace inside a block comment is not counted
check "block-comment" \
'void f() {
/* { not a brace */
x = 1;
}' \
'void f() {
    /* { not a brace */
    x = 1;
}'

# 7. Already-formatted code is unchanged (idempotent)
check "idempotent" \
'int main() {
    int x = 5;
    return x;
}' \
'int main() {
    int x = 5;
    return x;
}'

# 8. Blank lines are preserved
check "blank-lines" \
'void f() {
x = 1;

y = 2;
}' \
'void f() {
    x = 1;

    y = 2;
}'

# 9. File argument (not stdin)
tmpfile=$(mktemp /tmp/cfmt_test.XXXXXX.c)
printf 'void g() {\nz = 3;\n}\n' > "$tmpfile"
actual=$(./cfmt "$tmpfile" 2>&1)
expected='void g() {
    z = 3;
}'
if [ "$actual" = "$expected" ]; then
    echo "PASS: file-arg"
    PASS=$((PASS + 1))
else
    echo "FAIL: file-arg"
    echo "  --- expected ---"
    printf '%s\n' "$expected" | sed 's/^/  |/'
    echo "  --- actual ---"
    printf '%s\n' "$actual" | sed 's/^/  |/'
    FAIL=$((FAIL + 1))
fi
rm -f "$tmpfile"

echo ""
echo "$PASS passed, $FAIL failed."
[ "$FAIL" -eq 0 ]
