/********************************************************************
 *This is flash ioctl for  reading ,writing and erasing application
 *******************************************************************/

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
//#include <asm-mips/ioctl.h>
//#include <asm-mips/uaccess.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/spinlock_types.h>
#include <asm/system.h>

#include <atheros.h>
#include "ath_flash.h"
#include <linux/jiffies.h>
#include <linux/timer.h>

/*
 * IOCTL Command Codes
 */
#define ATH_FLASH_READ              0x01
#define ATH_FLASH_WRITE             0x02
#define ATH_FLASH_ERASE             0x03
//#define ATH_FLASH_APP_WRITE           0x04
//#define ATH_FLASH_APP_ERASE           0x05


#define ATH_IO_MAGIC                0xB3
#define ATH_IO_FLASH_READ           _IOR(ATH_IO_MAGIC, ATH_FLASH_READ, char)
#define ATH_IO_FLASH_WRITE          _IOW(ATH_IO_MAGIC, ATH_FLASH_WRITE, char)
#define ATH_IO_FLASH_ERASE          _IO (ATH_IO_MAGIC, ATH_FLASH_ERASE)
//#define ATH_IO_APP_WRITE              _IOW(ATH_IO_MAGIC, ATH_FLASH_APP_WRITE, char)
//#define ATH_IO_APP_ERASE              _IO(ATH_IO_MAGIC, ATH_FLASH_APP_ERASE)
#define ATH_IOC_MAXNR               14
#define flash_major                     239
#define flash_minor                     0

#define TP_LINK_FLASH_SIZE          CONFIG_MTD_ATH_FLASH_SIZE

/**
 * If we need to software control power bit
 * (when upgrade, blink it), we should define
 * this bit here
 */
#if defined(CONFIG_GPIO_POWER_LED_BIT)
#if (CONFIG_GPIO_POWER_LED_BIT >= 0)
#define POWER_LED_GPIO_BIT   (CONFIG_GPIO_POWER_LED_BIT)
#define POWER_LED_GPIO_ON    (CONFIG_GPIO_POWER_LED_ON)
#else
#undef  POWER_LED_GPIO_BIT
#endif
#else
#undef  POWER_LED_GPIO_BIT
#endif

#if defined(CONFIG_GPIO_INTERNET_LED_BIT)
#if (CONFIG_GPIO_INTERNET_LED_BIT >= 0)
#define INET_LED_GPIO               (CONFIG_GPIO_INTERNET_LED_BIT)
#else
#undef INET_LED_GPIO
#endif
#else
#undef  INET_LED_GPIO
#endif

extern void ath_gpio_config_output(int gpio);
extern void ath_gpio_out_val(int gpio, int val);
extern int ath_gpio_get_upgrade_flags(void);
extern void update_wdt(void);
extern void ath_flash_spi_down(void);
extern void ath_flash_spi_up(void);

void ath_spi_sector_erase(uint32_t addr);
int ath_flash_ioctl(struct inode *inode, struct file *file,  unsigned int cmd, unsigned long arg);
int ath_flash_open (struct inode *inode, struct file *file);

struct file_operations flash_device_op = {
        .owner = THIS_MODULE,
        .ioctl = ath_flash_ioctl,
        .open = ath_flash_open,
};

static struct cdev flash_device_cdev = {
        //.kobj   = {.name = "ar7240_flash_chrdev", },
        .owner  = THIS_MODULE,
        .ops = &flash_device_op,
};

typedef struct 
{
    u_int32_t addr;     /* flash r/w addr   */
    u_int32_t len;      /* r/w length       */
    u_int8_t* buf;      /* user-space buffer*/
    u_int32_t buflen;   /* buffer length    */
    u_int32_t reset;    /* reset flag       */
	u_int32_t hasHead;	/* has head or not	*/
}ARG;

//#define ATH_FLASH_IO_BUF_LEN      2048
#define ATH_FLASH_SECTOR_SIZE   (64 * 1024)

struct mtd_info *mtd = NULL;
unsigned char *rwBuf = NULL;

#define TRUE 1
#define FALSE 0
#define FLASH_ERASE_SIZE  ATH_FLASH_SECTOR_SIZE

static void tp_reset_cpu_force()
{   
    printk("file: %s,line: %d==:CPU will be reset now! --debug by HouXB\n",
            __FILE__, __LINE__);
    for(;;)
        {
       //ar7240_reg_wr(AR7240_RESET, AR7240_RESET_FULL_CHIP);
           ath_reg_wr(ATH_RESET_RST, ATH_RESET_FULL_CHIP);
        }
}

static int tp_wdt_reset_setting(int timeout)
{   
    static struct timer_list wdt_timer;
    printk("file: %s,line: %d==: set wdt, CPU will be reset in %d seconds.\n",
            __FILE__, __LINE__, timeout);
    
    init_timer(&wdt_timer);
    wdt_timer.expires = jiffies + timeout*HZ;
    wdt_timer.function = tp_reset_cpu_force;
    wdt_timer.data = 1;

    add_timer(&wdt_timer);
}

static inline void 
ath_flash_sector_erase(uint32_t addr)
{
    ath_flash_spi_down();
    ath_spi_sector_erase(addr);
    ath_flash_spi_up();
}

int nvram_flash_read(struct mtd_info *mtd, u_int8_t *tempData, 
    u_int32_t hasHead, u_int32_t offset, u_int8_t *data, u_int32_t len)
{
    int i = 0;
    int ret = 0;
    int retlen = 0;
	
    int nSector = len / (ATH_FLASH_SECTOR_SIZE); 	/* Divide ATH_FLASH_SECTOR_SIZE */	
    int oddLen  = len % (ATH_FLASH_SECTOR_SIZE);	/* odd length (0 ~ ATH_FLASH_SECTOR_SIZE) */

    /* copy from flash to kernel buffer */  
    for (i = 0; i < nSector; i++)
    {
        ret = ath_flash_read(mtd, offset, ATH_FLASH_SECTOR_SIZE, &retlen, tempData);
        if ((ret == 0) && (retlen == ATH_FLASH_SECTOR_SIZE))
        {
            /*copy from kernel to user*/
            if (copy_to_user(data, tempData, ATH_FLASH_SECTOR_SIZE))
            {
                printk("read copy_to_user failed\n");
                goto wrong;
            }
        }
        else
        {
            printk("Read failed, ret:%d!\n", ret);
            goto wrong;
        }
        offset += ATH_FLASH_SECTOR_SIZE;
        data += ATH_FLASH_SECTOR_SIZE;
    }
    
    if (oddLen) 
    {
        ret = ath_flash_read(mtd, offset, oddLen, &retlen, tempData);
        if ((ret == 0) && (retlen == oddLen))
        {
            /*copy from kernel to user*/
            if (copy_to_user(data, tempData, oddLen))
            {
                printk("read copy_to_user failed\n");
                goto wrong;
            }
        } 
        else 
        {
            printk("Read failed!\n");
            goto wrong;
        }
    }
    return 0;

wrong:
    return -1;
}

int nvram_flash_write(struct mtd_info *mtd, u_int8_t *tempData, 
    u_int32_t hasHead, u_int32_t offset, u_int8_t *data, u_int32_t len)
{
	u_int32_t address = 0;
	u_int32_t headLen = 0;
	u_int32_t endAddr = 0, startAddr = 0;
	u_int8_t *orignData = NULL;
	u_int32_t headData[2] = {len, 0};
	u_int32_t frontLen = 0, tailLen = 0;
	u_int32_t retLen = 0;
#if defined(POWER_LED_GPIO_BIT) || defined(INET_LED_GPIO)
	int gpio_val = 0x1;
#endif
	int gpio_upgradeflag = 0;
	int ret = 0;
	gpio_upgradeflag = ath_gpio_get_upgrade_flags();
	printk("\r\n gpio_upgradeflag:%d \r\n",gpio_upgradeflag);
	headData[0] = htonl(len);	

	if (hasHead != FALSE)
	{
		headLen = 2 * sizeof(u_int32_t);
		len += headLen;
	}

	frontLen = offset % FLASH_ERASE_SIZE;
	tailLen  = (offset + len) % FLASH_ERASE_SIZE;
	/* 第一个block起始地址 */
	address = offset - frontLen;
	/* 最后一个不完整的block起始地址，如果没有，则是再下一个block起始地址 */
	endAddr = offset + len - tailLen;

	orignData = tempData + FLASH_ERASE_SIZE;

	if (frontLen > 0 || headLen > 0)/* 第一个block */
	{
		ret = ath_flash_read(mtd, address, FLASH_ERASE_SIZE, &retLen, orignData);
		if ((ret != 0) || (retLen != FLASH_ERASE_SIZE))
		{
			printk("\Read in write to flash failed status:%d retLen:%d\n", ret, retLen);
			return -1;
		}
		memcpy(tempData, orignData, frontLen);/* 前面一部分为原来的的数据 */
		
		if (FLASH_ERASE_SIZE < frontLen + headLen) /* 头部被拆分到两个blcok */
		{
			headLen = FLASH_ERASE_SIZE - frontLen;
			/* 分区头部，第一部分 */
			memcpy(tempData + frontLen, headData, headLen);

			/***************************************************/
			if (memcmp(orignData, tempData, FLASH_ERASE_SIZE)) /*内容变化*/
			{
				ath_flash_sector_erase(address);
				ret = ath_flash_write(mtd, address, FLASH_ERASE_SIZE, &retLen, tempData);
				if ((ret != 0) && (retLen != FLASH_ERASE_SIZE))
				{
					printk("\nWrite in write to flash failed status:%d retLen:%d\n", ret, retLen);
					return -1;
				}
				
                printk("*");
#if defined(POWER_LED_GPIO_BIT) || defined(INET_LED_GPIO)
                gpio_val = gpio_val ^ 0x1;
#if defined(POWER_LED_GPIO_BIT)
            		ath_gpio_out_val(POWER_LED_GPIO_BIT,gpio_val);
#endif
#if defined(CONFIG_GPIO_940NV6) || defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
			if(gpio_upgradeflag)
				ath_gpio_out_val(INET_LED_GPIO,gpio_val);
#endif
#endif

			}
			address += FLASH_ERASE_SIZE;
			/***************************************************/

			ret = ath_flash_read(mtd, address, FLASH_ERASE_SIZE, &retLen, orignData);
			if ((ret != 0) || (retLen != FLASH_ERASE_SIZE))
			{
				printk("\Read in write to flash failed status:%d retLen:%d\n", ret, retLen);
				return -1;
			}
			
			/* 分区头部，第二部分 */
			memcpy(tempData, (u_int8_t*)(headData) + headLen, 8 - headLen);

			if (len - headLen < FLASH_ERASE_SIZE) /*写入数据长度小于一个block*/
			{
				headLen = 8 - headLen;
				copy_from_user(tempData + headLen, data, tailLen - headLen);/* 需要写入的数据 */
				memcpy(tempData + tailLen, orignData + tailLen, FLASH_ERASE_SIZE - tailLen);/* 原来的数据 */
				data += tailLen - headLen;
			}
			else
			{
				headLen = 8 - headLen;
				copy_from_user(tempData + headLen, data, FLASH_ERASE_SIZE - headLen);/* 需要写入的数据 */
				data += FLASH_ERASE_SIZE - headLen;
			}
		}
		else /* 头部未被拆分 */
		{
			memcpy(tempData + frontLen, headData, headLen);/* 分区头部(如果有的话) */
			
			if (len + frontLen < FLASH_ERASE_SIZE) /*写入数据长度小于一个block*/
			{
				copy_from_user(tempData + frontLen + headLen, data, len - headLen);/* 后面为需要写入的数据 */
				data += len - headLen;
				/* 再后面是原来的数据 */
				memcpy(tempData + frontLen + len,
						orignData + frontLen + len,
						FLASH_ERASE_SIZE - (frontLen + len));
			}
			else
			{
				copy_from_user(tempData + frontLen + headLen, data, FLASH_ERASE_SIZE - frontLen - headLen);
				/* 后面为需要写入的数据 */
				data += FLASH_ERASE_SIZE - frontLen - headLen;
			}
		}

		/***************************************************/
		if (memcmp(orignData, tempData, FLASH_ERASE_SIZE)) /*内容变化*/
		{
		    ath_flash_sector_erase(address);
		    ret = ath_flash_write(mtd, address, FLASH_ERASE_SIZE, &retLen, tempData);
			if ((ret != 0) && (retLen != FLASH_ERASE_SIZE))
			{
				printk("\nWrite in write to flash failed status:%d retLen:%d\n", ret, retLen);
				return -1;
			}
            printk("*");
#if defined(POWER_LED_GPIO_BIT) || defined(INET_LED_GPIO)
			gpio_val = gpio_val ^ 0x1;
#if defined(POWER_LED_GPIO_BIT)
            ath_gpio_out_val(POWER_LED_GPIO_BIT,gpio_val);
#endif
#if defined(CONFIG_GPIO_940NV6) || defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
			if(gpio_upgradeflag)
				ath_gpio_out_val(INET_LED_GPIO,gpio_val);
#endif				
#endif
		}
		address += FLASH_ERASE_SIZE;
		/***************************************************/
	}

	if (address < endAddr)/* 中间完整的block，注意:此处用 < 而不是 <=。 */
	{
		startAddr = address;
		while (address < endAddr)
		{
			ret = ath_flash_read(mtd, address, FLASH_ERASE_SIZE, &retLen, orignData);
			if ((ret != 0) || (retLen != FLASH_ERASE_SIZE))
			{
				printk("\Read in write to flash failed status:%d retLen:%d\n", ret, retLen);
				return -1;
			}
			copy_from_user(tempData, data, FLASH_ERASE_SIZE);
			/***************************************************/
			if (memcmp(orignData, tempData, FLASH_ERASE_SIZE)) /*内容变化*/
			{
				ath_flash_sector_erase(address);
				ret = ath_flash_write(mtd, address, FLASH_ERASE_SIZE, &retLen, tempData);
				if ((ret != 0) && (retLen != FLASH_ERASE_SIZE))
				{
					printk("\nWrite in write to flash failed status:%d retLen:%d\n", ret, retLen);
					return -1;
				}
                printk("*");
#if defined(POWER_LED_GPIO_BIT) || defined(INET_LED_GPIO)
				gpio_val = gpio_val ^ 0x1;
#if defined(POWER_LED_GPIO_BIT)
            		ath_gpio_out_val(POWER_LED_GPIO_BIT,gpio_val);
#endif
#if defined(CONFIG_GPIO_940NV6) || defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
				if(gpio_upgradeflag)
					ath_gpio_out_val(INET_LED_GPIO,gpio_val);
#endif
#endif
			}
			address += FLASH_ERASE_SIZE;
			/***************************************************/
			data += FLASH_ERASE_SIZE;
		}
	}

	if (address < offset + len) /* 如果还没有写完，则说明最后有一个不完整的block */
	{
		ret = ath_flash_read(mtd, address, FLASH_ERASE_SIZE, &retLen, orignData);
		if ((ret != 0) || (retLen != FLASH_ERASE_SIZE))
		{
			printk("\Read in write to flash failed status:%d retLen:%d\n", ret, retLen);
			return -1;
		}
		copy_from_user(tempData, data, tailLen);/*前面一部分为需要写入的数据*/
		memcpy(tempData + tailLen, orignData + tailLen, FLASH_ERASE_SIZE - tailLen);
		/*后面为原来数据*/
		/***************************************************/
		if (memcmp(orignData, tempData, FLASH_ERASE_SIZE)) /*内容变化*/
		{
		    ath_flash_sector_erase(address);
		    ret = ath_flash_write(mtd, address, FLASH_ERASE_SIZE, &retLen, tempData);
			if ((ret != 0) && (retLen != FLASH_ERASE_SIZE))
			{
				printk("\nWrite in write to flash failed status:%d retLen:%d\n", ret, retLen);
				return -1;
			}
            printk("*");
#if defined(POWER_LED_GPIO_BIT) || defined(INET_LED_GPIO)
			gpio_val = gpio_val ^ 0x1;
#if defined(POWER_LED_GPIO_BIT)
            ath_gpio_out_val(POWER_LED_GPIO_BIT,gpio_val);
#endif
#if defined(CONFIG_GPIO_940NV6) || defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
			if(gpio_upgradeflag)
				ath_gpio_out_val(INET_LED_GPIO,gpio_val);
#endif	
#endif
		}
		address += FLASH_ERASE_SIZE;
		/***************************************************/
	}

	return 0;
}


int ath_flash_ioctl(struct inode *inode, struct file *file,  unsigned int cmd, unsigned long arg)
{
    ARG *pArg = (ARG*)arg;
    u_int8_t* usrBuf = pArg->buf;
    u_int32_t usrBufLen = pArg->buflen;
    u_int32_t addr = pArg->addr;
    u_int32_t reset = pArg->reset;
	u_int32_t hasHead = pArg->hasHead;
	
	int ret = 0;
    
    if ((mtd == NULL) || (rwBuf == NULL))
    {
        goto wrong;
    }

    memset(mtd, 0, sizeof(struct mtd_info));

    mtd->size               =   TP_LINK_FLASH_SIZE;
    mtd->erasesize          =   ATH_FLASH_SECTOR_SIZE;
    mtd->numeraseregions    =   0;
    mtd->eraseregions       =   NULL;
    mtd->owner              =   THIS_MODULE;

    if (_IOC_TYPE(cmd) != ATH_IO_MAGIC)
    {
        printk("cmd type error!\n");
        goto wrong;
    }
    if (_IOC_NR(cmd) > ATH_IOC_MAXNR)
    { 
        printk("cmd NR error!\n");
        goto wrong;
    }
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        ret = access_ok(VERIFY_WRITE, (void __user *)usrBuf, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        ret = access_ok(VERIFY_READ, (void __user *)usrBuf, _IOC_SIZE(cmd));
    }
    if (ret < 0)
    { 
        printk("access no ok!\n");
        goto wrong;
    }

    switch(cmd)
    {
        case ATH_IO_FLASH_READ:
        {
            ret = nvram_flash_read(mtd, rwBuf, hasHead, addr, usrBuf, usrBufLen);
            if (ret < 0)
            {
                    goto wrong;
            }

            goto good;
            break;
        }
        

        case ATH_IO_FLASH_WRITE:
        {
            ret = nvram_flash_write(mtd, rwBuf, hasHead, addr, usrBuf, usrBufLen);
            if (ret < 0)
            {
                goto wrong;
            }
            
            if (reset)
            {
                /* now we reboot in user space */
                //machine_restart("reboot");    /*reboot after success */
                
                /* 
                 * reboot from userspace 
                 * but somtimes unlzma kernel thread will occur some errors
                 * so we use this hardware watchdog timer to make sure reboot
                 * will success
                 * this is a workaround, may be should stop unlzma first
                 */
                /* use more stable reset function by yw 15Oct14. */
                /*update_wdt();*/
                printk("reboot in 8s...\n");
                tp_wdt_reset_setting(8);
            }
            
            goto good;
            break;
        }
        
        case  ATH_FLASH_ERASE:
        {
            goto good;
            break;
        }
        
#if 0
        case    ATH_IO_APP_ERASE:
        {
//          printk("app erase ok!\n");
            goto good;
            break;
        }
    
        
        case ATH_IO_APP_WRITE:
        {
            to = 0x40000;

            if (len%mtd->erasesize) nsect++;
            s_curr = ((unsigned int) 0x40000)/mtd->erasesize; 
            s_last = s_curr + nsect;
            //printk("\nNow erase\n ");
            do {
                ath_spi_sector_erase(s_curr*1024*64);
//              printk("The erase addr is %08x\n",s_curr*1024*64);

            }while(++s_curr < s_last);
            ath_spi_done();
            //printk("App Erase succeeded\n");

            for (i = 0; i<len/buflen; i++)
            {
                if (copy_from_user(buffer, argp, buflen)) 
                {
                    printk("write to flash failed when copy_from_user\n");
                    goto wrong;
                 }
//              printk("The app write len is %08x\n", buflen);
                status =  ath_flash_write(mtd, to, buflen, &retlen, buffer);
                if ((status != 0) && (retlen != buflen)) {
                    printk("\nWrite to flash failed status:%d retlen:%d\n", status, retlen);
                    goto wrong;
                }
                    argp += buflen;
                    to += buflen;
            }
        
            if (rwlen)
            {
                if (copy_from_user(buffer, argp, rwlen)) {
                    printk("write to flash failed when copy_from_user\n");
                    goto wrong;
                }
                 printk("The app write len is %08x", rwlen);
                 status =  ath_flash_write(mtd, to, rwlen, &retlen, buffer);
                if ((status != 0) && (retlen != rwlen)) {
                    printk("\nWrite to flash failed. status:%d retlen:%d\n", status, retlen);
                    goto wrong;
                }
            
             }

            printk("\nWrite to flash successful\n");
            machine_restart("reboot"); /*reboot after success */
            goto good;
            break;
        }
    
        default:
        printk("Wrong cmd!\n");
        goto wrong;
#endif  
    }


good:

/*make sure no logic error*/
#if defined(POWER_LED_GPIO_BIT) 
ath_gpio_out_val(POWER_LED_GPIO_BIT,POWER_LED_GPIO_ON);
#endif
    
#if 0   /* change to alloc only once. */
    kfree(mtd);
    kfree(rwBuf);
#endif
    return 0;
wrong:

/*make sure no logic error*/
#if defined(POWER_LED_GPIO_BIT) 
ath_gpio_out_val(POWER_LED_GPIO_BIT,POWER_LED_GPIO_ON);
#endif
    
#if 0   /* change to alloc only once. */
    if (mtd)
    {
        kfree(mtd);
    }

    if (rwBuf)
    {
        kfree(rwBuf);
    }
#endif
    return -1;
    
}


int ath_flash_open (struct inode *inode, struct file *filp)
{
    int minor = iminor(inode);
    int devnum = minor;
    struct mtd_info *mtd;
    /*printk("Now flash open!\n");*/
    
    if ((filp->f_mode & 2) && (minor & 1)) {
        printk("You can't open the RO devices RW!\n");
        return -EACCES;
    }

    mtd = get_mtd_device(NULL, devnum);   
    if (!mtd) {
        printk("Can not open mtd!\n");
        return -ENODEV; 
    }
    filp->private_data = mtd;
    return 0;
    
}

int __init ath_flash_chrdev_init (void)
{
    dev_t dev;
    int result;
    int err;
    int ath_flash_major = flash_major;
    int ath_flash_minor = flash_minor;

    if (ath_flash_major) {
        dev = MKDEV(ath_flash_major, ath_flash_minor);
        result = register_chrdev_region(dev, 1, "ar7240_flash_chrdev");
    } else {
        result = alloc_chrdev_region(&dev, ath_flash_minor, 1, "ar7240_flash_chrdev");
        ath_flash_major = MAJOR(dev);
    }

    if (result < 0) {
        printk(KERN_WARNING "ar7240_flash_chrdev : can`t get major %d\n", ath_flash_major);
        return result;
    }
    cdev_init (&flash_device_cdev, &flash_device_op);
    err = cdev_add(&flash_device_cdev, dev, 1);
    if (err) printk(KERN_NOTICE "Error %d adding flash_chrdev ", err);
//    devfs_mk_cdev(dev, S_IFCHR | S_IRUSR | S_IWUSR, "ar7240flash_chrdev");


    /*
     * alloc buf at init, only need alloc once
     * if in ioctl func, may be failed
     */
    mtd = (struct mtd_info *)kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (mtd)
    {
        rwBuf = (unsigned char *)kmalloc(2*FLASH_ERASE_SIZE, GFP_KERNEL);
        if (NULL == rwBuf)
        {
            kfree(mtd);
            printk("%s kmalloc flash sector buf failed\n", __func__);
        }
    }
    else
    {
        printk("%s kmalloc mtd info buf failed\n", __func__);
    }   

#if defined(POWER_LED_GPIO_BIT)
    ath_gpio_config_output(POWER_LED_GPIO_BIT);
    ath_gpio_out_val(POWER_LED_GPIO_BIT, POWER_LED_GPIO_ON);
#endif

    return 0;

}

static void __exit cleanup_ath_flash_chrdev_exit (void)
{
//  unregister_chrdev_region(MKDEV(flash_major, flash_minor), 1);
    if (mtd)
    {
        kfree(mtd);
    }

    if (rwBuf)
    {
        kfree(rwBuf);
    }
}


module_init(ath_flash_chrdev_init);
module_exit(cleanup_ath_flash_chrdev_exit);
//MODULE_LICENSE("GPL");

