clang -O0 -S -emit-llvm  /tests/test-example/test1.cpp -o /tmp/test.ll
opt -load 231_solution.so -cse231-maypointto < /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-maypointto < /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
opt -load 231_solution.so -cse231-liveness< /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-liveness< /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
clang -O0 -S -emit-llvm  /tests/test-example/test2.c -o /tmp/test.ll
opt -load 231_solution.so -cse231-maypointto < /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-maypointto < /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
opt -load 231_solution.so -cse231-liveness< /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-liveness< /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
clang -O0 -S -emit-llvm  /tests/test-example/test1.c -o /tmp/test.ll
opt -load 231_solution.so -cse231-maypointto < /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-maypointto < /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
opt -load 231_solution.so -cse231-liveness< /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-liveness< /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
clang -O0 -S -emit-llvm  /tests/test-example/test1-main.cpp -o /tmp/test.ll
opt -load 231_solution.so -cse231-maypointto < /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-maypointto < /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
opt -load 231_solution.so -cse231-liveness< /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-liveness< /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
clang -O0 -S -emit-llvm  /tests/conditionalSum/main.cpp -o /tmp/test.ll
opt -load 231_solution.so -cse231-maypointto < /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-maypointto < /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
opt -load 231_solution.so -cse231-liveness< /tmp/test.ll 2> /tmp/ans
opt -load submission_pt3.so -cse231-liveness< /tmp/test.ll 2> /tmp/out
diff /tmp/ans /tmp/out
