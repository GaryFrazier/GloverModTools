#ifndef __GLO_H__
#define __GLO_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLO_VERSION_INT	0x0001
#define GLO_VERSION_STR	"0.1"

#define packed struct __attribute((__packed__))

#define OBJ_NAME_LEN  24
#define ANIM_NAME_LEN 24
#define TEX_NAME_LEN  16

#define R 0
#define G 1
#define B 2
#define A 3

#define X 0
#define Y 1
#define Z 2
#define W 3

#define U 0
#define V 1

typedef packed {
	char		magic[4];
	uint16_t	version;
} GLO_HEADER;

typedef packed {
	char	name[ANIM_NAME_LEN];
	int		start;
	int		end;
	int		flags;
	float	speed;
} GLO_ANIM;

typedef union {
	float v[2];
	struct { float x, y; };
} GLO_VEC2;

typedef union {
	float v[3];
	struct { float x, y, z; };
} GLO_VEC3;

typedef union {
	float v[4];
	struct { float x, y, z, w; };
} GLO_QUAT;

typedef union {
	uint32_t c;
	uint8_t v[4];
	struct { uint8_t r, g, b, a; };
} GLO_COLOR;

typedef packed {
	int time;
	union {
		GLO_VEC3 vert;
		GLO_QUAT quat;
	};
} GLO_KEYF;

typedef packed {
	uint16_t	index;
	GLO_VEC2	uv;
} GLO_VREF;

typedef packed {
	char		texture[TEX_NAME_LEN];
	GLO_COLOR	color;
	uint16_t	flags;
	GLO_VREF	vrefs[3];
} GLO_FACE;

typedef packed {
	char		texture[TEX_NAME_LEN];
	GLO_COLOR	color;
	GLO_VEC3	pos;
	GLO_VEC2	size;
	uint16_t	flags;
} GLO_SPRITE;

typedef packed GLO_MESH GLO_MESH;
typedef packed GLO_MESH {
	char		name[OBJ_NAME_LEN];
	uint16_t	num_movekeys;
	GLO_KEYF	*movekeys;
	uint16_t	num_scalekeys;
	GLO_KEYF	*scalekeys;
	uint16_t	num_rotatekeys;
	GLO_KEYF	*rotatekeys;
	uint16_t	num_vertices;
	GLO_VEC3	*vertices;
	uint16_t	num_faces;
	GLO_FACE	*faces;
	uint16_t	num_sprites;
	GLO_SPRITE	*sprites;
	uint16_t	xlu;
	uint16_t	flags;
	uint16_t	has_child;
	GLO_MESH	*child;
	uint16_t	has_next;
	GLO_MESH	*next;
} GLO_MESH;

typedef packed {
	uint16_t	num_anims;
	GLO_ANIM	*anims;
	uint16_t	num_meshes;
	GLO_MESH	*meshes;
} GLO_OBJECT;

typedef packed {
	GLO_HEADER	head;
	uint16_t	num_objects;
	GLO_OBJECT	*objects;
} GLO_FILE;

GLO_FILE* glo_load(const char *fname);
void glo_save(GLO_FILE *glo, const char *fname);
void glo_free(GLO_FILE *glo);
void glo_free_mesh(GLO_MESH *mesh);

const char* glo_error();

#ifdef GLO_IMPLEMENTATION

#define ERRMSG_LEN	256
char errmsg[ERRMSG_LEN] = {0};
#define SETERR(...) snprintf(errmsg, ERRMSG_LEN, __VA_ARGS__)

const char* glo_error() { return errmsg; }

// GLO LOAD (binary)

static void _glo_load_mesh(GLO_MESH *mesh, FILE *f) {
	fread(mesh->name, 1, OBJ_NAME_LEN, f);
	//printf(" %s\n", mesh->name);
	fread(&mesh->num_movekeys, 1, sizeof(uint16_t), f);
	//printf("  MoveKeys: %d\n", mesh->num_movekeys);
	if(mesh->num_movekeys) {
		mesh->movekeys = calloc(mesh->num_movekeys, sizeof(GLO_KEYF));
		for(int k = 0; k < mesh->num_movekeys; k++) {
			fread(&mesh->movekeys[k], 1, sizeof(int)+sizeof(GLO_VEC3), f);
		}
	}
	fread(&mesh->num_scalekeys, 1, sizeof(uint16_t), f);
	//printf("  ScaleKeys: %d\n", mesh->num_scalekeys);
	if(mesh->num_scalekeys) {
		mesh->scalekeys = calloc(mesh->num_scalekeys, sizeof(GLO_KEYF));
		for(int k = 0; k < mesh->num_scalekeys; k++) {
			fread(&mesh->scalekeys[k], 1, sizeof(int)+sizeof(GLO_VEC3), f);
		}
	}
	fread(&mesh->num_rotatekeys, 1, sizeof(uint16_t), f);
	//printf("  RotateKeys: %d\n", mesh->num_rotatekeys);
	if(mesh->num_rotatekeys) {
		mesh->rotatekeys = calloc(mesh->num_rotatekeys, sizeof(GLO_KEYF));
		for(int k = 0; k < mesh->num_rotatekeys; k++) {
			fread(&mesh->rotatekeys[k], 1, sizeof(int)+sizeof(GLO_QUAT), f);
		}
	}
	fread(&mesh->num_vertices, 1, sizeof(uint16_t), f);
	//printf("  Vertices: %d\n", mesh->num_vertices);
	if(mesh->num_vertices) {
		mesh->vertices = calloc(mesh->num_vertices, sizeof(GLO_VEC3));
		fread(mesh->vertices, mesh->num_vertices, sizeof(GLO_VEC3), f);
		//for(int i = 0; i < mesh->num_vertices; i++) {
		//	printf("   %f, %f, %f\n", mesh->vertices[i].v[0],
		//		mesh->vertices[i].v[1], mesh->vertices[i].v[2]);
		//}
	}
	fread(&mesh->num_faces, 1, sizeof(uint16_t), f);
	//printf("  Faces: %d\n", mesh->num_faces);
	if(mesh->num_faces) {
		mesh->faces = calloc(mesh->num_faces, sizeof(GLO_FACE));
		fread(mesh->faces, mesh->num_faces, sizeof(GLO_FACE), f);
		//for(int i = 0; i < mesh->num_faces; i++) {
		//	GLO_FACE *face = &mesh->faces[i];
		//	printf("   %d, %d, %d, %d\n", face->color.r,
		//		face->color.g, face->color.b, face->color.a);
		//	printf("    (%f, %f), (%f, %f), (%f, %f)\n", face->vrefs[0].uv.x,face->vrefs[0].uv.y,
		//		face->vrefs[1].uv.x,face->vrefs[1].uv.y,face->vrefs[2].uv.x,face->vrefs[2].uv.y);
		//}
	}
	fread(&mesh->num_sprites, 1, sizeof(uint16_t), f);
	//printf("  Sprites: %d\n", mesh->num_sprites);
	if(mesh->num_sprites) {
		mesh->sprites = calloc(mesh->num_sprites, sizeof(GLO_SPRITE));
		fread(mesh->sprites, mesh->num_sprites, sizeof(GLO_SPRITE), f);
	}
	fread(&mesh->xlu, 1, sizeof(uint16_t), f);
	//printf("  Translucency: %d\n", mesh->xlu);
	fread(&mesh->flags, 1, sizeof(uint16_t), f);
	//printf("  Flags: %d\n", mesh->flags);
	fread(&mesh->has_child, 1, sizeof(uint16_t), f);
	//printf("  Child: %d\n", mesh->has_child);
	if(mesh->has_child) {
		mesh->child = calloc(1, sizeof(GLO_MESH));
		_glo_load_mesh(mesh->child, f);
	}
	fread(&mesh->has_next, 1, sizeof(uint16_t), f);
	//printf("  Next: %d\n", mesh->has_next);
	if(mesh->has_next) {
		mesh->next = calloc(1, sizeof(GLO_MESH));
		_glo_load_mesh(mesh->next, f);
	}
}

GLO_FILE* glo_load(const char *fname) {
	FILE *f = fopen(fname, "rb");
	if(!f) {
		SETERR("Failed to open '%s'.\n", fname);
		return NULL;
	}
	GLO_FILE *glo = malloc(sizeof(GLO_FILE));
	fread(&glo->head, 1, sizeof(GLO_HEADER), f);
	if(strcmp(glo->head.magic, "GLO") != 0) {
		free(glo);
		fclose(f);
		SETERR("Invalid GLO header.\n");
		return NULL;
	}
	if(glo->head.version != GLO_VERSION_INT) {
		free(glo);
		fclose(f);
		SETERR("Bad GLO version.\n");
		return NULL;
	}
	fread(&glo->num_objects, 1, sizeof(uint16_t), f);
	//printf("Objects: %d\n", glo->num_objects);
	glo->objects = calloc(glo->num_objects, sizeof(GLO_OBJECT));
	for(int o = 0; o < glo->num_objects; o++) {
		GLO_OBJECT *obj = &glo->objects[o];
		fread(&obj->num_anims, 1, sizeof(uint16_t), f);
		//printf(" Anims: %d\n", obj->num_anims);
		if(obj->num_anims) {
			obj->anims = calloc(obj->num_anims, sizeof(GLO_ANIM));
			fread(obj->anims, obj->num_anims, sizeof(GLO_ANIM), f);
		}
		fread(&obj->num_meshes, 1, sizeof(uint16_t), f);
		//printf(" Meshes: %d\n", obj->num_meshes);
		if(obj->num_meshes) {
			obj->meshes = calloc(obj->num_meshes, sizeof(GLO_MESH));
			for(int m = 0; m < obj->num_meshes; m++) {
				_glo_load_mesh(&obj->meshes[m], f);
			}
		}
	}
	fclose(f);
	return glo;
}

// GLO LOAD (text)

#define isspace(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n')

static int txtln;
static char token[80];

#define ALLOC_OBJ	256
#define ALLOC_ANIM	256
#define ALLOC_MESH	256
#define ALLOC_KEYF	256
#define ALLOC_VTX	256
#define ALLOC_FACE	256
#define ALLOC_SPR	256

static char* _wsp(char *pos) {
	for(;;) {
		switch(*pos) {
			case '\n':
			txtln++; // Fallthrough
			case ' ': case '\t': case '\r':
			pos++;
			break;
			case ';':
			while(*pos != '\n' && *pos != 0) pos++;
			break;
			default: return pos;
		}
	}
}

static int _tklen(char *pos) {
	int len = 0;
	for(;;) {
		if(pos[0] == '"') { // Strings might have spaces
			if(len > 0 && pos[len] == '"') return len+1;
		} else {
			if(isspace(pos[len]) || pos[len] == 0) return len;
		}
		len++;
	}
}

#define NEXT_LINE { \
	while(*pos != '\n' && *pos != 0) pos++; \
}
#define NEXT_TOKEN { \
	pos = _wsp(pos); \
	int len = _tklen(pos); \
	strncpy(token, pos, len); \
	token[len] = 0; \
	pos += len; \
}
#define CMP_TOKEN(str) (strcmp(token, str) == 0)
#define ASSERT_TOKEN(str, ...) if(!CMP_TOKEN(str)) { \
	SETERR(__VA_ARGS__); \
	return NULL; \
}

#define PARSE_STRING(res, maxlen) { \
	if(token[0] != '"') { \
		SETERR("%d: Expected string.\n", txtln); \
		return NULL; \
	} \
	int len = 0; \
	while(token[len+1] != '"') { \
		if(len >= maxlen) { \
			SETERR("%d: String too long, max %d.\n", txtln, maxlen); \
			return NULL; \
		} \
		len++; \
	} \
	strncpy(res, token+1, len); \
	res[len] = 0; \
}
#define PARSE_NUM(res, fmt, ...) { \
	int num = sscanf(token, fmt, &res); \
	if(!num) { \
		SETERR(__VA_ARGS__); \
		return NULL; \
	} \
}

static char* _glo_load_mesh_txt(GLO_MESH *mesh, char *pos) {
	NEXT_TOKEN;
	PARSE_STRING(mesh->name, OBJ_NAME_LEN);
	NEXT_TOKEN;
	PARSE_NUM(mesh->xlu, "%hX", "%d: Bad 'xlu' for 'mesh'\n", txtln);
	NEXT_TOKEN;
	PARSE_NUM(mesh->flags, "%hX", "%d: Bad 'flags' for 'mesh'\n", txtln);
	NEXT_TOKEN;
	ASSERT_TOKEN("{", "%d: Expected '{'\n", txtln);
	NEXT_TOKEN;
	while(CMP_TOKEN("movekey")) {
		if(!mesh->num_movekeys) mesh->movekeys = calloc(ALLOC_KEYF, sizeof(GLO_KEYF));
		GLO_KEYF *key = &mesh->movekeys[mesh->num_movekeys];
		mesh->num_movekeys++;
		NEXT_TOKEN;
		PARSE_NUM(key->time, "%d", "%d: Bad 'time' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.x, "%f", "%d: Bad 'x' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.y, "%f", "%d: Bad 'y' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.z, "%f", "%d: Bad 'z' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
	}
	while(CMP_TOKEN("scalekey")) {
		if(!mesh->num_scalekeys) mesh->scalekeys = calloc(ALLOC_KEYF, sizeof(GLO_KEYF));
		GLO_KEYF *key = &mesh->scalekeys[mesh->num_scalekeys];
		mesh->num_scalekeys++;
		NEXT_TOKEN;
		PARSE_NUM(key->time, "%d", "%d: Bad 'time' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.x, "%f", "%d: Bad 'x' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.y, "%f", "%d: Bad 'y' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->vert.z, "%f", "%d: Bad 'z' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
	}
	while(CMP_TOKEN("rotatekey")) {
		if(!mesh->num_rotatekeys) mesh->rotatekeys = calloc(ALLOC_KEYF, sizeof(GLO_KEYF));
		GLO_KEYF *key = &mesh->rotatekeys[mesh->num_rotatekeys];
		mesh->num_rotatekeys++;
		NEXT_TOKEN;
		PARSE_NUM(key->time, "%d", "%d: Bad 'time' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->quat.x, "%f", "%d: Bad 'x' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->quat.y, "%f", "%d: Bad 'y' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->quat.z, "%f", "%d: Bad 'z' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(key->quat.w, "%f", "%d: Bad 'w' for 'keyframe'.\n", txtln);
		NEXT_TOKEN;
	}
	while(CMP_TOKEN("vertex")) {
		if(!mesh->num_vertices) mesh->vertices = calloc(ALLOC_VTX, sizeof(GLO_VEC3));
		GLO_VEC3 *vtx = &mesh->vertices[mesh->num_vertices];
		mesh->num_vertices++;
		NEXT_TOKEN;
		PARSE_NUM(vtx->x, "%f", "%d: Bad 'x' for 'vertex'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(vtx->y, "%f", "%d: Bad 'y' for 'vertex'.\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(vtx->z, "%f", "%d: Bad 'z' for 'vertex'.\n", txtln);
		NEXT_TOKEN;
	}
	while(CMP_TOKEN("face")) {
		if(!mesh->num_faces) mesh->faces = calloc(ALLOC_FACE, sizeof(GLO_FACE));
		GLO_FACE *face = &mesh->faces[mesh->num_faces];
		mesh->num_faces++;
		NEXT_TOKEN;
		PARSE_STRING(face->texture, TEX_NAME_LEN);
		NEXT_TOKEN;
		PARSE_NUM(face->color.c, "%X", "%d: Bad color for 'face'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(face->flags, "%hX", "%d: Bad flag for 'face'\n", txtln);
		NEXT_TOKEN;
		ASSERT_TOKEN("{", "%d: Expected '{'\n", txtln);
		NEXT_TOKEN;
		for(int i = 0; i < 3; i++) {
			ASSERT_TOKEN("vref", "%d: Expected vref.\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(face->vrefs[i].index, "%hd", "%d: Bad index for 'vref'\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(face->vrefs[i].uv.x, "%f", "%d: Bad 'u' for 'vref'\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(face->vrefs[i].uv.y, "%f", "%d: Bad 'v' for 'vref'\n", txtln);
			NEXT_TOKEN;
		}
		ASSERT_TOKEN("}", "%d: Face vrefs not closed.\n", txtln);
		NEXT_TOKEN;
	}
	while(CMP_TOKEN("sprite")) {
		if(!mesh->num_sprites) mesh->sprites = calloc(ALLOC_SPR, sizeof(GLO_SPRITE));
		GLO_SPRITE *sprite = &mesh->sprites[mesh->num_sprites];
		mesh->num_sprites++;
		NEXT_TOKEN;
		PARSE_STRING(sprite->texture, TEX_NAME_LEN);
		NEXT_TOKEN;
		PARSE_NUM(sprite->color.c, "%X", "%d: Bad color for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->flags, "%hX", "%d: Bad flag for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->pos.x, "%f", "%d: Bad 'x pos' for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->pos.y, "%f", "%d: Bad 'y pos' for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->pos.z, "%f", "%d: Bad 'z pos' for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->size.x, "%f", "%d: Bad 'x size' for 'sprite'\n", txtln);
		NEXT_TOKEN;
		PARSE_NUM(sprite->size.y, "%f", "%d: Bad 'y size' for 'sprite'\n", txtln);
		NEXT_TOKEN;
	}
	if(CMP_TOKEN("child")) {
		mesh->has_child = 1;
		mesh->child = calloc(1, sizeof(GLO_MESH));
		pos = _glo_load_mesh_txt(mesh->child, pos);
		if(!pos) return NULL;
		NEXT_TOKEN;
	}
	if(CMP_TOKEN("next")) {
		mesh->has_next = 1;
		mesh->next = calloc(1, sizeof(GLO_MESH));
		pos = _glo_load_mesh_txt(mesh->next, pos);
		if(!pos) return NULL;
		NEXT_TOKEN;
	}
	ASSERT_TOKEN("}", "%d: Mesh not closed.\n", txtln);
	return pos;
}

GLO_FILE *glo_load_txt(const char *fname) {
	FILE *f = fopen(fname, "rb");
	if(!f) {
		SETERR("Failed to open '%s'.\n", fname);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	if (!len) {
		SETERR("File is empty '%s'\n", fname);
		fclose(f);
		return NULL;
	}
	fseek(f, 0, SEEK_SET);
	char *txt = malloc(len+1);
	fread(txt, 1, len, f);
	txt[len] = 0;
	fclose(f);
	GLO_FILE *glo = calloc(1, sizeof(GLO_FILE));
	strcpy(glo->head.magic, "GLO");
	glo->head.version = GLO_VERSION_INT;
	char *pos = txt;
	txtln = 1;
	NEXT_TOKEN;
	while(CMP_TOKEN("object")) {
		if(!glo->num_objects) glo->objects = calloc(ALLOC_OBJ, sizeof(GLO_OBJECT));
		GLO_OBJECT *obj = &glo->objects[glo->num_objects];
		glo->num_objects++;
		NEXT_TOKEN;
		ASSERT_TOKEN("{", "%d: Expected '{' after 'object'\n", txtln);
		NEXT_TOKEN;
		while(CMP_TOKEN("anim")) {
			if(!obj->num_anims) obj->anims = calloc(ALLOC_ANIM, sizeof(GLO_ANIM));
			GLO_ANIM *anim = &obj->anims[obj->num_anims];
			obj->num_anims++;
			NEXT_TOKEN;
			PARSE_STRING(anim->name, ANIM_NAME_LEN);
			NEXT_TOKEN;
			PARSE_NUM(anim->start, "%d", "%d: Bad 'start' for 'anim'\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(anim->end, "%d", "%d: Bad 'end' for 'anim'\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(anim->flags, "%X", "%d: Bad 'flags' for 'anim'\n", txtln);
			NEXT_TOKEN;
			PARSE_NUM(anim->speed, "%f", "%d: Bad 'speed' for 'anim'\n", txtln);
			NEXT_TOKEN;
		}
		while(CMP_TOKEN("mesh")) {
			if(!obj->num_meshes) obj->meshes = calloc(ALLOC_MESH, sizeof(GLO_MESH));
			GLO_MESH *mesh = &obj->meshes[obj->num_meshes];
			obj->num_meshes++;
			pos = _glo_load_mesh_txt(mesh, pos);
			if(!pos) return NULL;
			NEXT_TOKEN;
		}
		ASSERT_TOKEN("}", "%d: Object not closed.\n", txtln);
		NEXT_TOKEN;
	}
	free(txt);
	return glo;
}

// GLO SAVE (binary)

static void _glo_save_mesh(GLO_MESH *mesh, FILE *f) {
	fwrite(mesh->name, 1, OBJ_NAME_LEN, f);
	fwrite(&mesh->num_movekeys, 1, sizeof(uint16_t), f);
	for(int k = 0; k < mesh->num_movekeys; k++) {
		fwrite(&mesh->movekeys[k], 1, sizeof(int)+sizeof(GLO_VEC3), f);
	}
	fwrite(&mesh->num_scalekeys, 1, sizeof(uint16_t), f);
	for(int k = 0; k < mesh->num_scalekeys; k++) {
		fwrite(&mesh->scalekeys[k], 1, sizeof(int)+sizeof(GLO_VEC3), f);
	}
	fwrite(&mesh->num_rotatekeys, 1, sizeof(uint16_t), f);
	for(int k = 0; k < mesh->num_rotatekeys; k++) {
		fwrite(&mesh->rotatekeys[k], 1, sizeof(int)+sizeof(GLO_QUAT), f);
	}
	fwrite(&mesh->num_vertices, 1, sizeof(uint16_t), f);
	fwrite(mesh->vertices, mesh->num_vertices, sizeof(GLO_VEC3), f);
	fwrite(&mesh->num_faces, 1, sizeof(uint16_t), f);
	fwrite(mesh->faces, mesh->num_faces, sizeof(GLO_FACE), f);
	fwrite(&mesh->num_sprites, 1, sizeof(uint16_t), f);
	fwrite(mesh->sprites, mesh->num_sprites, sizeof(GLO_SPRITE), f);
	fwrite(&mesh->xlu, 1, sizeof(uint16_t), f);
	fwrite(&mesh->flags, 1, sizeof(uint16_t), f);
	fwrite(&mesh->has_child, 1, sizeof(uint16_t), f);
	if(mesh->has_child) _glo_save_mesh(mesh->child, f);
	fwrite(&mesh->has_next, 1, sizeof(uint16_t), f);
	if(mesh->has_next) _glo_save_mesh(mesh->next, f);
}

void glo_save(GLO_FILE *glo, const char *fname) {
	FILE *f = fopen(fname, "wb");
	if(!f) {
		SETERR("Failed to open '%s'.\n", fname);
		return;
	}
	fwrite(&glo->head, 1, sizeof(GLO_HEADER), f);
	fwrite(&glo->num_objects, 1, sizeof(uint16_t), f);
	for(int i = 0; i < glo->num_objects; i++) {
		GLO_OBJECT *obj = &glo->objects[i];
		fwrite(&obj->num_anims, 1, sizeof(uint16_t), f);
		for(int a = 0; a < obj->num_anims; a++) {
			fwrite(&obj->anims[a], 1, sizeof(GLO_ANIM), f);
		}
		fwrite(&obj->num_meshes, 1, sizeof(uint16_t), f);
		for(int m = 0; m < obj->num_meshes; m++) {
			_glo_save_mesh(&obj->meshes[m], f);
		}
	}
	fclose(f);
}

// GLO SAVE (text)

static void newline(FILE *txt, int tabs) {
	fprintf(txt, "\n");
	for(int i = 0; i < tabs; i++) fprintf(txt, "\t");
}

static void glo_save_txt_mesh(GLO_MESH *mesh, FILE *txt, int tabs) {
	fprintf(txt, "\"%s\" %04X %04X {", mesh->name, mesh->xlu, mesh->flags);
	tabs++;
	for(int k = 0; k < mesh->num_movekeys; k++) {
		GLO_KEYF *key = &mesh->movekeys[k];
		newline(txt, tabs);
		fprintf(txt, "movekey %d %f %f %f", key->time, 
				key->vert.x, key->vert.y, key->vert.z);
	}
	for(int k = 0; k < mesh->num_scalekeys; k++) {
		GLO_KEYF *key = &mesh->scalekeys[k];
		newline(txt, tabs);
		fprintf(txt, "scalekey %d %f %f %f", key->time, 
				key->vert.x, key->vert.y, key->vert.z);
	}
	for(int k = 0; k < mesh->num_rotatekeys; k++) {
		GLO_KEYF *key = &mesh->rotatekeys[k];
		newline(txt, tabs);
		fprintf(txt, "rotatekey %d %f %f %f %f", key->time, 
				key->quat.x, key->quat.y, key->quat.z, key->quat.w);
	}
	for(int v = 0; v < mesh->num_vertices; v++) {
		GLO_VEC3 *vtx = &mesh->vertices[v];
		newline(txt, tabs);
		fprintf(txt, "vertex %f %f %f", vtx->x, vtx->y, vtx->z);
	}
	for(int f = 0; f < mesh->num_faces; f++) {
		GLO_FACE *face = &mesh->faces[f];
		newline(txt, tabs);
		fprintf(txt, "face \"%s\" %08X %04X {", 
				face->texture, face->color.c, face->flags);
		tabs++;
		for(int v = 0; v < 3; v++) {
			GLO_VREF *vref = &face->vrefs[v];
			newline(txt, tabs);
			fprintf(txt, "vref %d %f %f", vref->index, vref->uv.x, vref->uv.y);
		}
		tabs--;
		newline(txt, tabs);
		fprintf(txt, "}");
	}
	for(int s = 0; s < mesh->num_sprites; s++) {
		GLO_SPRITE *sprite = &mesh->sprites[s];
		newline(txt, tabs);
		fprintf(txt, "sprite \"%s\" %08X %04X %f %f %f %f %f", 
				sprite->texture, sprite->color.c, sprite->flags,
				sprite->pos.x, sprite->pos.y, sprite->pos.z, 
				sprite->size.x, sprite->size.y);
	}
	if(mesh->has_child) {
		GLO_MESH *child = mesh->child;
		newline(txt, tabs);
		fprintf(txt, "child ");
		glo_save_txt_mesh(child, txt, tabs);
	}
	if(mesh->has_next) {
		GLO_MESH *next = mesh->next;
		newline(txt, tabs);
		fprintf(txt, "next ");
		glo_save_txt_mesh(next, txt, tabs);
	}
	tabs--;
	newline(txt, tabs);
	fprintf(txt, "}");
}

void glo_save_txt(GLO_FILE *glo, const char *fname) {
	FILE *txt = fopen(fname, "w");
	if(!txt) {
		SETERR("Failed to open '%s'.\n", fname);
		return;
	}
	fprintf(txt, "; %s - %s\n", fname, GLO_VERSION_STR);
	int tabs = 0;
	for(int o = 0; o < glo->num_objects; o++) {
		GLO_OBJECT *obj = &glo->objects[o];
		newline(txt, tabs);
		fprintf(txt, "object {");
		tabs++;
		for(int a = 0; a < obj->num_anims; a++) {
			GLO_ANIM *anim = &obj->anims[a];
			newline(txt, tabs);
			fprintf(txt, "anim \"%s\" %d %d %d %f", anim->name,
					anim->start, anim->end, anim->flags, anim->speed);
		}
		for(int m = 0; m < obj->num_meshes; m++) {
			GLO_MESH *mesh = &obj->meshes[m];
			newline(txt, tabs);
			fprintf(txt, "mesh ");
			glo_save_txt_mesh(mesh, txt, tabs);
		}
		tabs--;
		newline(txt, tabs);
		fprintf(txt, "}");
	}
	newline(txt, tabs);
	fclose(txt);
}

// GLO FREE

void glo_free_mesh(GLO_MESH *mesh) {
	if(mesh->num_movekeys) free(mesh->movekeys);
	if(mesh->num_scalekeys) free(mesh->scalekeys);
	if(mesh->num_rotatekeys) free(mesh->rotatekeys);
	if(mesh->num_vertices) free(mesh->vertices);
	if(mesh->num_faces) free(mesh->faces);
	if(mesh->num_sprites) free(mesh->sprites);
	if(mesh->has_child) glo_free_mesh(mesh->child);
	if(mesh->has_next) glo_free_mesh(mesh->next);
}

void glo_free(GLO_FILE *glo) {
	for(int i = 0; i < glo->num_objects; i++) {
		if(glo->objects[i].num_anims) free(glo->objects[i].anims);
		for(int j = 0; j < glo->objects[i].num_meshes; j++) {
			glo_free_mesh(&glo->objects[i].meshes[j]);
		}
		free(glo->objects[i].meshes);
	}
	if(glo->num_objects) free(glo->objects);
	free(glo);
}

#endif
#endif
