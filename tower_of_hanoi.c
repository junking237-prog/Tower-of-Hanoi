/*
 * Tower of Hanoi – Terminal visualizer (no GTK)
 * ===============================================
 * A terminal program that draws the Tower of Hanoi using ANSI escape codes
 * and lets you step through the solution one move at a time.
 *
 * HOW TO COMPILE:
 *   gcc tower_of_hanoi.c -o hanoi
 *
 * HOW TO RUN:
 *   ./hanoi
 *
 * CONTROLS:
 *   n  = Next move
 *   p  = Previous move
 *   s  = Solve (enter a new number of rings)
 *   q  = Quit
 */

#include <stdio.h>    /* printf, scanf, snprintf */
#include <stdlib.h>   /* atoi                    */
#include <string.h>   /* memset                  */

/* -----------------------------------------------------------------
   SECTION 1 – GLOBAL SETTINGS
   ----------------------------------------------------------------- */

#define MAX_RINGS   7      /* maximum number of rings allowed        */
#define MAX_MOVES   128    /* 2^7 - 1 = 127 moves at most            */

/* Canvas dimensions (in character cells) */
#define CANVAS_W    72     /* total width of the drawing             */
#define CANVAS_H    12     /* height: enough rows for MAX_RINGS + base */

/* ANSI colour codes for each ring (foreground colours) */
static const char *RING_COLORS[MAX_RINGS] = {
    "\033[91m",   /* ring 1 – bright red    */
    "\033[33m",   /* ring 2 – amber/yellow  */
    "\033[92m",   /* ring 3 – bright green  */
    "\033[96m",   /* ring 4 – bright cyan   */
    "\033[94m",   /* ring 5 – bright blue   */
    "\033[95m",   /* ring 6 – bright purple */
    "\033[93m",   /* ring 7 – bright yellow */
};
#define RESET_COLOR "\033[0m"
#define COLOR_POLE  "\033[90m"   /* dark grey for poles and base */
#define COLOR_LABEL "\033[37m"   /* white for labels             */
#define COLOR_TITLE "\033[1;97m" /* bold white for title         */
#define COLOR_INFO  "\033[36m"   /* cyan for status              */
#define COLOR_DIM   "\033[2m"    /* dim for disabled buttons     */

/* Width of the widest ring (in chars, must be odd for centering) */
#define MAX_RING_W  17   /* ring 7 */
#define MIN_RING_W   3   /* ring 1 */

/* -----------------------------------------------------------------
   SECTION 2 – PROGRAM STATE
   ----------------------------------------------------------------- */

typedef struct {
    int from;
    int to;
} Move;

static int  num_rings    = 3;
static Move moves[MAX_MOVES];
static int  total_moves  = 0;
static int  current_step = 0;

/* pegs[p][0] = bottom ring, pegs[p][top[p]-1] = top ring */
static int pegs[3][MAX_RINGS];
static int top[3];

/* -----------------------------------------------------------------
   SECTION 3 – SOLVE (recursion)
   ----------------------------------------------------------------- */

static void gen_moves(int n, int from, int to, int aux)
{
    if (n == 0) return;
    gen_moves(n - 1, from, aux, to);
    moves[total_moves].from = from;
    moves[total_moves].to   = to;
    total_moves++;
    gen_moves(n - 1, aux, to, from);
}

/* -----------------------------------------------------------------
   SECTION 4 – PEG HELPERS
   ----------------------------------------------------------------- */

static void reset_pegs(void)
{
    memset(pegs, 0, sizeof(pegs));
    memset(top,  0, sizeof(top));
    for (int ring = num_rings; ring >= 1; ring--)
    {
        pegs[0][ top[0] ] = ring;
        top[0]++;
    }
}

static void do_move(int idx)
{
    int src  = moves[idx].from;
    int dest = moves[idx].to;
    top[src]--;
    int ring = pegs[src][ top[src] ];
    pegs[dest][ top[dest] ] = ring;
    top[dest]++;
}

static void undo_move(int idx)
{
    int src  = moves[idx].from;
    int dest = moves[idx].to;
    top[dest]--;
    int ring = pegs[dest][ top[dest] ];
    pegs[src][ top[src] ] = ring;
    top[src]++;
}

/* -----------------------------------------------------------------
   SECTION 5 – TERMINAL DRAWING
   ----------------------------------------------------------------- */

/* Clear the terminal screen */
static void clear_screen(void)
{
    printf("\033[2J\033[H");   /* erase screen, move cursor to top-left */
}

/*
 * ring_width() – return the character width of a given ring number.
 *   ring 1 (smallest) → MIN_RING_W
 *   ring num_rings (biggest) → MAX_RING_W
 *   Widths are always odd so the ring centres neatly on the pole.
 */
static int ring_width(int ring)
{
    if (num_rings == 1) return MIN_RING_W;
    int w = MIN_RING_W + (ring - 1) * (MAX_RING_W - MIN_RING_W) / (num_rings - 1);
    if (w % 2 == 0) w++;   /* ensure odd */
    return w;
}

/*
 * print_centered() – print 'text' of length 'len' inside a field of
 *                    total width 'field', padded with spaces.
 */
static void print_centered(const char *text, int len, int field)
{
    int left  = (field - len) / 2;
    int right = field - len - left;
    for (int i = 0; i < left;  i++) putchar(' ');
    printf("%s", text);
    for (int i = 0; i < right; i++) putchar(' ');
}

/*
 * draw_board() – draw the full Tower of Hanoi board.
 *
 * Layout: three pegs, each occupying CANVAS_W/3 = 24 chars.
 * Rows go from the top (empty sky) down to the base line.
 */
static void draw_board(void)
{
    /* Column centre positions (in chars from left of canvas) */
    int col_w  = CANVAS_W / 3;           /* width of each peg column */
    /* We render row by row, top to bottom.
       row 0 = topmost visible row (above the tallest stack)
       row rows-1 = base line */
    int rows = MAX_RINGS + 2;             /* rows of ring space + base */

    /* Title */
    printf("%s", COLOR_TITLE);
    print_centered("Tower  of  Hanoi", 16, CANVAS_W);
    printf("%s\n\n", RESET_COLOR);

    /* Draw rows top→bottom */
    for (int row = rows - 1; row >= 0; row--)
    {
        for (int p = 0; p < 3; p++)
        {
            int cx = col_w / 2;   /* centre x within this peg's column */

            if (row == 0)
            {
                /* Base row – solid line */
                printf("%s", COLOR_POLE);
                for (int c = 0; c < col_w; c++) putchar('=');
                printf("%s", RESET_COLOR);
            }
            else
            {
                /* Which ring (if any) is at this height? */
                /* height 1 = bottom slot, height rows-1 = top slot */
                int height = row;          /* 1..rows-1 */
                int ring_here = 0;
                if (height <= top[p])
                    ring_here = pegs[p][height - 1];

                if (ring_here > 0)
                {
                    int rw  = ring_width(ring_here);
                    int pad = (col_w - rw) / 2;

                    /* Left padding */
                    for (int c = 0; c < pad; c++) putchar(' ');

                    /* Ring body – coloured block with ring number */
                    printf("%s", RING_COLORS[(ring_here - 1) % MAX_RINGS]);
                    char label[32];
                    /* Build the ring face: "[===N===]" style */
                    int inner = rw - 2;           /* space inside brackets */
                    putchar('[');
                    int left_eq  = (inner - 1) / 2;
                    int right_eq = inner - 1 - left_eq;
                    for (int c = 0; c < left_eq;  c++) putchar('=');
                    snprintf(label, sizeof(label), "%d", ring_here);
                    printf("%s", label);
                    for (int c = 0; c < right_eq; c++) putchar('=');
                    putchar(']');
                    printf("%s", RESET_COLOR);

                    /* Right padding */
                    int used = pad + rw;
                    for (int c = used; c < col_w; c++) putchar(' ');
                }
                else
                {
                    /* Empty row – show just the pole */
                    int pole_pos = cx;
                    for (int c = 0; c < col_w; c++)
                    {
                        if (c == pole_pos || c == pole_pos + 1)
                        {
                            printf("%s|%s", COLOR_POLE, RESET_COLOR);
                        }
                        else
                        {
                            putchar(' ');
                        }
                    }
                }
            }
        }
        putchar('\n');
    }

    /* Peg labels */
    printf("%s", COLOR_LABEL);
    for (int p = 0; p < 3; p++)
    {
        char lbl[16];
        snprintf(lbl, sizeof(lbl), "Peg %d", p + 1);
        print_centered(lbl, (int)strlen(lbl), col_w);
    }
    printf("%s\n", RESET_COLOR);
}

/*
 * print_status() – show the current step description and controls.
 */
static void print_status(void)
{
    printf("\n%s", COLOR_INFO);

    if (current_step == 0)
    {
        printf("  Ready. %d ring%s need%s %d move%s. Press [n] to start.",
               num_rings,
               num_rings > 1 ? "s" : "",
               num_rings > 1 ? ""  : "s",
               total_moves,
               total_moves > 1 ? "s" : "");
    }
    else if (current_step == total_moves)
    {
        printf("  Solved! All %d rings are now on Peg 3.", num_rings);
    }
    else
    {
        int from = moves[current_step - 1].from + 1;
        int to   = moves[current_step - 1].to   + 1;
        printf("  Step %d of %d  --  moved a ring from Peg %d to Peg %d.",
               current_step, total_moves, from, to);
    }

    printf("%s\n\n", RESET_COLOR);

    /* Controls row */
    printf("  ");

    /* [p] Prev – dimmed if at the start */
    if (current_step > 0)
        printf("\033[1;97m[ p ] Prev\033[0m");
    else
        printf("%s[ p ] Prev%s", COLOR_DIM, RESET_COLOR);

    printf("   ");

    /* [n] Next – dimmed if at the end */
    if (current_step < total_moves)
        printf("\033[1;97m[ n ] Next\033[0m");
    else
        printf("%s[ n ] Next%s", COLOR_DIM, RESET_COLOR);

    printf("   \033[1;97m[ s ] New solve   [ q ] Quit\033[0m\n\n  > ");
    fflush(stdout);
}

/* -----------------------------------------------------------------
   SECTION 6 – INPUT HANDLING (no terminal raw mode needed)
   ----------------------------------------------------------------- */

/*
 * get_char() – read one character from stdin, skipping newlines.
 */
static int get_char(void)
{
    int c;
    while ((c = getchar()) == '\n' || c == '\r');
    return c;
}

/*
 * ask_rings() – prompt the user for a new ring count and rebuild state.
 */
static void ask_rings(void)
{
    int n = 0;
    while (n < 1 || n > MAX_RINGS)
    {
        printf("\n  Enter number of rings (1-%d): ", MAX_RINGS);
        fflush(stdout);
        if (scanf("%d", &n) != 1) n = 0;
        /* flush rest of line */
        int c; while ((c = getchar()) != '\n' && c != EOF);
        if (n < 1 || n > MAX_RINGS)
            printf("  Please enter a number between 1 and %d.\n", MAX_RINGS);
    }
    num_rings    = n;
    total_moves  = 0;
    current_step = 0;
    gen_moves(num_rings, 0, 2, 1);
    reset_pegs();
}

/* -----------------------------------------------------------------
   SECTION 7 – MAIN LOOP
   ----------------------------------------------------------------- */

int main(void)
{
    /* Initial setup */
    gen_moves(num_rings, 0, 2, 1);
    reset_pegs();

    for (;;)
    {
        clear_screen();
        draw_board();
        print_status();

        int ch = get_char();

        switch (ch)
        {
            case 'n': case 'N':
                if (current_step < total_moves)
                {
                    do_move(current_step);
                    current_step++;
                }
                break;

            case 'p': case 'P':
                if (current_step > 0)
                {
                    current_step--;
                    undo_move(current_step);
                }
                break;

            case 's': case 'S':
                ask_rings();
                break;

            case 'q': case 'Q':
                clear_screen();
                printf("  Goodbye!\n\n");
                return 0;

            default:
                break;   /* ignore unknown keys */
        }
    }
}
