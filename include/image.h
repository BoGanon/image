#ifndef __IMAGE_H__
#define __IMAGE_H__

typedef struct {
	unsigned char *data;
	unsigned short width;
	unsigned short height;
	unsigned char psm;
} texture_t;

typedef struct {
	unsigned char *data;
	unsigned short width;
	unsigned short height;
	unsigned char psm;
} palette_t;

typedef struct {
	texture_t *texture;
	palette_t *palette;
} image_t;

// Array to swizzle palette entries for the GS
//	for (i=0;i<256;i++)
//		gs_palette[swizzle_8[i]] = palette[i];
extern int swizzle_8[];

#ifdef __cplusplus
extern "C" {
#endif

	image_t *image_init();
	void     image_free(image_t *image);

	image_t *image_load_png_buffer(unsigned char *pngbuf);
	image_t *image_load_png_file(char *path);
#ifdef __cplusplus
};
#endif

#endif /*__IMAGE_H__*/
