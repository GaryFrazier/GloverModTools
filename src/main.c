#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLO_IMPLEMENTATION
#include "./glo.h"

typedef uint8_t bool;
enum { false, true };

void* slurp_file(const char *fn, size_t *size) {
	FILE *f = fopen(fn, "rb");
	if (!f) {
		printf("Failed to open: %s\n", fn);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	if (!*size) {
		printf("File is empty: %s\n", fn);
		fclose(f);
		return NULL;
	}
	fseek(f, 0, SEEK_SET);
	void *data = malloc(*size);
	if (!data) {
		printf("Malloc failure: %s\n", fn);
		fclose(f);
		return NULL;
	}
	fread(data, 1, *size, f);
	fclose(f);
	return data;
}

// test

void do_test(int argc, char *argv[]) {
	if(argc != 2) {
		printf("USAGE: heck test <in>\n");
		printf("  in: path to a GLO file.\n");
		return;
	}
	GLO_FILE *glo = glo_load(argv[1]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	glo_save(glo, "test.glo");
	glo_free(glo);
	size_t size1, size2;
	uint32_t *data1 = slurp_file(argv[1], &size1);
	uint32_t *data2 = slurp_file("test.glo", &size2);
	bool badsize = false;
	if(size1 != size2) {
		printf("FAIL - Sizes don't match!\n");
		printf("  %I64u vs %I64u\n", size1, size2);
		badsize = true;
	}
	for(size_t i = 0; i < size1/4 && i < size2/4; i++) {
		if(data1[i] != data2[i]) {
			if(badsize) {
				printf("  First difference at %I64X:\n", i * 4);
			} else {
				printf("FAIL - Data does not match at %I64X!\n", i * 4);
			}
			printf("  %08X vs %08X\n", data1[i], data2[i]);
			free(data1);
			free(data2);
			return;
		}
	}
	if(badsize) {
		printf("  Data matches until end of shortest file.\n");
	} else {
		printf("SUCCESS - Files match exactly.\n");
	}
	free(data1);
	free(data2);
}

// texswap

void texswap_mesh(GLO_MESH *mesh, char *from, char *to) {
	for(int f = 0; f < mesh->num_faces; f++) {
		if(strncmp(mesh->faces[f].texture, from, TEX_NAME_LEN) == 0) {
			strncpy(mesh->faces[f].texture, to, TEX_NAME_LEN);
		}
	}
	for(int s = 0; s < mesh->num_sprites; s++) {
		if(strncmp(mesh->sprites[s].texture, from, TEX_NAME_LEN) == 0) {
			strncpy(mesh->sprites[s].texture, to, TEX_NAME_LEN);
		}
	}
	if(mesh->has_child) texswap_mesh(mesh->child, from, to);
	if(mesh->has_next) texswap_mesh(mesh->next, from, to);
}

void do_texswap(int argc, char *argv[]) {
	if(argc != 4 && argc != 5) {
		printf("USAGE: heck texswap <from> <to> <in> [out]\n");
		printf("  from: Texture name to replace\n");
		printf("  to:   New texture name\n");
		printf("  in:   Input GLO file\n");
		printf("  out:  Output file or blank to overwrite <in>\n");
		return;
	}
	GLO_FILE *glo = glo_load(argv[3]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	for(int o = 0; o < glo->num_objects; o++) {
		for(int m = 0; m < glo->objects[o].num_meshes; m++) {
			texswap_mesh(&glo->objects[o].meshes[m], argv[1], argv[2]);
		}
	}
	glo_save(glo, argc == 5 ? argv[4] : argv[3]);
	glo_free(glo);
}

// texwrap

char texlist[256][TEX_NAME_LEN] = {""};

bool texwrap_douv(GLO_VEC2 *uv, char *tex) {
	if(uv->x > 1.0f || uv->y > 1.0f) {
		int i = 0;
		for(; texlist[i][0] != 0; i++) {
			if(strncmp(texlist[i], tex, TEX_NAME_LEN) == 0) break;
		}
		if(texlist[i][0] == 0) {
			// Sort alphabetical
			int j = 0;
			for(; j < i; j++) {
				if(strncmp(texlist[j], tex, TEX_NAME_LEN) > 0) break;
			}
			for(int k = i; k > j; k--) {
				strncpy(texlist[k], texlist[k-1], TEX_NAME_LEN);
			}
			strncpy(texlist[j], tex, TEX_NAME_LEN);
			//printf("%s\n", tex);
		}
		return true;
	}
	return false;
}

void texwrap_mesh(GLO_MESH *mesh) {
	for(int f = 0; f < mesh->num_faces; f++) {
		GLO_FACE *face = &mesh->faces[f];
		for(int v = X; v <= Z; v++) {
			if(texwrap_douv(&face->vrefs[v].uv, face->texture)) break;
		}
	}
	if(mesh->has_child) texwrap_mesh(mesh->child);
	if(mesh->has_next) texwrap_mesh(mesh->next);
}

void do_texwrap(int argc, char *argv[]) {
	if(argc != 2) {
		printf("USAGE: heck texwrap <in>\n");
		printf("  in: path to a GLO file\n");
		return;
	}
	GLO_FILE *glo = glo_load(argv[1]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	for(int o = 0; o < glo->num_objects; o++) {
		for(int m = 0; m < glo->objects[o].num_meshes; m++) {
			texwrap_mesh(&glo->objects[o].meshes[m]);
		}
	}
	glo_free(glo);
	for(int i = 0; texlist[i][0] != 0; i++) {
		printf("%s\n", texlist[i]);
	}
}

// meshdel

void do_meshdel(int argc, char *argv[]) {
	if(argc != 3 && argc != 4) {
		printf("USAGE: heck meshdel <meshname> <in> [out]\n");
		return;
	}
	GLO_FILE *glo = glo_load(argv[2]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	for(int o = 0; o < glo->num_objects; o++) {
		GLO_OBJECT *obj = &glo->objects[o];
		for(int m = 0; m < obj->num_meshes; m++) {
			GLO_MESH *mesh = &obj->meshes[m];
			if(strncmp(mesh->name, argv[1], OBJ_NAME_LEN) == 0) {
				glo_free_mesh(mesh);
				obj->num_meshes--;
				for(int d = m; d < obj->num_meshes; d++) {
					obj->meshes[d] = obj->meshes[d+1];
				}
				m--;
			}
		}
	}
	glo_save(glo, argc == 4 ? argv[3] : argv[2]);
	glo_free(glo);
}

// glo2txt

void do_glo2txt(int argc, char *argv[]) {
	if(argc != 3) {
		printf("USAGE: heck glo2txt <in> <out>\n");
		return;
	}
	GLO_FILE *glo = glo_load(argv[1]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	glo_save_txt(glo, argv[2]);
	glo_free(glo);
}

// txt2glo

void do_txt2glo(int argc, char *argv[]) {
	if(argc != 3) {
		printf("USAGE: heck glo2txt <in> <out>\n");
		return;
	}
	GLO_FILE *glo = glo_load_txt(argv[1]);
	if(!glo) {
		printf("%s\n", glo_error());
		scanf("\npress any key to end");
		exit(1);
	}
	glo_save(glo, argv[2]);
	glo_free(glo);
}

int main(int argc, char *argv[]) {
	if(argc == 1) {
		printf("USAGE: heck <action> <stuff>\n");
		printf("Enter just the action to see the stuff. Actions include:\n");
		printf("  test:    verify load+save results in an exact match\n");
		printf("  texswap: rename textures\n");
		printf("  texwrap: list all textures with >1 UV\n");
		printf("  meshdel: delete meshes matching a name\n");
		printf("  glo2txt: convert glo to text format\n");
		printf("  txt2glo: convert text file back into binary glo\n");
		return 0;
	}

	if(strcmp(argv[1], "test") == 0) do_test(argc-1, argv+1);
	if(strcmp(argv[1], "texswap") == 0) do_texswap(argc-1, argv+1);
	if(strcmp(argv[1], "texwrap") == 0) do_texwrap(argc-1, argv+1);
	if(strcmp(argv[1], "meshdel") == 0) do_meshdel(argc-1, argv+1);
	if(strcmp(argv[1], "glo2txt") == 0) do_glo2txt(argc-1, argv+1);
	if(strcmp(argv[1], "txt2glo") == 0) do_txt2glo(argc-1, argv+1);

	scanf("press any key to end");
	return 0;
}
