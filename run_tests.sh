#!/bin/bash

# SIL Test Runner
# This script runs various edge case tests on the SIL interpreter

echo "=== SIL Interpreter Bug Test Suite ==="
echo ""

# Compile the program
gcc -o sil /Users/akr/Documents/SIL/Main.c
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed"
    exit 1
fi
echo "✓ Compilation successful"
echo ""

# Create test results directory
mkdir -p /Users/akr/Documents/SIL/test_results

# Test 1: Basic number parsing
echo "Test 1: Number parsing (10, 123, 0, 999)"
echo -e "let x\nx = 10\nput x\nx = 123\nput x\nx = 0\nput x\nx = 999\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test1.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test1.txt
echo ""

# Test 2: Expression parsing with single variable
echo "Test 2: Expression - x = x + 1"
echo -e "let x\nx = 10\nput x\nx = x + 1\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test2.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test2.txt
echo ""

# Test 3: Expression with two variables
echo "Test 3: Expression - x = y + z"
echo -e "let x\nlet y\nlet z\ny = 10\nz = 5\nx = y + z\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test3.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test3.txt
echo ""

# Test 4: Multiplication
echo "Test 4: Multiplication - x = x * 2"
echo -e "let x\nx = 10\nput x\nx = x * 2\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test4.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test4.txt
echo ""

# Test 5: Division
echo "Test 5: Division - x = x / 2"
echo -e "let x\nx = 20\nput x\nx = x / 2\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test5.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test5.txt
echo ""

# Test 6: Subtraction
echo "Test 6: Subtraction - x = x - 5"
echo -e "let x\nx = 20\nput x\nx = x - 5\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test6.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test6.txt
echo ""

# Test 7: Complex expression
echo "Test 7: Complex - x = a + b * c"
echo -e "let x\nlet a\nlet b\nlet c\na = 2\nb = 3\nc = 4\nx = a + b * c\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test7.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test7.txt
echo ""

# Test 8: Parentheses
echo "Test 8: Parentheses - x = (a + b) * c"
echo -e "let x\nlet a\nlet b\nlet c\na = 2\nb = 3\nc = 4\nx = (a + b) * c\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test8.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test8.txt
echo ""

# Test 9: String output
echo "Test 9: String output"
echo -e 'put "Hello World\\n"\nstop' | ./sil > /Users/akr/Documents/SIL/test_results/test9.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test9.txt
echo ""

# Test 10: Kill variable
echo "Test 10: Kill variable"
echo -e "let x\nx = 10\nput x\nkill x\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test10.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test10.txt
echo ""

# Test 11: Multiple comma-separated lets
echo "Test 11: Multiple lets - let a, b, c"
echo -e "let a, b, c\na = 1\nb = 2\nc = 3\nput a\nput b\nput c\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test11.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test11.txt
echo ""

# Test 12: Multiple puts
echo "Test 12: Multiple puts - put a, b, c"
echo -e "let a\nlet b\na = 5\nb = 10\nput a, b\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test12.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test12.txt
echo ""

# Test 13: Edge case - zero
echo "Test 13: Zero value"
echo -e "let x\nx = 0\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test13.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test13.txt
echo ""

# Test 14: Edge case - negative result
echo "Test 14: Negative result"
echo -e "let x\nx = 5\nx = x - 10\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test14.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test14.txt
echo ""

# Test 15: Variable name edge cases
echo "Test 15: Variable name with numbers"
echo -e "let var1\nvar1 = 99\nput var1\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test15.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test15.txt
echo ""

# Test 16: Error - variable doesn't exist
echo "Test 16: Error - undefined variable"
echo -e "put x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test16.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test16.txt
echo ""

# Test 17: Error - variable already exists
echo "Test 17: Error - duplicate variable"
echo -e "let x\nlet x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test17.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test17.txt
echo ""

# Test 18: Error - variable name too long
echo "Test 18: Error - variable name too long (>14 chars)"
echo -e "let abcdefghijklmnop\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test18.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test18.txt
echo ""

# Test 19: Consecutive operations
echo "Test 19: Consecutive operations"
echo -e "let x\nx = 1\nx = x + 1\nx = x + 1\nx = x + 1\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test19.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test19.txt
echo ""

# Test 20: Division by zero
echo "Test 20: Division by zero"
echo -e "let x\nlet y\nx = 10\ny = 0\nx = x / y\nput x\nstop" | ./sil > /Users/akr/Documents/SIL/test_results/test20.txt 2>&1
echo "Output:"
cat /Users/akr/Documents/SIL/test_results/test20.txt
echo ""

echo ""
echo "=== All tests completed ==="
echo "Results saved in /Users/akr/Documents/SIL/test_results/"
