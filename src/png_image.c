#include <png.h>
#include <gs_psm.h>

#include "image.h"

typedef struct {
	size_t pos;
	unsigned char *data;
} png_data;

// Mimics fread()
static void png_read_data( png_structp png_ptr, png_bytep buf, png_size_t size )
{

	png_data *png = (png_data*)png_get_io_ptr(png_ptr);

	// buf is an already allocated buffer by libpng
	memcpy(buf,png->data + png->pos, size);
	png->pos += size;

}

void image_get_png_palette(png_structp png_ptr, png_infop info_ptr, png_uint_32 depth, image_t *image)
{
	int i;
	int entries;
	png_bytep alpha;
	png_color_16p alpha_color;

	png_colorp palette;

	if (png_get_valid(png_ptr,info_ptr,PNG_INFO_PLTE))
	{
		png_get_PLTE(png_ptr,info_ptr,&palette,&entries);
	}
	else
	{
		image->palette->width = 0;
		image->palette->height = 0;
		image->palette->psm = 0;
		image->palette->data = NULL;
		return;
	}

	if (depth == 4)
	{
		image-> palette->width = 8;
		image->palette->height = 2;
		image->palette->psm = GS_PSM_32;
		image->palette->data = (unsigned char*)memalign(16,16*4);
		memset(image->palette->data,0,16*4);

		// PNG seems to only store 32/24-bit palettes
		// 4-bit palettes are stored from 0-15 consecutively
		for (i = 0; i < entries; i++)
		{
			image->palette->data[(i*4)+0] = palette[i].red;
			image->palette->data[(i*4)+1] = palette[i].green;
			image->palette->data[(i*4)+2] = palette[i].blue;
			image->palette->data[(i*4)+3] = 0x80;
			//printf("data  = %08x\n",((u32*)&image->palette->data[0])[i]);
		}

		if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{
			png_get_tRNS(png_ptr,info_ptr,&alpha,&entries,&alpha_color);
			for (i = 0; i < entries; i++)
			{
				image->palette->data[(i*4)+3] = (int)((float)(alpha[i])/2.0f);
				//printf("alpha  = %08x\n",((u32*)&image->palette->data[0])[i]);
			}
		}
	}
	else
	{
		image->palette->width = 16;
		image->palette->height = 16;
		image->palette->psm = GS_PSM_32;
		image->palette->data = (unsigned char*)memalign(16,256*4);
		memset(image->palette->data,0,256*4);

		// PNG seems to only store 32/24-bit palettes
		// 8-bit palettes need to be swizzled
		for (i = 0; i < entries; i++)
		{
			image->palette->data[(swizzle_8[i]*4)+0] = palette[i].red;
			image->palette->data[(swizzle_8[i]*4)+1] = palette[i].green;
			image->palette->data[(swizzle_8[i]*4)+2] = palette[i].blue;
			image->palette->data[(swizzle_8[i]*4)+3] = 0x80;
			//printf("data[%x]  = %08x\n",swizzle_8[i],((u32*)&image->palette->data[0])[swizzle_8[i]]);
		}

		if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{
			png_get_tRNS(png_ptr,info_ptr,&alpha,&entries,&alpha_color);
			for (i = 0; i < entries; i++)
			{
				image->palette->data[(swizzle_8[i]*4)+3] = (int)((float)(alpha[i])/2.0f);
				//printf("alpha  = %08x\n",((u32*)&image->palette->data[0])[i]);
			}
		}
	}


}

void image_get_png_texture(png_structp png_ptr, png_infop info_ptr, image_t *image)
{

	int i,j;
	int row_bytes;
	int row;
	int channels;
	unsigned char **row_pointers;

	int alpha_byte;

	row_bytes = png_get_rowbytes(png_ptr,info_ptr);
	channels = png_get_channels(png_ptr,info_ptr);

	//printf("row_bytes = %d\n",row_bytes);
	//printf("channels = %d\n",channels);

	row_pointers = calloc(image->texture->height, sizeof(png_bytep));

	for (row = 0; row < image->texture->height; row++)
	{
		row_pointers[row] = malloc(row_bytes);
	}

	png_read_image(png_ptr,row_pointers);

	image->texture->data = (unsigned char *)memalign(16,image->texture->height*row_bytes);

	//png_read_image(png_ptr,image->texture->data);
	for (i = 0; i < image->texture->height; i++)
	{
		for (j = 0; j < row_bytes; j++)
		{
			image->texture->data[(i*row_bytes)+j] = row_pointers[i][j];
		}

		// Fix up alpha values
		if( channels == 4 )
		{
			for( row = 0,alpha_byte = 3; alpha_byte < row_bytes; alpha_byte += 4 )
			{
				*(image->texture->data + row + alpha_byte ) = 
					(int)(((float)*(image->texture->data + row + alpha_byte )) / 2.0f);
			}
		}

		row += row_bytes;
	}

	for (row = 0; row < image->texture->height; row++)
	{
		free(row_pointers[row]);
	}

	free(row_pointers);

}

image_t *image_create_from_png(unsigned char *buf)
{
	image_t *image;
	png_infop info_ptr;
	png_structp png_ptr;

	png_data png;

	png_uint_32 width,height;
	png_uint_32 depth;
	png_uint_32 color;
	png_uint_32 interlace;

	if (buf == NULL)
	{
		return NULL;
	}

	if (png_sig_cmp(buf,0,8))
	{
		return NULL;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);

	if (!png_ptr)
	{
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr,NULL,NULL);
		return NULL;
	}

	image = image_init();

	if (image == NULL)
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		return NULL;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		image_free(image);
		return NULL;
	}

	// Setup our mimic file pointer
	png.data = buf;
	png.pos = 0;

	png_set_read_fn(png_ptr,(png_voidp)&png,png_read_data);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr,info_ptr,&width,&height,&depth,&color,&interlace,NULL,NULL);

	image->texture->width = width;
	image->texture->height = height;

	//printf("width = %u\n", width);
	//printf("height = %u\n", height);
	//printf("depth = %u\n", depth);
	//printf("color = %u\n", color);
	//printf("interlace = %u\n", interlace);

	if (depth == 16)
	{
		png_set_strip_16(png_ptr);
	}

	if (color == PNG_COLOR_TYPE_PALETTE)
	{
		if (depth == 4)
		{
			image->texture->psm = GS_PSM_4;
			png_set_packswap(png_ptr);
		}
		else if (depth == 8)
		{
			image->texture->psm = GS_PSM_8;
		}

		image_get_png_palette(png_ptr,info_ptr,depth,image);

		if (image->palette->data == NULL)
		{
			// Have libpng deal with it if there's something wrong
			png_set_expand(png_ptr);

			// Change the color type now that it's expanded
			if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
			{
				color = PNG_COLOR_TYPE_RGB_ALPHA;
			}
			else
			{
				color = PNG_COLOR_TYPE_RGB;
			}
		}

	}

	if (color == PNG_COLOR_TYPE_GRAY)
	{
		image->texture->psm = GS_PSM_32;
		png_set_gray_to_rgb(png_ptr);
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}
	if (color == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		image->texture->psm = GS_PSM_32;
		png_set_gray_to_rgb(png_ptr);
		if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}
		else
		{
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		}
	}

	if (color == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		image->texture->psm = GS_PSM_32;
		if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}
		else
		{
			png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
		}
	}
	else if (color == PNG_COLOR_TYPE_RGB)
	{
		image->texture->psm = GS_PSM_32;
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	png_read_update_info(png_ptr, info_ptr);

	image_get_png_texture(png_ptr,info_ptr,image);

	png_destroy_read_struct(&png_ptr,&info_ptr,NULL);

	return image;

}
