vec2 R; // see more about aniso x16 here: https://www.shadertoy.com/view/ltXfRr

vec4 textureAniso(sampler2D T, vec2 p) {
    mat2 J = inverse(mat2(dFdx(p),dFdy(p)));       // dFdxy: pixel footprint in texture space
    J = transpose(J)*J;                            // quadratic form
    float d = determinant(J), t = J[0][0]+J[1][1], // find ellipse: eigenvalues, max eigenvector
          D = sqrt(abs(t*t-4.*d)),                 // abs() fix a bug: in weird view angles 0 can be slightly negative
          V = (t-D)/2., v = (t+D)/2.,                     // eigenvalues. ( ATTENTION: not sorted )
          M = 1./sqrt(V), m = 1./sqrt(v), l =log2(m*R.y); // = 1./radii^2
  //if (M/m>16.) l = log2(M/16.*R.y);                     // optional
    vec2 A = M * normalize(vec2( -J[0][1] , J[0][0]-V )); // max eigenvector = main axis
    vec4 O = vec4(0);
    for (float i = -7.5; i<8.; i++)                       // sample x16 along main axis at LOD min-radius
        O += textureLod(iChannel0, p+(i/16.)*A, l);
    return O/16.;
}



void mainImage( out vec4 O, vec2 U )
{
    R = iResolution.xy;
    vec3 r = vec3( 2.* U / R - 1. , 1 );                         // ray
    vec2 p = - r.xz / r.y + .1 * iTime;                          // intersection on floor

    p /= 4.;  // because texture contains big tiles
    O =   r.y > 0.  ? vec4(.5, .7, .9, 1)                        // background
        : r.x < -.4 ? textureLod(iChannel0, p, 0.)               // bilinear
        : r.x <  .25 ? texture(iChannel0, p)                     // MIPmap
                 // : textureGrad(iChannel0, p,dFdx(p),dFdy(p)); // (equivalent)
                    : textureAniso(iChannel0, p);                // anisotropic
    O = pow(O,vec4(1./2.2));
}
