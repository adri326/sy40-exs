cmake_minimum_required(VERSION 3.15)

add_definitions(-DBUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

project(PereFils)
add_executable(PereFils src/tp1/PereFils.c)

project(PereFils2)
add_executable(PereFils2 src/tp1/PereFils2.c)

project(Prg2ter)
add_executable(Prg2ter src/tp1/Prg2ter.c)

project(Prg1)
add_executable(Prg1 src/tp1/Prg1.c)

project(child_exec)
add_executable(child_exec src/tp2/child_exec.c)
project(child_exec_c)
add_executable(child_exec_c src/tp2/child_exec_c.c)

project(prg3_avant)
add_executable(prg3_avant src/tp2/prg3_avant.c)
project(prg3_apres)
add_executable(prg3_apres src/tp2/prg3_apres.c)

project(fork_exec)
project(fork_exec_child)
add_executable(fork_exec src/tp2/fork_exec.c)
target_compile_definitions(fork_exec PUBLIC -DPARENT)
add_executable(fork_exec_child src/tp2/fork_exec.c)

project(dual_fork)
add_executable(dual_fork src/tp2/dual_fork.c)

project(td3)
add_executable(td3 src/td3/main.c)

project(tp3_prg0)
add_executable(tp3_prg0 src/tp3/prg0.c)

project(tp3_prg1)
add_executable(tp3_prg1 src/tp3/prg1.c)

project(prg2Ter)
project(prg2TerFils)
add_executable(prg2Ter src/tp3/prg2Ter.c)
target_compile_definitions(prg2Ter PUBLIC -DPARENT)
add_executable(prg2TerFils src/tp3/prg2Ter.c)

project(tp3_prg3)
add_executable(tp3_prg3 src/tp3/prg3.c)

project(tp3_prg4)
add_executable(tp3_prg4 src/tp3/prg4.c)

project(tp4_prg1)
add_executable(tp4_prg1 src/tp4/prg1.c)

project(tp4_prg3)
add_executable(tp4_prg3 src/tp4/prg3.c)

project(tp4_prg4)
add_executable(tp4_prg4 src/tp4/prg4.c)

project(tp4_prg4bis)
add_executable(tp4_prg4bis src/tp4/prg4bis.c)

project(tp4_prg5)
add_executable(tp4_prg5 src/tp4/prg5.c)

project(tp4_sleep)
add_executable(tp4_sleep src/tp4/sleep.c)

project(port)
add_executable(port src/port.c)

project(tp5_p1)
project(tp5_p2)
project(tp5_p3)
add_executable(tp5_p1 src/tp5/prg1.c)
add_executable(tp5_p2 src/tp5/prg1.c)
target_compile_definitions(tp5_p2 PUBLIC -DP2)
add_executable(tp5_p3 src/tp5/prg1.c)
target_compile_definitions(tp5_p3 PUBLIC -DP3)

project(tp5_prg2)
add_executable(tp5_prg2 src/tp5/prg2.c)
