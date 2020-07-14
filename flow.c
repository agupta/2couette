#include "navier-stokes/centered.h"
#include "two-phase.h"
#include "tension.h"
#include "view.h"

#define MAX_LEVEL 7
#define TIME_STEP 1./25 // Timestep for movie and log file.
#define MAX_TIME 3.0 // Time to stop at.
#define beta 0.93 // Location of the fluid interface

// Fluid 1 is water at 25°C
#define rhoWater 0.9970
#define muWater 0.8937
// Source: retrieved 7/9/20 from
// https://en.wikipedia.org/wiki/Water_(data_page)#Liquid_physical_properties

// Fluid 2 is sunflower oil at 25°C
#define rhoOil 0.9188
#define muOil 49.14
// Source: retrieved 7/9/20 from
// https://en.wikipedia.org/wiki/Sunflower_oil#Physical_properties

#define Fr 1./sqrt(9.81)
#define We 1.*1.*1./0.037

int main () {
  init_grid(1 << MAX_LEVEL);

  // Set up fluids, doing this in the init event doesn't work.

  // Fluid 1 is water 
  rho1 = rhoWater;
  mu1 = muWater;

  // Fluid 2 is sunflower oil
  rho2 = rhoOil;
  mu2 = muOil;

  // Surface tension coefficient
  f.sigma = 1./We;

  run();
}

event acceleration (i++)
{
  face vector av = a;
  foreach_face(x)
    av.x[] -= 1./sq(Fr);
}

event init(t = 0) {

  // Flow out of the right -> into the left.
  periodic(right);

  // positive -> fluid 1
  // negative -> fluid 2 
  // bottom beta% is oil.
  fraction (f, beta-y + 0.04*sin(4*pi*x));

  // Initially velocity is 0 everywhere.
  foreach () {
    u.x[] = 0.;
    u.y[] = 0.;
  }
  
  // No-slip boundary conditions.
  u.t[top] = dirichlet(0.);
  u.t[bottom] = dirichlet(0.);

}

event adapt (i++) {
  // error thresholds for            f     u.x   u.y
  adapt_wavelet ({f, u}, (double []){1e-2, 1e-2, 1e-2}, MAX_LEVEL);
}

event animationSpeed (t += TIME_STEP; t <= MAX_TIME) {
  // Compute the speed
  scalar speed[]; 
  foreach ()
    speed[] = norm(u); // l2
  view(tx=-0.5, ty=-0.5);
  clear();
  squares("speed", spread = -1, linear = true, map=cool_warm );
  draw_vof("f", lc = {0.0,0.0,0.0}, lw=1);
  // box();
  save ("Speed.mp4");
}

event animationUx (t += TIME_STEP; t <= MAX_TIME) {
  view(tx=-0.5, ty=-0.5);
  clear();
  squares("u.x", spread = -1, linear = true, map=cool_warm );
  draw_vof("f", lc = {0.0,0.0,0.0}, lw=1);
  // box();
  save ("HorizontalVelocity.mp4");
}

event animationGridLevels (t += TIME_STEP; t <= MAX_TIME) {
  scalar l[];
  foreach()
    l[] = level;
  view(tx=-0.5, ty=-0.5);
  clear();
  squares("l", map=cool_warm, linear = true, min=0, max=MAX_LEVEL);
  draw_vof("f", lc = {0.0,0.0,0.0}, lw=1);
  // box();
  save ("GridLevels.mp4");
}

event animationFluids(t += TIME_STEP; t <= MAX_TIME) {
  view(tx=-0.5, ty=-0.5);
  clear();
  squares("f", spread = -1, map=cool_warm, linear = true, min=0, max=MAX_LEVEL);
  draw_vof("f", lc = {0.0,0.0,0.0}, lw=1);
  // box();
  save ("Fluids.mp4");
}

event logfile (t += TIME_STEP; t <= MAX_TIME) {
  // Compute the speed
  scalar speed[]; 
  foreach ()
    speed[] = norm(u); // l2
  // generate statistics
  stats s = statsf(speed);

  // log to file
  FILE * fp = fopen("flow.log", "a");
  fprintf (fp, "%g %d %g %g %g %g\n", t, i, dt, s.sum, s.max, s.min);
  fclose(fp);
}