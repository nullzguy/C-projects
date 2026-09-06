/*
terminal 3d projection mini project
gcc -O3 -o 3dProj ./3dProjection.c -lm && ./3dProj
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define PI 3.141592653589793f
#define DEG_TO_RAD(x) ((x) * 0.01745329251994329577f)
#define RAND0_1 (float)(rand() % 10000000) / 10000000.0f

#define DIFFUSE 1
#define LIGHT 2

// Vectors
struct fvec2 { float x, y; };
struct fvec3 { float x, y, z; };
struct fvec4 { float x, y, z, w; };
struct ivec2 { int x, y; };
struct ivec3 { int x, y, z; };
struct ivec4 { int x, y, z, w; };
static inline struct fvec3 fvec3_add(struct fvec3 a, struct fvec3 b) { return (struct fvec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline struct fvec3 fvec3_sub(struct fvec3 a, struct fvec3 b) { return (struct fvec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline struct fvec3 fvec3_scl(struct fvec3 v, float s) { return (struct fvec3){v.x * s, v.y * s, v.z * s}; }
static inline struct fvec3 fvec3_mul(struct fvec3 a, struct fvec3 b) { return (struct fvec3){a.x * b.x, a.y * b.y, a.z * b.z}; }
static inline float fvec3_dot(struct fvec3 a, struct fvec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline struct fvec3 fvec3_cross(struct fvec3 a, struct fvec3 b) {
	return (struct fvec3){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}
static inline struct fvec3 fvec3_normalize(struct fvec3 v) {
	float len = sqrtf(fvec3_dot(v, v));
	if (len == 0.0f) return (struct fvec3){0.0f, 0.0f, 0.0f};
	return fvec3_scl(v, 1.0f/len);
}

struct fvec3 forward = {1,0,0}, up = {0,1,0}, right = {0,0,1};
// -------

static inline float edge( struct fvec2 a, struct fvec2 b, float x, float y ) {
    return (x - a.x) * (b.y - a.y) - (y - a.y) * (b.x - a.x);
}

static inline int clampi(int x, int min, int max) {
    return (x < min) ? min : (x > max) ? max : x;
}

struct Settings {
	int w, h;
	float t;
};

struct Pixel {
	struct fvec3 rgb;
};

struct Camera {
	struct fvec3 pos, vec, vecRight, vecUp;
	float pitch, yaw, fov;
};

struct Triangle {
	struct fvec3 vp1, vp2, vp3;
	struct fvec3 col; int matType;
};

void updateCamVectors(struct Camera *cam) {
	cam->vec = fvec3_normalize((struct fvec3){cosf(cam->pitch) * sinf(cam->yaw), sinf(cam->pitch), cosf(cam->pitch) * cosf(cam->yaw)});
	cam->vecRight = fvec3_normalize(fvec3_cross(up, cam->vec));
	cam->vecUp = fvec3_normalize(fvec3_cross(cam->vec, cam->vecRight));
}

static inline void getBgColor(char **dst, struct Pixel *px) {
	struct ivec3 rgb = { (int)(px->rgb.x * 255.999f), (int)(px->rgb.y * 255.999f), (int)(px->rgb.z * 255.999f) };
	rgb.x = (rgb.x<0) ? 0 : (rgb.x>255) ? 255 : rgb.x; rgb.y = (rgb.y<0) ? 0 : (rgb.y>255) ? 255 : rgb.y; rgb.z = (rgb.z<0) ? 0 : (rgb.z>255) ? 255 : rgb.z;
	*dst += sprintf(*dst, "\033[48;2;%d;%d;%dm", rgb.x, rgb.y, rgb.z);
}

void rasterize(struct Settings *set, struct Pixel *pxs, struct Triangle *tris, int trisCount, float *depthBuffer, struct Camera *cam, struct fvec3 lightDir) {
	float focalY = (set->h * 0.5f) / tanf(cam->fov * 0.5f);
	float focalX = focalY*2.0f;
	for (int i=0; i<set->h*set->w; i++) {
		pxs[i].rgb = (struct fvec3){0, 0, 0};
		depthBuffer[i] = 0;
	}

	for (int i=0;i<trisCount;i++) {
		struct fvec3 rel1 = fvec3_sub(tris[i].vp1, cam->pos), rel2 = fvec3_sub(tris[i].vp2, cam->pos), rel3 = fvec3_sub(tris[i].vp3, cam->pos);
		struct fvec3 camP1 = { fvec3_dot(rel1, cam->vecRight), fvec3_dot(rel1, cam->vecUp), fvec3_dot(rel1, cam->vec) },
			camP2 = { fvec3_dot(rel2, cam->vecRight), fvec3_dot(rel2, cam->vecUp), fvec3_dot(rel2, cam->vec) },
			camP3 = { fvec3_dot(rel3, cam->vecRight), fvec3_dot(rel3, cam->vecUp), fvec3_dot(rel3, cam->vec) };
		if (camP1.z <= 0.0f || camP2.z <= 0.0f || camP3.z <= 0.0f) continue;
		float invZ1 = 1.0f / camP1.z;
		float invZ2 = 1.0f / camP2.z;
		float invZ3 = 1.0f / camP3.z;

		struct fvec2 pv1 = (struct fvec2){ set->w * 0.5f + focalX * camP1.x / camP1.z, set->h * 0.5 - focalY * camP1.y / camP1.z };
		struct fvec2 pv2 = (struct fvec2){ set->w * 0.5f + focalX * camP2.x / camP2.z, set->h * 0.5 - focalY * camP2.y / camP2.z };
		struct fvec2 pv3 = (struct fvec2){ set->w * 0.5f + focalX * camP3.x / camP3.z, set->h * 0.5 - focalY * camP3.y / camP3.z };

		float minX = fminf(pv1.x, fminf(pv2.x, pv3.x));
		float maxX = fmaxf(pv1.x, fmaxf(pv2.x, pv3.x));
		float minY = fminf(pv1.y, fminf(pv2.y, pv3.y));
		float maxY = fmaxf(pv1.y, fmaxf(pv2.y, pv3.y));

		int x0 = clampi((int)floorf(minX), 0, set->w - 1);
		int x1 = clampi((int)ceilf (maxX), 0, set->w - 1);
		int y0 = clampi((int)floorf(minY), 0, set->h - 1);
		int y1 = clampi((int)ceilf (maxY), 0, set->h - 1);

		float area = edge(pv1, pv2, pv3.x, pv3.y);
		if (area == 0.0f) continue;

		for (int y = y0; y <= y1; y++) {
			for (int x = x0; x <= x1; x++) {
				float px = x + 0.5f;
				float py = y + 0.5f;

				float e1 = edge(pv1, pv2, px, py);
				float e2 = edge(pv2, pv3, px, py);
				float e3 = edge(pv3, pv1, px, py);

				int inside = (e1 >= 0.0f && e2 >= 0.0f && e3 >= 0.0f) || (e1 <= 0.0f && e2 <= 0.0f && e3 <= 0.0f);
        			if (inside) { 
					float w1 = e1 / area;
					float w2 = e2 / area;
					float w3 = e3 / area;
					float invDepth = w1 * invZ1 + w2 * invZ2 + w3 * invZ3;
					int idx = y * set->w + x;
					if (invDepth > depthBuffer[idx]) {
						struct fvec3 edge1 = fvec3_sub(tris[i].vp2, tris[i].vp1);
						struct fvec3 edge2 = fvec3_sub(tris[i].vp3, tris[i].vp1);
						struct fvec3 normal = fvec3_normalize(fvec3_cross(edge1, edge2));
						float lightMul = fmax(fvec3_dot(fvec3_normalize(normal), fvec3_scl(fvec3_normalize(lightDir), -1)), 0.0);
						depthBuffer[idx] = invDepth;
						pxs[idx].rgb = fvec3_scl(tris[i].col, lightMul);
					}
				}
			}
		}
	}
}

void spinTris(struct Triangle *in, int trisCount, struct Triangle *out, float angle) {
	float c = cosf(angle), s = sinf(angle);
	for (int i = 0; i < trisCount; i++) {
		out[i] = in[i];
		struct fvec3 *v[3] = { &out[i].vp1, &out[i].vp2, &out[i].vp3 };
		for (int j = 0; j < 3; j++) {
			float x = v[j]->x, z = v[j]->z;
			v[j]->x = x * c + z * s;
			v[j]->z = -x * s + z * c;
		}
	}
}

void render(struct Settings *set, struct Pixel *pxs) {
	int bufSize = set->h*(set->w*21+1);
	char *buf = malloc(bufSize);
	if (!buf) return;
	char *out = buf;
	int idx = 0;

	for (int y=0;y<set->h;y++) {
		for (int x=0;x<set->w;x++) {
			getBgColor(&out, &pxs[idx++]);
			*out++ = ' ';
		}
		*out++ = '\n';
	}

	write(STDOUT_FILENO, "\033[?25l\033[H", 9);
	write(STDOUT_FILENO, buf, out-buf);
	free(buf);
}
	
struct Triangle *loadObj(const char *filename, int *triCount, struct fvec3 col, float noise) {
	FILE *f = fopen(filename, "r");
	if (!f) { perror(filename); return NULL; }
	
	struct fvec3 *verts = NULL;
	int vcap = 0, vcnt = 0;
	struct Triangle *tris = NULL;
	int tcap = 0, tcnt = 0;
	char line[256];
	
	while (fgets(line, sizeof(line), f)) {
		if (strncmp(line, "v ", 2) == 0) {
			struct fvec3 v;
			sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
			if (vcnt >= vcap) {
				vcap = vcap ? vcap * 2 : 64;
				verts = realloc(verts, vcap * sizeof(struct fvec3));
			}
			verts[vcnt++] = v;
		}
		else if (strncmp(line, "f ", 2) == 0) {
			int indices[4] = {0};
			int idxCount = 0;
			char *p = line + 2;
			while (*p && idxCount < 4) {
				while (*p == ' ' || *p == '\t') p++;
				if (*p == '\0' || *p == '\n') break;
				int v = strtol(p, &p, 10);
				indices[idxCount++] = v;
				while (*p && *p != ' ' && *p != '\t' && *p != '\n') p++;
			}
	
			if (idxCount >= 3) {
				struct fvec3 color = fvec3_scl(col, (RAND0_1-0.5f)*noise+1.0f);
				if (tcnt >= tcap) {
					tcap = tcap ? tcap * 2 : 64;
					tris = realloc(tris, tcap * sizeof(struct Triangle));
				}
				tris[tcnt].vp1 = verts[indices[0] - 1];
				tris[tcnt].vp2 = verts[indices[1] - 1];
				tris[tcnt].vp3 = verts[indices[2] - 1];
				tris[tcnt].col = color;
	            		tcnt++;
	
				if (idxCount == 4) {
					if (tcnt >= tcap) {
						tcap = tcap ? tcap * 2 : 64;
						tris = realloc(tris, tcap * sizeof(struct Triangle));
					}
					tris[tcnt].vp1 = verts[indices[0] - 1];
					tris[tcnt].vp2 = verts[indices[2] - 1];
					tris[tcnt].vp3 = verts[indices[3] - 1];
					tris[tcnt].col = color;
					tcnt++;
				}
			}
		}
	}

	fclose(f);
	free(verts);
	*triCount = tcnt;
	return tris;
}

int main() {
	srand(1);
	struct Settings set = {200*2, 100, 0}; // resolution, time=0 (of course)
	struct Pixel pxs[set.h*set.w]; float *depthBuffer = malloc(set.w*set.h*sizeof(float));
	struct Camera cam = {{0,3.5,-6.0}, {0,0,0}, {0,0,0}, {0,0,0}, DEG_TO_RAD(-10), DEG_TO_RAD(0), DEG_TO_RAD(60)};
	int triCount; char *objPath = "/home/nullz/Projects/C-projects-main/Cprojects/Objects/SuperHero.obj";
	struct Triangle *base = loadObj(objPath, &triCount, (struct fvec3){1.0, 1.0, 1.0}, 0.15);
	struct Triangle *tris = malloc(triCount * sizeof(struct Triangle));
	struct fvec3 lightDir = {-0.5, -0.5, 0.5};
	if (!base || triCount == 0) {system("clear"); printf("ERROR! Failed to load .obj"); return 1;};

	for (int i=0;i<set.h*set.w;i++) {
		pxs[i].rgb = (struct fvec3){0,0,0};
	}
	updateCamVectors(&cam);

	system("clear");
	int i = 0;
	struct timespec ts;
	while (i++ != -1) {
		clock_gettime(CLOCK_MONOTONIC, &ts);

		float angle = set.t * 2.0f;
		set.t = ts.tv_sec + ts.tv_nsec/1e9f;

		spinTris(base, triCount, tris, angle);
		rasterize(&set, pxs, tris, triCount, depthBuffer, &cam, lightDir);
		
		render(&set, pxs);
	}

	free(base); free(tris);
	return 0;
}
