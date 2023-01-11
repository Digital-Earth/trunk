
// Data from http://local.wasp.uwa.edu.au/~pbourke/modelling_rendering/starfield/
// Collated from Bright Star Catalogue, 5th Revised Ed. by Paul Bourke
// Further processed by mlepage

typedef struct {
	unsigned char r,g,b,a;
	float x,y,z;
} C4UB_V3F;

extern C4UB_V3F stardata[];
extern int starcount;
