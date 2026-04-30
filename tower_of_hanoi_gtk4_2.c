/*
 * Tower of Hanoi – GTK4 visualizer
 * ===================================
 * A beginner-friendly GTK4 program that draws the Tower of Hanoi
 * and lets you step through the solution one move at a time.
 *
 * HOW TO COMPILE:
 *   gcc tower_of_hanoi_gtk4.c -o hanoi $(pkg-config --cflags --libs gtk4) -lm
 *
 * HOW TO RUN:
 *   ./hanoi
 */

#include <gtk/gtk.h>   /* GTK4 – for the window, buttons, entry box */
#include <stdlib.h>    /* atoi() – converts text to integer          */
#include <string.h>    /* memset() – clears arrays to zero           */
#include <stdio.h>     /* snprintf() – formats text into a buffer    */

/* -----------------------------------------------------------------
   SECTION 1 – GLOBAL SETTINGS (easy to tweak)
   ----------------------------------------------------------------- */

#define MAX_RINGS  7      /* maximum number of rings allowed   */
#define MAX_MOVES  128    /* 2^7 - 1 = 127 moves at most       */

/* Canvas size in pixels */
#define CANVAS_W   680
#define CANVAS_H   300

/* Where the base platform sits on the canvas */
#define BASE_Y     260

/* How tall the poles are and how thick */
#define POLE_HEIGHT 180
#define POLE_WIDTH    8

/* Each ring is this tall (in pixels) */
#define RING_HEIGHT  20

/* Narrowest and widest a ring can be */
#define MIN_RING_W   28
#define MAX_RING_W  150

/* One colour (R, G, B) for each possible ring.
   Values are between 0.0 (dark) and 1.0 (bright). */
static const double COLORS[MAX_RINGS][3] = {
    {0.89, 0.29, 0.29},   /* ring 1 - red    */
    {0.73, 0.46, 0.09},   /* ring 2 - amber  */
    {0.39, 0.60, 0.13},   /* ring 3 - green  */
    {0.11, 0.62, 0.46},   /* ring 4 - teal   */
    {0.22, 0.54, 0.87},   /* ring 5 - blue   */
    {0.50, 0.47, 0.87},   /* ring 6 - purple */
    {0.83, 0.33, 0.49},   /* ring 7 - pink   */
};

/* -----------------------------------------------------------------
   SECTION 2 – PROGRAM STATE
   (variables that store what is happening)
   ----------------------------------------------------------------- */

/* A "move" is just: take the top ring from peg 'from' and put it on peg 'to' */
typedef struct {
    int from;   /* source peg      (0, 1, or 2) */
    int to;     /* destination peg (0, 1, or 2) */
} Move;

static int  num_rings    = 3;        /* how many rings the user chose    */
static Move moves[MAX_MOVES];        /* the full list of moves to solve  */
static int  total_moves  = 0;        /* how many moves are in that list  */
static int  current_step = 0;        /* which move we are currently on   */

/* Each peg is a stack of rings.
   pegs[p][0] is the bottom ring, pegs[p][top[p]-1] is the top ring. */
static int pegs[3][MAX_RINGS];       /* rings sitting on each peg        */
static int top[3];                   /* how many rings on each peg       */

/* GTK widgets we need to access from multiple functions */
static GtkWidget *canvas;            /* the drawing area                 */
static GtkWidget *entry;             /* the text box where user types    */
static GtkWidget *label_status;      /* shows current step description   */
static GtkWidget *btn_prev;          /* "<-- Prev" button                */
static GtkWidget *btn_next;          /* "Next -->" button                */

/* -----------------------------------------------------------------
   SECTION 3 – SOLVING THE PUZZLE (recursion)
   ----------------------------------------------------------------- */

/*
 * gen_moves - recursive function that generates all moves needed.
 *
 *   n    = number of rings to move
 *   from = peg we are moving rings OFF
 *   to   = peg we are moving rings ONTO
 *   aux  = the spare (helper) peg
 *
 * The classic algorithm:
 *   1. Move (n-1) rings from 'from' to 'aux' (using 'to' as spare)
 *   2. Move the biggest ring directly from 'from' to 'to'
 *   3. Move (n-1) rings from 'aux' to 'to' (using 'from' as spare)
 */
static void gen_moves(int n, int from, int to, int aux)
{
    if (n == 0) return;                  /* nothing to move - stop recursing */

    gen_moves(n - 1, from, aux, to);    /* step 1 */

    /* record this move: move the nth (biggest remaining) ring */
    moves[total_moves].from = from;
    moves[total_moves].to   = to;
    total_moves++;

    gen_moves(n - 1, aux, to, from);   /* step 3 */
}

/* -----------------------------------------------------------------
   SECTION 4 – PEG / STACK HELPERS
   ----------------------------------------------------------------- */

/* Place all rings on peg 0 (the starting position) */
static void reset_pegs(void)
{
    /* Clear every peg */
    memset(pegs, 0, sizeof(pegs));
    memset(top,  0, sizeof(top));

    /* Stack rings largest-first (largest = num_rings, smallest = 1) */
    for (int ring = num_rings; ring >= 1; ring--)
    {
        pegs[0][ top[0] ] = ring;   /* put ring on peg 0 */
        top[0]++;                    /* peg 0 now has one more ring */
    }
}

/* Execute move number 'idx': take the top ring from source and put it on dest */
static void do_move(int idx)
{
    int src  = moves[idx].from;
    int dest = moves[idx].to;

    top[src]--;                           /* remove from source        */
    int ring = pegs[src][ top[src] ];     /* remember which ring it is */

    pegs[dest][ top[dest] ] = ring;       /* place on destination      */
    top[dest]++;                          /* destination has one more   */
}

/* Undo move number 'idx': reverse the above */
static void undo_move(int idx)
{
    int src  = moves[idx].from;
    int dest = moves[idx].to;

    top[dest]--;                          /* remove from destination   */
    int ring = pegs[dest][ top[dest] ];   /* which ring was it?        */

    pegs[src][ top[src] ] = ring;         /* put it back on source     */
    top[src]++;
}

/* -----------------------------------------------------------------
   SECTION 5 – UPDATING THE STATUS LABEL
   ----------------------------------------------------------------- */

static void update_status(void)
{
    char text[128];

    if (current_step == 0)
    {
        /* Haven't started yet */
        snprintf(text, sizeof(text),
                 "Ready. %d rings need %d moves. Press Next to start.",
                 num_rings, total_moves);
    }
    else if (current_step == total_moves)
    {
        /* Finished */
        snprintf(text, sizeof(text),
                 "Solved! All %d rings are now on peg 3.", num_rings);
    }
    else
    {
        /* In the middle - describe the last move we did */
        int from = moves[current_step - 1].from + 1;  /* +1 so it shows 1,2,3 */
        int to   = moves[current_step - 1].to   + 1;
        snprintf(text, sizeof(text),
                 "Step %d of %d  --  moved a ring from peg %d to peg %d.",
                 current_step, total_moves, from, to);
    }

    gtk_label_set_text(GTK_LABEL(label_status), text);

    /* Gray out the buttons when there is nothing to do */
    gtk_widget_set_sensitive(btn_prev, current_step > 0);
    gtk_widget_set_sensitive(btn_next, current_step < total_moves);
}

/* -----------------------------------------------------------------
   SECTION 6 – DRAWING THE CANVAS WITH CAIRO
   ----------------------------------------------------------------- */

/*
 * This function is called automatically by GTK every time the
 * canvas needs to be redrawn (when the window opens, is resized,
 * or when we call gtk_widget_queue_draw).
 *
 * cr  = the Cairo drawing context (like a "pen" we draw with)
 * w,h = current pixel size of the canvas widget
 */
static void draw_canvas(GtkDrawingArea *area, cairo_t *cr,
                         int w, int h, gpointer unused)
{
    (void)area; (void)unused;   /* we don't use these parameters */

    /* Scale everything so the drawing always fills the window */
    double sx = (double)w / CANVAS_W;
    double sy = (double)h / CANVAS_H;
    cairo_scale(cr, sx, sy);

    /* -- Background (light cream colour) -- */
    cairo_set_source_rgb(cr, 0.97, 0.96, 0.94);
    cairo_rectangle(cr, 0, 0, CANVAS_W, CANVAS_H);
    cairo_fill(cr);

    /* -- Base platform -- */
    cairo_set_source_rgb(cr, 0.68, 0.67, 0.65);
    cairo_rectangle(cr, 40, BASE_Y, CANVAS_W - 80, 12);
    cairo_fill(cr);

    /* The three pegs are evenly spaced across the canvas */
    double peg_x[3];
    peg_x[0] = CANVAS_W / 4.0;         /* left peg   */
    peg_x[1] = CANVAS_W / 2.0;         /* middle peg */
    peg_x[2] = CANVAS_W * 3.0 / 4.0;  /* right peg  */

    /* Draw each peg (pole + rings on it + label below) */
    for (int p = 0; p < 3; p++)
    {
        /* -- Pole -- */
        cairo_set_source_rgb(cr, 0.60, 0.59, 0.57);
        cairo_rectangle(cr,
                        peg_x[p] - POLE_WIDTH / 2.0,   /* x (centred) */
                        BASE_Y - POLE_HEIGHT,            /* y (top)     */
                        POLE_WIDTH,                      /* width       */
                        POLE_HEIGHT);                    /* height      */
        cairo_fill(cr);

        /* -- Rings sitting on this peg -- */
        for (int i = 0; i < top[p]; i++)
        {
            int ring = pegs[p][i];   /* ring number (1=smallest, num_rings=biggest) */

            /* Width grows linearly with ring number */
            double t  = (num_rings > 1)
                        ? (double)(ring - 1) / (num_rings - 1)
                        : 0.5;
            double rw = MIN_RING_W + t * (MAX_RING_W - MIN_RING_W);
            double rx = peg_x[p] - rw / 2.0;                   /* left edge   */
            double ry = BASE_Y - (i + 1) * (RING_HEIGHT + 2);  /* bottom-up   */

            /* Pick this ring's colour */
            int c = (ring - 1) % MAX_RINGS;
            cairo_set_source_rgb(cr, COLORS[c][0], COLORS[c][1], COLORS[c][2]);
            cairo_rectangle(cr, rx, ry, rw, RING_HEIGHT);
            cairo_fill(cr);

            /* Write the ring number in white on top of the ring */
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_set_font_size(cr, 11);
            char num[4];
            snprintf(num, sizeof(num), "%d", ring);
            cairo_text_extents_t ext;
            cairo_text_extents(cr, num, &ext);
            /* Centre the number inside the ring rectangle */
            cairo_move_to(cr,
                peg_x[p] - ext.width / 2.0,
                ry + RING_HEIGHT / 2.0 + ext.height / 2.0);
            cairo_show_text(cr, num);
        }

        /* -- Peg label below the base -- */
        cairo_set_source_rgb(cr, 0.40, 0.39, 0.37);
        cairo_set_font_size(cr, 13);
        char lbl[8];
        snprintf(lbl, sizeof(lbl), "Peg %d", p + 1);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, lbl, &ext);
        cairo_move_to(cr, peg_x[p] - ext.width / 2.0, BASE_Y + 28);
        cairo_show_text(cr, lbl);
    }
}

/* -----------------------------------------------------------------
   SECTION 7 – BUTTON CALLBACKS
   (functions called when the user clicks)
   ----------------------------------------------------------------- */

/* Called when the user clicks "Next -->" */
static void on_next(GtkButton *b, gpointer d)
{
    (void)b; (void)d;
    if (current_step >= total_moves) return;   /* already at the end */

    do_move(current_step);   /* move a ring on the pegs */
    current_step++;

    update_status();
    gtk_widget_queue_draw(canvas);   /* ask GTK to repaint the canvas */
}

/* Called when the user clicks "<-- Prev" */
static void on_prev(GtkButton *b, gpointer d)
{
    (void)b; (void)d;
    if (current_step <= 0) return;   /* already at the beginning */

    current_step--;
    undo_move(current_step);   /* put the ring back */

    update_status();
    gtk_widget_queue_draw(canvas);
}

/* Called when user types a number and presses Enter (or clicks Solve) */
static void on_solve(GtkWidget *w, gpointer d)
{
    (void)w; (void)d;

    /* Read the number from the text box */
    const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
    int n = atoi(text);           /* convert text "3" to integer 3 */

    /* Clamp to a safe range */
    if (n < 1) n = 1;
    if (n > MAX_RINGS) n = MAX_RINGS;

    /* Update the entry box in case we clamped the value */
    char fixed[4];
    snprintf(fixed, sizeof(fixed), "%d", n);
    gtk_editable_set_text(GTK_EDITABLE(entry), fixed);

    /* Rebuild the solution for this number of rings */
    num_rings    = n;
    total_moves  = 0;
    current_step = 0;
    gen_moves(num_rings, 0, 2, 1);   /* from peg 0, to peg 2, spare peg 1 */
    reset_pegs();

    update_status();
    gtk_widget_queue_draw(canvas);
}

/* -----------------------------------------------------------------
   SECTION 8 – BUILDING THE WINDOW (UI layout)
   ----------------------------------------------------------------- */

/*
 * activate - GTK calls this once when the app starts.
 * We create the window and all widgets here.
 */
static void activate(GtkApplication *app, gpointer data)
{
    (void)data;

    /* Create the main window */
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Tower of Hanoi");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 460);

    /*
     * Layout: a vertical box (vbox) stacks children top-to-bottom.
     * We will add: title -> input row -> canvas -> status label -> buttons
     */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top   (vbox, 14);
    gtk_widget_set_margin_bottom(vbox, 14);
    gtk_widget_set_margin_start (vbox, 18);
    gtk_widget_set_margin_end   (vbox, 18);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    /* -- Title label -- */
    GtkWidget *title = gtk_label_new("Tower of Hanoi");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), title);

    /* -- Input row: label + text entry + Solve button -- */
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(row, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), row);

    GtkWidget *lbl = gtk_label_new("Number of rings (1 to 7):");
    gtk_box_append(GTK_BOX(row), lbl);

    entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(entry), "3"); /* default value – GTK4 uses gtk_editable_set_text, not gtk_entry_set_text */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);   /* only 1 digit  */
    gtk_widget_set_size_request(entry, 55, -1);
    /* When the user presses Enter inside the entry, call on_solve */
    g_signal_connect(entry, "activate", G_CALLBACK(on_solve), NULL);
    gtk_box_append(GTK_BOX(row), entry);

    GtkWidget *btn_solve = gtk_button_new_with_label("Solve");
    g_signal_connect(btn_solve, "clicked", G_CALLBACK(on_solve), NULL);
    gtk_box_append(GTK_BOX(row), btn_solve);

    /* -- Drawing canvas -- */
    canvas = gtk_drawing_area_new();
    gtk_widget_set_vexpand(canvas, TRUE);    /* stretch to fill extra space */
    gtk_widget_set_hexpand(canvas, TRUE);
    /* Tell GTK which function to call whenever the canvas must be drawn */
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(canvas),
                                   draw_canvas, NULL, NULL);
    gtk_box_append(GTK_BOX(vbox), canvas);

    /* -- Status label (shows current step description) -- */
    label_status = gtk_label_new("Enter a number and press Solve.");
    gtk_widget_set_halign(label_status, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), label_status);

    /* -- Navigation buttons: Prev and Next -- */
    GtkWidget *btn_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_row, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), btn_row);

    btn_prev = gtk_button_new_with_label("<-- Prev");
    btn_next = gtk_button_new_with_label("Next -->");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_prev), NULL);
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_next), NULL);
    gtk_box_append(GTK_BOX(btn_row), btn_prev);
    gtk_box_append(GTK_BOX(btn_row), btn_next);

    /* -- First draw: set up pegs and compute initial solution -- */
    gen_moves(num_rings, 0, 2, 1);
    reset_pegs();
    update_status();

    gtk_window_present(GTK_WINDOW(window));
}

/* -----------------------------------------------------------------
   SECTION 9 – PROGRAM ENTRY POINT
   ----------------------------------------------------------------- */

int main(int argc, char **argv)
{
    /* Create a GTK application with a unique ID */
    GtkApplication *app = gtk_application_new("org.example.hanoi",
                                              G_APPLICATION_DEFAULT_FLAGS);

    /* Connect the "activate" signal to our activate() function */
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    /* Run the app (this blocks until the window is closed) */
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);   /* free memory */
    return status;
}
