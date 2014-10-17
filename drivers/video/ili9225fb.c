/*
* linux/drivers/video/ili9225fb.c -- FB driver for ILI9225 LCD controller
* Layout is based on skeletonfb.c by James Simmons and Geert Uytterhoeven.
*
* Copyright (C) 2011, Matt Porter
*
* This file is subject to the terms and conditions of the GNU General Public
* License. See the file COPYING in the main directory of this archive for
* more details.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <linux/of_device.h>
#include <video/ili9225fb.h>


static struct ili9225_function ili9225_cfg_script[] = {

        {ILI9225_START, ILI9225_START},


	{ILI9225_CMD,  0x01},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x1C},

	{ILI9225_CMD,  0x02},
	{ILI9225_DATA, 0x01},
	{ILI9225_DATA, 0x00},

	{ILI9225_CMD,  0x03},
	{ILI9225_DATA, 0x10},
	{ILI9225_DATA, 0x38},

	{ILI9225_CMD,  0x08},
	{ILI9225_DATA, 0x08},
	{ILI9225_DATA, 0x08}, 




	{ILI9225_CMD,  0x0C},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x0F},
	{ILI9225_DATA, 0x0E},
	{ILI9225_DATA, 0x01}, 

	{ILI9225_CMD,  0x20},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x21},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 


	{ILI9225_DELAY, 50}, 


	{ILI9225_CMD,  0x10},
	{ILI9225_DATA, 0x09},
	{ILI9225_DATA, 0x00}, 


	{ILI9225_CMD,  0x11},
	{ILI9225_DATA, 0x10},
	{ILI9225_DATA, 0x38}, 


	{ILI9225_DELAY, 50}, 


	{ILI9225_CMD,  0x12},
	{ILI9225_DATA, 0x11},
	{ILI9225_DATA, 0x21}, 


	{ILI9225_CMD,  0x13},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x65}, 


	{ILI9225_CMD,  0x14},
	{ILI9225_DATA, 0x50},
	{ILI9225_DATA, 0x58}, 


//Set GRAM area
	{ILI9225_CMD,  0x30},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x31},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0xDB}, 

	{ILI9225_CMD,  0x32},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x33},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x34},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0xDB}, 

	{ILI9225_CMD,  0x35},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x36},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0xAF}, 

	{ILI9225_CMD,  0x37},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x38},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0xDB}, 

	{ILI9225_CMD,  0x39},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x00}, 


//gamma
	{ILI9225_CMD,  0x50},
	{ILI9225_DATA, 0x04},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x51},
	{ILI9225_DATA, 0x06},
	{ILI9225_DATA, 0x0B}, 

	{ILI9225_CMD,  0x52},
	{ILI9225_DATA, 0x0C},
	{ILI9225_DATA, 0x0A}, 

	{ILI9225_CMD,  0x53},
	{ILI9225_DATA, 0x01},
	{ILI9225_DATA, 0x05}, 

	{ILI9225_CMD,  0x54},
	{ILI9225_DATA, 0x0A},
	{ILI9225_DATA, 0x0C}, 

	{ILI9225_CMD,  0x55},
	{ILI9225_DATA, 0x0B},
	{ILI9225_DATA, 0x06}, 

	{ILI9225_CMD,  0x56},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x04}, 

	{ILI9225_CMD,  0x57},
	{ILI9225_DATA, 0x05},
	{ILI9225_DATA, 0x01}, 

	{ILI9225_CMD,  0x58},
	{ILI9225_DATA, 0x0E},
	{ILI9225_DATA, 0x00}, 

	{ILI9225_CMD,  0x59},
	{ILI9225_DATA, 0x00},
	{ILI9225_DATA, 0x0E}, 


	{ILI9225_DELAY, 50},


	{ILI9225_CMD,  0x07},
	{ILI9225_DATA, 0x10},
	{ILI9225_DATA, 0x17}, 


        {ILI9225_END, ILI9225_END}

};

static struct fb_fix_screeninfo ili9225fb_fix = {
        .id 		=   "ILI9225",
        .type 		=   FB_TYPE_PACKED_PIXELS,
 //       .visual 	=   FB_VISUAL_PSEUDOCOLOR,
	.visual		=   FB_VISUAL_TRUECOLOR,
        .xpanstep 	=   0,
        .ypanstep 	=   0,
        .ywrapstep 	=   0,
        .line_length 	=   WIDTH * BPP / 8,
        .accel 		=   FB_ACCEL_NONE,
};

static struct fb_var_screeninfo ili9225fb_var = {
        .xres 		=   WIDTH,
        .yres 		=   HEIGHT,
        .xres_virtual 	=   WIDTH,
        .yres_virtual 	=   HEIGHT,
        .bits_per_pixel =   BPP,
        .red		=   {11, 5, 0},
        .green		=   {5, 6, 0},
        .blue		=   {0, 5, 0},
        .activate	= FB_ACTIVATE_NOW,
        .vmode		= FB_VMODE_NONINTERLACED,
  //      .nonstd        	=   1,
};

static int ili9225_write(struct ili9225fb_par *par, u8 data)
{
        par->buf[0] = data;
        return spi_write(par->spi, par->buf, 1);
}

static void ili9225_write_data(struct ili9225fb_par *par, u8 data)
{
        int ret = 0;

        /* Set data mode */
        gpio_set_value(par->dc, 1);

        ret = ili9225_write(par, data);
        if (ret < 0)
                pr_err("%s: write data %02x failed with status %d\n",
                        par->info->fix.id, data, ret);
}





static int ili9225_write_data_buf(struct ili9225fb_par *par,
                                        u8 *txbuf, int size)
{
	int p = 0, ret = 0;

        /* Set data mode */
        gpio_set_value(par->dc, 1);

        /* Write entire buffer */
	while(size > 4096)
	{
		 ret += spi_write(par->spi, &txbuf[p], 4096);
	 	 p += 4096;
	 	 size -= 4096;
	}
	if(size != 0 && size <= 4096)
	{
	 	 ret += spi_write(par->spi, &txbuf[p], size);
	}

	return ret;
}

static void ili9225_write_cmd(struct ili9225fb_par *par, u8 data)
{
        int ret = 0;

        /* Set command mode */
        gpio_set_value(par->dc, 0);

        ret = ili9225_write(par, data);
        if (ret < 0)
                pr_err("%s: write command %02x failed with status %d\n",
                        par->info->fix.id, data, ret);
}




static void ili9225_run_cfg_script(struct ili9225fb_par *par)
{
        int i = 0;
        int end_script = 0;

        do {
                switch (ili9225_cfg_script[i].cmd)
                {
                	case ILI9225_START:
                        	break;
                	case ILI9225_CMD:
                        	ili9225_write_cmd(par, ili9225_cfg_script[i].data & 0xff);
                        	break;
                	case ILI9225_DATA:
                        	ili9225_write_data(par, ili9225_cfg_script[i].data & 0xff);
                        	break;
                	case ILI9225_DELAY:
                        	mdelay(ili9225_cfg_script[i].data);
                        	break;
                	case ILI9225_END:
                        	end_script = 1;
                }
                i++;
        } while (!end_script);

}


static void ili9225_set_addr(struct ili9225fb_par *par, int x, int y)
{
	ili9225_write_cmd(par, 0x21);
	ili9225_write_data(par, 0x0);
	ili9225_write_data(par, (u8)x);

	ili9225_write_cmd(par, 0x20);
	ili9225_write_data(par, 0x0);
	ili9225_write_data(par, (u8)y);

	ili9225_write_cmd(par, 0x22);
}


static void ili9225_reset(struct ili9225fb_par *par)
{
        /* Reset controller */
        gpio_set_value(par->rst, 0);
        mdelay(100);
        gpio_set_value(par->rst, 1);
        mdelay(50);
}




static u8 ili9225_spiblock[4096 * 2];

static int ili9225_spi_write_datablock(struct ili9225fb_par *par, unsigned short *block, int len)
{
	int x;
	unsigned short value;

	if(len > 4096)
	{
		len = 4096;
	}
	
	for(x = 0; x < len; x++)
	{
		value = block[x];
		ili9225_spiblock[x * 2 + 1] = (value >> 8) & 0xff;
		ili9225_spiblock[x * 2] = value & 0xff;
	}

	ili9225_write_data_buf(par, ili9225_spiblock, len * 2);

	return 0;
}










static int  ili9225_video_alloc(struct ili9225fb_par *par)
{
	u32	vmem_size;

	vmem_size = par->info->fix.line_length * par->info->var.yres;
	par->pages_count = vmem_size / PAGE_SIZE;

	if((par->pages_count * PAGE_SIZE) < vmem_size)
	{
		par->pages_count++;
	}



	par->info->fix.smem_len = par->pages_count * PAGE_SIZE;	
	par->info->fix.smem_start = (unsigned long)vmalloc(par->info->fix.smem_len);
	if(!par->info->fix.smem_start)
	{
		return -ENOMEM;
	}

	memset((void *)par->info->fix.smem_start, 0, par->info->fix.smem_len);

	return 0;
}






static int ili9225_pages_alloc(struct ili9225fb_par *par)
{
	u16	pixels_per_page;
	u16	yoffset_per_page;
	u16	xoffset_per_page;
	u16	index;
	u16	x = 0;
	u16	y = 0;
	u16	*buffer;
	u32	len;


	par->pages = kmalloc(par->pages_count * sizeof(struct ili9225_page), GFP_KERNEL);
	if (!par->pages)
	{
		return -ENOMEM;
	}

	pixels_per_page = PAGE_SIZE / (BPP / 8);
	yoffset_per_page = pixels_per_page / WIDTH;
	xoffset_per_page = pixels_per_page - (yoffset_per_page * WIDTH);

	buffer = (u16 *)par->info->fix.smem_start;

	for (index = 0; index < par->pages_count; index++)
	{
		len = (WIDTH * HEIGHT) - (index * pixels_per_page);
		if(len > pixels_per_page)
		{
			len = pixels_per_page;
		}

		par->pages[index].x = x;
		par->pages[index].y = y;
		par->pages[index].buffer = buffer;
		par->pages[index].len = len;

		x += xoffset_per_page;
		
		if(x >= WIDTH)
		{
			y++;
			x -= WIDTH;
		}
		y += yoffset_per_page;
		buffer += pixels_per_page;
	}

	return 0;

}











static void ili9225_copy(struct ili9225fb_par *par, unsigned int index)
{
	u16	x;
	u16	y;
	u16	*buffer;
	u32	len;

	x = par->pages[index].x;
	y = par->pages[index].y;
	buffer = par->pages[index].buffer;
	len = par->pages[index].len;

	ili9225_set_addr(par, x, y);
	
	ili9225_spi_write_datablock(par, buffer, len); 	 

}






static void ili9225fb_deferred_io(struct fb_info *info,
                                struct list_head *pagelist)
{

	struct ili9225fb_par *par = info->par;
	struct page *page;
	int i;

	list_for_each_entry(page, pagelist, lru)
	{
		par->pages[page->index].must_update = 1;
	}

	for(i = 0; i < par->pages_count; i++)
	{
		if(par->pages[i].must_update)
		{
			par->pages[i].must_update = 0;
			ili9225_copy(par, i);
		}
	}

}



static int ili9225fb_init_display(struct ili9225fb_par *par)
{
        /* TODO: Need some error checking on gpios */

        /* Request GPIOs and initialize to default values */
        gpio_request_one(par->rst, GPIOF_OUT_INIT_HIGH, "ILI9225 Reset Pin");

        gpio_request_one(par->dc, GPIOF_OUT_INIT_LOW, "ILI9225 Data/Command Pin");

        ili9225_reset(par);

        ili9225_run_cfg_script(par);

        return 0;
}

static struct fb_deferred_io ili9225fb_defio = {
        .delay                = HZ / 25,
        .deferred_io        = ili9225fb_deferred_io,
};











static void ili9225_touch(struct fb_info *info, int x, int y, int w, int h)
{
	struct fb_deferred_io *fbdefio = info->fbdefio;
	struct ili9225fb_par *par = info->par;
	int i, ys, ye;


	if(fbdefio)
	{
		for (i = 0; i < par->pages_count; i++)
		{
			ys = par->pages[i].y;
			ye = par->pages[i].y + (par->pages[i].len / (WIDTH * 2)) + 1;
			if(!((y+h) < ys || y > ye))
			{
			 	 par->pages[i].must_update = 1;
			}
		}
		schedule_delayed_work(&info->deferred_work, fbdefio->delay);
	}
}



static inline __u32 CNVT_TOHW(__u32 val, __u32 width)
{
	return ((val<<width) + 0x7FFF - val)>>16;
}

//This routine is needed because the console driver won't work without it.
static int ili9225fb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	int ret = 1;

	/*
	 * If greyscale is true, then we convert the RGB value
	 * to greyscale no matter what visual we are using.
	 */
	if (info->var.grayscale)
		red = green = blue = (19595 * red + 38470 * green +
				      7471 * blue) >> 16;
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;
			u32 value;

			red = CNVT_TOHW(red, info->var.red.length);
			green = CNVT_TOHW(green, info->var.green.length);
			blue = CNVT_TOHW(blue, info->var.blue.length);
			transp = CNVT_TOHW(transp, info->var.transp.length);

			value = (red << info->var.red.offset) |
				(green << info->var.green.offset) |
				(blue << info->var.blue.offset) |
				(transp << info->var.transp.offset);

			pal[regno] = value;
			ret = 0;
		}
		break;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		break;
	}
	return ret;
}





void ili9225fb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
	
        sys_fillrect(info, rect);
	ili9225_touch(info, rect->dx, rect->dy, rect->width, rect->height);

}

void ili9225fb_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{

        sys_copyarea(info, area);
	ili9225_touch(info, area->dx, area->dy, area->width, area->height);

}

void ili9225fb_imageblit(struct fb_info *info, const struct fb_image *image)
{

        sys_imageblit(info, image);
	ili9225_touch(info, image->dx, image->dy, image->width, image->height);

}



/*
int ili9225fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	int retval = -ENOMEM;
	//struct ili9225fb_par *par = info->par;

	switch(cmd)
	{
		default:
			retval = 0;
			break;
	}
	
	return retval;
}
*/






static ssize_t ili9225fb_write(struct fb_info *info, const char __user *buf,
                size_t count, loff_t *ppos)
{
	ssize_t res;

	res = fb_sys_write(info, buf, count, ppos);
	ili9225_touch(info, 0, 0, WIDTH, HEIGHT);
	return res;
}


static struct fb_ops ili9225fb_ops = {
        .owner    	= THIS_MODULE,
        .fb_read        = fb_sys_read,
        .fb_write       = ili9225fb_write,
        .fb_fillrect    = ili9225fb_fillrect,
        .fb_copyarea    = ili9225fb_copyarea,
        .fb_imageblit   = ili9225fb_imageblit,
	.fb_setcolreg	= ili9225fb_setcolreg,
        //.fb_ioctl	= ili9225fb_ioctl,
};

struct fb_info *ili9225fb_extern_info = NULL;
EXPORT_SYMBOL(ili9225fb_extern_info);

void ili9225fb_extern_touch(int x, int y, int w, int h)
{
	ili9225_touch(ili9225fb_extern_info, x, y, w, h);	
}
EXPORT_SYMBOL(ili9225fb_extern_touch);

static int ili9225fb_probe (struct spi_device *spi)
{
//        int chip = spi_get_device_id(spi)->driver_data;
//        struct ili9225fb_platform_data *pdata = spi->dev.platform_data;
        struct fb_info *info;
        struct ili9225fb_par *par;
        int retval = -ENOMEM;

/*	//liu, changed for 3.12
        if (chip != ILI9225_DISPLAY_AF_TFT18) {
                pr_err("%s: only the %s device is supported\n", DRVNAME,
                        to_spi_driver(spi->dev.driver)->id_table->name);
                return -EINVAL;
        }

        if (!pdata) {
                pr_err("%s: platform data required for rst and dc info\n",
                        DRVNAME);
                return -EINVAL;
        }
*/

	//printk("**********liu, ili9225fb_probe - 000\n");
        info = framebuffer_alloc(sizeof(struct ili9225fb_par), &spi->dev);
        if (!info)
                goto fballoc_fail;


        info->fbops = &ili9225fb_ops;

        info->fix = ili9225fb_fix;

        info->var = ili9225fb_var;
        info->var.transp.offset = 0;
        info->var.transp.length = 0;
        
        info->flags = FBINFO_FLAG_DEFAULT | FBINFO_VIRTFB;


	par = info->par;
        par->info = info;
        par->spi = spi;

       //liu, par->rst = pdata->rst_gpio;
       //liu,  par->dc = pdata->dc_gpio;
        par->rst = 8;
        par->dc = 9;
       
       
        par->buf = kmalloc(1, GFP_KERNEL);


	retval = ili9225_video_alloc(par);


	if (retval)
	{
        	pr_err("%s: video_alloc faild\n", DRVNAME);
		goto vialloc_fail;
	}
        info->screen_base =(u8 __iomem*)info->fix.smem_start;

        ili9225fb_extern_info = info;

	retval = ili9225_pages_alloc(par);
	if (retval)
	{
		pr_err("%s: page_alloc faild\n", DRVNAME);
		goto pagealloc_fail;
	}


        retval = register_framebuffer(info);
        if (retval < 0)
                goto fbreg_fail;

        spi_set_drvdata(spi, info);


        retval = ili9225fb_init_display(par);
        if (retval < 0)
                goto init_fail;
	
        info->fbdefio = &ili9225fb_defio;
        fb_deferred_io_init(info);

        printk(KERN_INFO
                "fb%d: %s frame buffer device,\n\tusing %d KiB of video memory\n",
                info->node, info->fix.id, info->fix.smem_len);

        return 0;


        /* TODO: release gpios on fail */
init_fail:
        spi_set_drvdata(spi, NULL);

fbreg_fail:
	kfree(par->pages);
        framebuffer_release(info);

pagealloc_fail:
	vfree((void *)info->fix.smem_start);

vialloc_fail:
	kfree(par->buf);

fballoc_fail:
        return retval;
}

static int ili9225fb_remove(struct spi_device *spi)
{
        struct fb_info *info = spi_get_drvdata(spi);

        spi_set_drvdata(spi, NULL);

        if (info) {
        	struct ili9225fb_par *par = info->par;

                unregister_framebuffer(info);
		vfree((void *)info->fix.smem_start);
                kfree(par->pages);
                kfree(par->buf);
                gpio_free(par->rst);
                gpio_free(par->dc);
               	framebuffer_release(info); 
        }


        return 0;
}





/*	//liu, changed for 3.12
static const struct spi_device_id ili9225fb_ids[] = {
        { "spitft22", ILI9225_DISPLAY_AF_TFT18 },
        { },
};

MODULE_DEVICE_TABLE(spi, ili9225fb_ids);
*/

static const struct of_device_id ili9225_of_match[] = {
	{ .compatible = "ilitek,ili9225", },  
	{},  
};
MODULE_DEVICE_TABLE(of, ili9225_of_match);  







static struct spi_driver ili9225fb_driver = {
        .driver = {
                .name = "ili9225",
                .owner = THIS_MODULE,
		.of_match_table = ili9225_of_match,  
        },
        //.id_table = ili9225fb_ids,		//liu, changed for 3.12
        .probe = ili9225fb_probe,
        .remove = ili9225fb_remove,
};

static int __init ili9225fb_init(void)
{
        return spi_register_driver(&ili9225fb_driver);
}

static void __exit ili9225fb_exit(void)
{
        spi_unregister_driver(&ili9225fb_driver);
}

/* ------------------------------------------------------------------------- */

module_init(ili9225fb_init);
module_exit(ili9225fb_exit);

MODULE_DESCRIPTION("FB driver for ILI9225 display controller");
MODULE_AUTHOR("liu");
MODULE_LICENSE("GPL");
