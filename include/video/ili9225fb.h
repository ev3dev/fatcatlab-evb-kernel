/*
* linux/include/video/ili9225fb.h -- FB driver for ILI9225 LCD controller
*
* Copyright (C) 2011, Matt Porter
*
* This file is subject to the terms and conditions of the GNU General Public
* License. See the file COPYING in the main directory of this archive for
* more details.
*/

#define DRVNAME            "ili9225fb"
#define WIDTH              220 
#define HEIGHT             176
#define BPP                16

/* Supported display modules */
#define ILI9225_DISPLAY_AF_TFT18                0        /* ILI9225 SPI TFT 1.8" */


/* Init script function */
struct ili9225_function {
        u8 cmd;
        u8 data;
};


/* Init script commands */
enum ili9225_cmd {
        ILI9225_START,
        ILI9225_END,
        ILI9225_CMD,
        ILI9225_DATA,
        ILI9225_DELAY
};


struct ili9225fb_platform_data {
        int rst_gpio;
        int dc_gpio;
};


struct ili9225_page {
	u16	x;
	u16	y;
	u16	*buffer;
	u32	len;
	int	must_update;
};


struct ili9225fb_par {
	struct spi_device	*spi;
	struct fb_info		*info;
	struct ili9225_page	*pages;
	u32			pages_count;
	u8			*buf;
	int			rst;
	int			dc;
};		


