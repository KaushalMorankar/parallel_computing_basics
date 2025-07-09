# Parallel & Systems Programming

A collection of small projects demonstrating multithreading with Pthreads and low-level RISC-V assembly.

## Projects

1. **Parallel BFS Traversal**  
   – Breadth-First Search on an adjacency-list graph.  
   – Work is spread across threads to explore frontier levels in parallel.

2. **Multithreaded Sudoku Verifier**  
   – Validates a 9×9 Sudoku solution.  
   – Spawns 27 threads: 9 for rows, 9 for columns, 9 for 3×3 subgrids.

3. **RISC-V Matrix Multiplication**  
   – Computes C = A×B in assembly (RV64I), respecting caller/callee conventions.  
   – Demonstrates stack frame setup, register preservation, and loop unrolling.


