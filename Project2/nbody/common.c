#include "common.h"


/*@q
 * ===================================================================
 */

/*@T
 * \section{Lennard-Jones}
 * 
 * The Lennard-Jones potential has the form
 * \[
 *   V(r) = 
 *   4 \epsilon 
 *     \left[ \left( \frac{\sigma}{r} \right)^6 -
 *            \left( \frac{\sigma}{r} \right)^{12} \right] =
 *   4 \epsilon \left( \frac{\sigma}{r} \right)^6  
 *     \left[ 1 - \left( \frac{\sigma}{r} \right)^{6} \right].
 * \]
 * We will also truncate the potential at $r_c = 2.5 \sigma$.
 * For our simulation, we use the following parameters for the potential:
 *@c*/
#define EPS   1
#define SIG   1e-2
#define RCUT  (2.5*SIG)

/*@T
 * 
 * The Lennard-Jones force on a particle from interaction
 * with another particle at distance
 * $\Delta \bfx = (\Delta x, \Delta y)$ 
 * is given by the negative gradient of the potential, i.e.
 * \[
 *   \bfF = -\nabla V_{LJ}(\|\bfx\|)
 *        = -\frac{dV_{LJ}}{dr} \nabla r
 *        = -\frac{dV_{LJ}}{dr} \frac{\bfx}{r}
 *        = C_{LJ}(r) \bfx
 * \]
 * The scalar expression
 * \[
 *   C_{LJ}(r) 
 *   = \frac{1}{r} \frac{dV_{LJ}}{dr} 
 *   = \frac{24 \epsilon}{r^2}
 *         \left( \frac{\sigma}{r} \right)^6
 *         \left[ 1 - 2\left( \frac{\sigma}{r} \right)^6 \right]
 * \]
 * is particularly convenient because, like the Lennard-Jones
 * potential itself, it will only depend on even powers of $r$.  Thus,
 * there is no need to compute a square root during the computation
 * of $C_{LJ}(r)$:
 *@c*/
float compute_LJ_scalar(float r2, float eps, float sig2)
{
    if (r2 < 6.25*sig2) { /* r_cutoff = 2.5 sigma */
        float z = sig2/r2;
        float u = z*z*z;
        return 24*eps/r2 * u*(1-2*u);
    }
    return 0;
}

/*@T
 * In order to compute the total energy for monitoring purposes,
 * we also want the potential itself:
 *@c*/
float potential_LJ(float r2, float eps, float sig2)
{
    float z = sig2/r2;
    float u = z*z*z;
    return 4*eps*u*(1-u);
}

/*@q
 * ===================================================================
 *@T
 * \section{Leapfrog integration}
 *
 * The leapfrog (or velocity Verlet) method takes two steps
 * to get from $t$ to $t + \Delta t$.  In the first step,
 * we compute estimates of $x(t+\Delta t)$ and $v(t+\Delta t/2)$
 * based on Taylor expansions using $x(t)$, $v(t)$, and $a(t)$:
 * \begin{align*}
 *   v(t+\Delta t/2) &= v(t) + \frac{1}{2} a(t) \Delta t \\
 *   x(t+\Delta t) &= x(t) + v(t + \Delta t/2) \Delta t.
 * \end{align*}
 *@c*/
void leapfrog1(int n, float dt, float* restrict x, 
               float* restrict v, float* restrict a)
{
    for (int i = 0; i < n; ++i, x += 2, v += 2, a += 2) {
        v[0] += a[0]*dt/2;
        v[1] += a[1]*dt/2;
        x[0] += v[0]*dt;
        x[1] += v[1]*dt;
    }
}

/*@T
 * In the second step, we compute $a(t+\Delta t)$ based on
 * the computed $x(t+\Delta t)$, and use the new acceleration
 * to update the velocity from $t+\Delta t/2$ to $t + \Delta t$:
 * \[
 *   v(t + \Delta t) = v(t + \Delta t/2) + \frac{1}{2} a(t+\Delta t) \Delta t.
 * \]
 *@c*/
void leapfrog2(int n, float dt, float* restrict v, float* restrict a)
{
    for (int i = 0; i < n; ++i, v += 2, a += 2) {
        v[0] += a[0]*dt/2;
        v[1] += a[1]*dt/2;
    }
}

/*@T
 * Of course, the interesting part of the work (the part that isn't
 * embarrassingly parallel) occurs in the acceleration computation
 * between the two phases.
 *
 * The leapfrog integration scheme is attractive and widely used because
 * it is {\em symplectic} --- that is, it preserves something about the
 * Hamiltonian structure of the original problem.  In particular, leapfrog
 * integration tends to do a pretty good job at neither gaining nor losing
 * energy over time.  The catch is that these nice properties disappear if
 * one varies the time step, or uses different time steps for different
 * particles.  See recent work by Farr and Bertschinger (2007) for an example
 * of a (more complicated) higher-order symplectic integrator that {\em does}
 * allow variations in the time steps.
 *@q*/

/* ===================================================================
 *@T
 * \section{Reflection boundaries}
 *
 * In order to prevent our particles from flying off to infinity,
 * we impose reflection boundary conditions (at the end of a step).
 * These conditions should be imposed after the first half-step of
 * Verlet.  Currently we don't check if for multiple reflections ---
 * if they occur, something is amiss!
 *@c*/
static void reflect(float wall, float* restrict x, 
                    float* restrict v, float* restrict a)
{
    *x = (2*wall-(*x));
    *v = -(*v);
    *a = -(*a);
}

void apply_reflect(int n, float* restrict x, 
                   float* restrict v, float* restrict a)
{
    for (int i = 0; i < n; ++i, x += 2, v += 2, a += 2) {
        if (x[0] < XMIN) reflect(XMIN, x+0, v+0, a+0);
        if (x[0] > XMAX) reflect(XMAX, x+0, v+0, a+0);
        if (x[1] < YMIN) reflect(YMIN, x+1, v+1, a+1);
        if (x[1] > YMAX) reflect(YMAX, x+1, v+1, a+1);
    }
}

