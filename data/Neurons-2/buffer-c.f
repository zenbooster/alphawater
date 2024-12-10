#define radius 2.0
void mainImage( out vec4 fragColor, in vec2 pos )
{
    float rho = 0.001;
    vec2 vel = vec2(0., 0.);

    //compute the smoothed density and velocity
    range(i, -2, 2) range(j, -2, 2)
    {
        vec2 tpos = pos + vec2(i,j);
        vec4 data = T(tpos);

        vec2 X0 = DECODE(data.x) + tpos;
        vec2 V0 = DECODE(data.y);
        float M0 = data.z;
        vec2 dx = X0 - pos;

        float K = GS(dx/radius)/(radius);
        rho += M0*K;
        vel += M0*K*V0;
    }

    vel /= rho;

    fragColor = vec4(vel, rho, 1.0);
}