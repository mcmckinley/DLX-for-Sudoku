# DLX for Sudoku - C++

**Author**: Michael McKinley<br>
**Course**: CSCI 411 - Final Project<br>
**Instructor**: Professor Richard Carter Tillquist<br>

## Description

This project implements Donald Knuth's Dancing Links algorithm to solve Sudoku puzzles of arbitrary size and valid shape.

---

## Building the Project

To compile the project, run:

```
make
```

---

## Running Tests

**Linux:**
```
./runTests.sh
```
**MacOS:**
```
./runTestsMacOS.sh
```

> Note: You may need to grant execute permission first:

```
chmod +x runTests.sh
```

---

## Running on Custom Sudoku Files

To solve a custom Sudoku puzzle:

```
./dlx <filename>
```

### Input

* The file must contain N² integers, where N is a perfect square (e.g., 4, 9, 16).
* Each line should contain N integers.
* Whitespace is ignored, so the puzzle can be formatted neatly.

## Output

* If a valid solution is found, it is printed to the console.
* If the puzzle has no solution, an error message will be displayed.
---


## References

* Donald Knuth, "Dancing Links" ([https://arxiv.org/abs/cs/0011047](https://arxiv.org/abs/cs/0011047))

---
