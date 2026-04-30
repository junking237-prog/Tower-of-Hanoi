# 🗼 Tower of Hanoi — Three C Implementations

A collection of three C programs that solve and visualize the **Tower of Hanoi** puzzle, each targeting a different interface: a simple console printout, an interactive terminal visualizer, and a full graphical application built with GTK 4.

---

## 📁 Repository Structure

```
tower-of-hanoi/
├── Tower_of_hanoi.c          # Version 1 – Basic console solver
├── tower_of_hanoi.c          # Version 2 – Interactive terminal visualizer
├── tower_of_hanoi_gtk4.c     # Version 3 – GTK 4 graphical visualizer
└── README.md
```

---

## 🧩 What is the Tower of Hanoi?

The Tower of Hanoi is a classic mathematical puzzle consisting of three pegs and a number of disks of different sizes. The objective is to move the entire stack from **Peg 1** to **Peg 3**, following these rules:

1. Only **one disk** may be moved at a time.
2. Each move takes the **top disk** from one peg and places it on another.
3. No disk may be placed **on top of a smaller disk**.

For `n` rings, the minimum number of moves required is **2ⁿ − 1**.

---

## 📄 Version 1 — Basic Console Solver (`Tower_of_hanoi.c`)

The simplest of the three programs. The user enters a number of rings and the program prints the full sequence of moves as plain text.

### Features
- Handles 1 to any number of rings
- Prints each move in the format `1-->3` (source peg → destination peg)
- Uses `pow(2, n) - 1` to compute the total number of moves
- No external dependencies — pure standard C (`stdio.h`, `math.h`)

### How to Compile and Run

```bash
gcc Tower_of_hanoi.c -o hanoi_basic -lm
./hanoi_basic
```

### Example Output

```
-----------Tower of Hanoi solution------------
enter the number of rings that your tower of hanoi have: 3
For 3 ring, the solution is:
 1-->2
 1-->3
 2-->3
 1-->2
 3-->1
 3-->2
 1-->2
that's what you should have done to win the tower of hanoi game.
```

---

## 🖥️ Version 2 — Interactive Terminal Visualizer (`tower_of_hanoi.c`)

A fully interactive terminal application that draws the three pegs and rings using **ANSI escape codes**. The user steps through the solution one move at a time using keyboard keys — no GTK or external library needed.

### Features
- Coloured ASCII art drawing of all three pegs and rings
- Step **forward** and **backward** through the solution (Next / Prev)
- Status bar describing the last move made
- Supports **1 to 7 rings**
- Prev / Next controls are visually dimmed when unavailable
- Restart with a new ring count at any time

### Controls

| Key | Action |
|-----|--------|
| `n` | Next move → |
| `p` | ← Previous move |
| `s` | Start a new solve (enter new ring count) |
| `q` | Quit |

### How to Compile and Run

```bash
gcc tower_of_hanoi.c -o hanoi_terminal
./hanoi_terminal
```

> **Requirements:** A terminal that supports ANSI escape codes (Linux, macOS, Windows Terminal, Git Bash).

### Screenshot

```
                        Tower  of  Hanoi

            ||                      ||                      ||
            ||                      ||                      ||
            ||                      ||                      ||
            ||                      ||                      ||
          [1]                       ||                      ||
      [====2====]                   ||                      ||
   [=======3=======]                ||                      ||
================================================================================================================
         Peg 1                   Peg 2                   Peg 3

  Ready. 3 rings need 7 moves. Press [n] to start.

  [ p ] Prev   [ n ] Next   [ s ] New solve   [ q ] Quit

  >
```

---

## 🎨 Version 3 — GTK 4 Graphical Visualizer (`tower_of_hanoi_gtk4.c`)

A graphical desktop application built with **GTK 4** and **Cairo**. It renders the pegs and coloured rings on a drawing canvas and lets the user navigate the solution with clickable buttons.

### Features
- Smooth graphical rendering with Cairo (coloured rectangles for rings)
- **Next** and **Prev** buttons to step through the solution
- Text entry to choose the number of rings (1–7) and press **Solve**
- Status label showing the current step and move description
- Buttons are automatically greyed out at the start and end of the solution
- Supports **1 to 7 rings** (up to 127 moves)

### How to Compile and Run

```bash
gcc tower_of_hanoi_gtk4.c -o hanoi_gtk $(pkg-config --cflags --libs gtk4) -lm
./hanoi_gtk
```

> **Requirements:** GTK 4 development libraries must be installed.
>
> - **Ubuntu / Debian:** `sudo apt install libgtk-4-dev`
> - **Fedora:** `sudo dnf install gtk4-devel`
> - **MSYS2 / Windows:** `pacman -S mingw-w64-ucrt-x86_64-gtk4`

---

## ⚙️ Algorithm

All three versions use the same classic **recursive algorithm** to generate the move sequence:

```c
void hanoi(int n, int from, int to, int aux) {
    if (n == 0) return;
    hanoi(n - 1, from, aux, to);   // Move n-1 rings to spare peg
    printf("%d --> %d\n", from, to); // Move the largest ring
    hanoi(n - 1, aux, to, from);   // Move n-1 rings to destination
}
```

Versions 2 and 3 additionally implement `undo_move()` to support stepping **backwards** through the solution.

---

## 📊 Comparison Table

| Feature | Version 1 | Version 2 | Version 3 |
|---|:---:|:---:|:---:|
| No external dependencies | ✅ | ✅ | ❌ |
| Graphical display | ❌ | ❌ | ✅ |
| Coloured output | ❌ | ✅ | ✅ |
| Step forward / backward | ❌ | ✅ | ✅ |
| Choose ring count interactively | ✅ | ✅ | ✅ |
| Requires GTK 4 | ❌ | ❌ | ✅ |

---

## 👤 Author

Developed as part of a software development coursework project at **PK Fokam Institut of Excellence**.

---

## 📜 License

This project is open source and available under the [MIT License](LICENSE).
