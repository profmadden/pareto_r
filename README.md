# pareto_r
Bentley's divide-and-conquer ECDF/Maxima Pareto set finding code. 

This is a Rust wrapper around my implementation of Bentley's algorithm.  It supports a maximum of five dimensions (this is in pareto.h), and computes the dominance of points efficiently.  There are
two simple examples in the src/bin directory.

To use the library, create a ParetoProblem object, specifying the dimensions of the points.
Then, add a point at a time (you can add a tag to the points if you desire).  Call the solve
method, and the resulting points will have their rank computed and updated.

This library uses my pstools routines to generate PostScript output (just showing 2D).  The
output PostScript can be converted to PDF using GhostScript (and the command line to run
GhostScript is in the header of the PS file; I can never remember how to do it, so I have
the generator embed the command line stuff.).

With Bentley's code, I didn't see a clean way to handle points with identical positions
on one or more axis; I use the input order for tie breaking, but that might wind up not
detecting dominance in cases where it reasonably should be detected.  I'll try to write
up a more detailed explanation of what can happen (and maybe someone can clue me in to
a fix for this).

The points with zero rank are not dominated; that's the Pareto Front, or Pareto Optimal
Set, depending on the terminology you prefer.  There may be some co-located points with
zero rank, that could be eliminated (based on how tie breaking happens).
