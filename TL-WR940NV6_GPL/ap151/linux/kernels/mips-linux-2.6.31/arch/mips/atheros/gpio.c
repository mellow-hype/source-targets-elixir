#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <linux/config.h>/*  by huangwenzhong, 28May13 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/system.h>
#include <linux/mtd/mtd.h>
#include <linux/cdev.h>
#include <linux/irqreturn.h>

#include <atheros.h>
/*
 * IOCTL Command Codes
 */

#define ATH_GPIO_IOCTL_BASE         (0x01)
#define ATH_GPIO_IOCTL_CMD1         (ATH_GPIO_IOCTL_BASE)
#define ATH_GPIO_IOCTL_CMD2         (ATH_GPIO_IOCTL_BASE + 0x01)
#define ATH_GPIO_IOCTL_CMD3         (ATH_GPIO_IOCTL_BASE + 0x02)
#define ATH_GPIO_IOCTL_CMD4         (ATH_GPIO_IOCTL_BASE + 0x03)
#define ATH_GPIO_IOCTL_CMD5         (ATH_GPIO_IOCTL_BASE + 0x04)
#define ATH_GPIO_IOCTL_CMD6         (ATH_GPIO_IOCTL_BASE + 0x05)
#define ATH_GPIO_IOCTL_CMD7         (ATH_GPIO_IOCTL_BASE + 0x06)
/* for wr941ndv6 wan led ctrl, only in ap151 temporarily, by yw 24Sep14 */
#define ATH_GPIO_IOCTL_CMD8         (ATH_GPIO_IOCTL_BASE + 0x07)
#define ATH_GPIO_IOCTL_CMD9         (ATH_GPIO_IOCTL_BASE + 0x08)
#define ATH_GPIO_IOCTL_CMD10        (ATH_GPIO_IOCTL_BASE + 0x09)
#define ATH_GPIO_IOCTL_CMD11        (ATH_GPIO_IOCTL_BASE + 0x0A)
#define ATH_GPIO_IOCTL_CMD12        (ATH_GPIO_IOCTL_BASE + 0x0B)
#define ATH_GPIO_IOCTL_CMD15		(ATH_GPIO_IOCTL_BASE + 0x0E)
#define ATH_GPIO_IOCTL_CMD16		(ATH_GPIO_IOCTL_BASE + 0x0F)
#define ATH_GPIO_IOCTL_CMD17		 ATH_GPIO_IOCTL_BASE + 0x10
#define ATH_GPIO_IOCTL_MAX          (ATH_GPIO_IOCTL_CMD17)

#define ATH_GPIO_MAGIC              (0xB2)
#define ATH_GPIO_BTN_READ           (_IOR(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD1, int))
#define ATH_GPIO_WIFI_SW_READ       (_IOR(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD2, int))
#define ATH_GPIO_LED_WRITE          (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD3, int))
#define ATH_GPIO_USB_LED1_WRITE     (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD4, int))
#define ATH_GPIO_INET_LED_WRITE     (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD5, int))
#define ATH_GPIO_WIFI_BTN_READ      (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD6, int))
#define ATH_GPIO_BTN_PUSH_CNT_READ  (_IOR(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD7, int))
/* for wr941ndv6 wan led ctrl, only in ap151 temporarily, by yw 24Sep14 */
#define ATH_GPIO_WAN_LED_WRITE      (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD8, int))
#define ATH_GPIO_WIFI_LED_WRITE     (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD9, int))
#define ATH_GPIO_POWER_LED_WRITE    (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD10, int))
#define ATH_GPIO_LAN_LED_WRITE    (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD12, int))
#define ATH_GPIO_RE_BTN_READ		(_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD15, int))
#define ATH_GPIO_RESET_BTN_STATUS_READ	(_IOR(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD16, int))
#define ATH_GPIO_UPGRADE_FLAG_WRITE  (_IOW(ATH_GPIO_MAGIC, ATH_GPIO_IOCTL_CMD17, int))
/* for GPIO device number */
#define gpio_major                  (238)
#define gpio_minor                  (0)


/*
 * GPIO assignment
 */

#define ATH_GPIO_MIN                (0)

/* reset default button, default is GPIO12 */
#define RST_DFT_GPIO                (CONFIG_GPIO_RESET_FAC_BIT)
/* How long the user must press the button before Router rst, default is 5 */
#define RST_HOLD_TIME               (CONFIG_GPIO_FAC_RST_HOLD_TIME)

#if (CONFIG_GPIO_READY_STATUS_BIT >= ATH_GPIO_MIN)
/* system LED, default is GPIO13    */
#define SYS_LED_GPIO                (CONFIG_GPIO_READY_STATUS_BIT)
/* system LED's value when off, default is 1 */
#define SYS_LED_OFF                 (!CONFIG_GPIO_READY_STATUS_ON)
#define SYS_LED_ON                  (CONFIG_GPIO_READY_STATUS_ON)
#else
#undef SYS_LED_GPIO
#endif

/* QSS LED, default is GPIO3 */
#define TRICOLOR_LED_GREEN_PIN      (CONFIG_GPIO_JUMPSTART_LED_BIT)
/* jump start LED's value when off */
#define JUMPSTART_LED_OFF           (!CONFIG_GPIO_JUMPSTART_LED_ON)
/* jump start LED'S value when on, default is 0 */
#define JUMPSTART_LED_ON            (CONFIG_GPIO_JUMPSTART_LED_ON)

/* CN model may need not wifi switch */
#if (CONFIG_GPIO_WIFI_SWITCH_BIT >= ATH_GPIO_MIN)
/* WiFi switch, default is GPIO17 */
#define WIFI_RADIO_SW_GPIO          (CONFIG_GPIO_WIFI_SWITCH_BIT)
#define WIFI_BUTTON_HOLD_TIME       (CONFIG_GPIO_WIFI_BUTTON_HOLD_TIME)
#else
#undef WIFI_RADIO_SW_GPIO
#endif

#if (CONFIG_GPIO_WORKMODE_SWITCH_1ST_BIT >= ATH_GPIO_MIN)
/* Work Mode switch for 803, GPIO 14 and 16 */
#define WORKMODE_SWITCH_1ST_GPIO    (CONFIG_GPIO_WORKMODE_SWITCH_1ST_BIT)
#define WORKMODE_SWITCH_2ND_GPIO    (CONFIG_GPIO_WORKMODE_SWITCH_2ND_BIT)
#else
#undef WORKMODE_SWITCH_1ST_GPIO
#endif

#if (CONFIG_GPIO_INTERNET_LED_BIT >= ATH_GPIO_MIN)
/* Internet LED for 803, default is GPIO11 */
#define INET_LED_GPIO               (CONFIG_GPIO_INTERNET_LED_BIT)
/* Internet LED's vaule when on, default is 0 */
#define INET_LED_ON                 (CONFIG_GPIO_INTERNET_LED_ON)
/* Internet LED's vaule when off, default is 1 */
#define INET_LED_OFF                (!CONFIG_GPIO_INTERNET_LED_ON)
#else
#undef INET_LED_GPIO
#endif

#if (CONFIG_GPIO_WAN_LED_BIT >= ATH_GPIO_MIN)
/* WAN LED for 941V6, default is GPIO15 */
#define WAN_LED_GPIO               (CONFIG_GPIO_WAN_LED_BIT)
/* WAN LED's vaule when on, default is 0 */
#define WAN_LED_ON                 (CONFIG_GPIO_WAN_LED_ON)
/* WAN LED's vaule when off, default is 1 */
#define WAN_LED_OFF                (!CONFIG_GPIO_WAN_LED_ON)
#else
#undef  WAN_LED_GPIO
#endif

#if (CONFIG_GPIO_LAN_LED_BIT >= ATH_GPIO_MIN)
#define LAN_LED_GPIO               (CONFIG_GPIO_LAN_LED_BIT)
#define LAN_LED_ON                 (CONFIG_GPIO_LAN_LED_ON)
#define LAN_LED_OFF                (!CONFIG_GPIO_LAN_LED_ON)
#else
#undef  LAN_LED_GPIO
#endif

#if (CONFIG_GPIO_WIFI_LED_BIT >= ATH_GPIO_MIN)
/* WIFI LED for 941V6, default is GPIO8 */
#define WIFI_LED_GPIO               (CONFIG_GPIO_WIFI_LED_BIT)
/* WIFI LED's vaule when on, default is 0 */
#define WIFI_LED_ON                 (CONFIG_GPIO_WIFI_LED_ON)
/* WIFI LED's vaule when off, default is 1 */
#define WIFI_LED_OFF                (!CONFIG_GPIO_WIFI_LED_ON)
#else
#undef  WIFI_LED_GPIO
#endif

#if (CONFIG_GPIO_USB_LED_BIT >= ATH_GPIO_MIN)
/* USB LED, default is GPIO11 */
#define AP_USB_LED_GPIO             (CONFIG_GPIO_USB_LED_BIT)
/* USB LED's value when off */
#define USB_LED_OFF                 (!CONFIG_GPIO_USB_LED_ON)
/* USB LED's value when on, default is 0 */
#define USB_LED_ON                  (CONFIG_GPIO_USB_LED_ON)
/* usb power switch, default is 4 */
#define USB_POWER_SW_GPIO           (CONFIG_GPIO_USB_SWITCHFOR3G_BIT)
#define USB_POWER_ON                (1)
#define USB_POWER_OFF               (!USB_POWER_ON)
#else
#undef AP_USB_LED_GPIO
#endif

#if (CONFIG_GPIO_RE_BTN_BIT >= ATH_GPIO_MIN)
#define RE_BTN_GPIO               (CONFIG_GPIO_RE_BTN_BIT)
#define RE_BTN_HOLD_TIME       (CONFIG_GPIO_RE_BTN_HOLD_TIME)
#define RE_WPS_LED_GPIO      (CONFIG_GPIO_RE_LED_BIT)
#define RE_WPS_LED_OFF           (!CONFIG_GPIO_RE_LED_ON)
#define RE_WPS_LED_ON            (CONFIG_GPIO_RE_LED_ON)
#else
#undef  RE_BTN_GPIO
#endif

#if (CONFIG_GPIO_JUMPSTART_SW_BIT >= ATH_GPIO_MIN)
#define WPS_BTN_GPIO               (CONFIG_GPIO_JUMPSTART_SW_BIT)
#else
#undef  WPS_BTN_GPIO
#endif

static int counter = 0;
static int bRstBtnDown = 0;
static int bUpgradeFlags = 0;
static int bBlockWps = 0;
static int btnPushCnt = 0;
static struct timer_list rst_timer;

#ifdef WIFI_RADIO_SW_GPIO
/* local var for wifi-switch */
static struct timer_list wifi_button_timer;
static int ignore_wifibutton = 1;
static int wifi_button_flag = 0;
/* some models use wifi button for wps */
static int l_bMultiUseWifiButton        =   0;

#endif
#ifdef RE_BTN_GPIO
static struct timer_list re_button_timer;
static int ignore_rebutton = 1;
static int re_button_flag = 0;
#endif

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

/* control params for reset button reuse, by zjg, 13Apr10 */
static int l_bMultiUseResetButton       =   0;
static int l_bWaitForQss                =   1;


#ifdef AP_USB_LED_GPIO
int g_usbLedBlinkCountDown = 1;
EXPORT_SYMBOL(g_usbLedBlinkCountDown);
#endif

#ifdef CONFIG_GPIO_OPTION66
/* used to calc how many unit of RST_HOLD_TIME rst button is hold */
static int rstPressTimeCounter = 0;
static struct timer_list rst_timer_opt66;
#define OPTION66_RST_HOLD_TIME 30
#ifdef CONFIG_GPIO_940NV6
#define WAN_GPIO_INDEX 1
static int GPIO_LIST_940[] = {INET_LED_GPIO, WAN_LED_GPIO, TRICOLOR_LED_GREEN_PIN};
#else
#define LAN_PORT_1_GPIO_940v5 8
#define LAN_PORT_2_GPIO_940v5 6
#define LAN_PORT_3_GPIO_940v5 18
#define LAN_PORT_4_GPIO_940v5 4
#define WAN_GPIO_INDEX 3

static int GPIO_LIST_940[] = {POWER_LED_GPIO_BIT, WIFI_LED_GPIO, INET_LED_GPIO, WAN_LED_GPIO, \
	TRICOLOR_LED_GREEN_PIN, LAN_PORT_1_GPIO_940v5, LAN_PORT_2_GPIO_940v5, LAN_PORT_3_GPIO_940v5, \
	LAN_PORT_4_GPIO_940v5};
#endif
#endif


/*
 * GPIO interrupt stuff
 */
typedef enum 
{
    INT_TYPE_EDGE,
    INT_TYPE_LEVEL,
}ath_gpio_int_type_t;

typedef enum 
{
    INT_POL_ACTIVE_LOW,
    INT_POL_ACTIVE_HIGH,
}ath_gpio_int_pol_t;

/*  by huangwenzhong, 11Oct13 */
typedef enum
{
    ATH_GPIO_LEVEL_LOW = 0,
    ATH_GPIO_LEVEL_HIGH,
}ath_gpio_level;


/*changed by hujia.*/
typedef irqreturn_t(*sc_callback_t)(int, void *, void *, void *);
static sc_callback_t registered_cb = NULL;
static char* cb_name = NULL;
static void *cb_arg = NULL;
/*add by hujia.*/
static void *cb_pushtime = NULL;
/*end add.*/

/* top dir entry */
static struct proc_dir_entry *simple_config_entry = NULL;
static struct proc_dir_entry *simulate_push_button_entry = NULL;
static struct proc_dir_entry *tricolor_led_entry = NULL;
#ifdef RE_BTN_GPIO
static struct proc_dir_entry *re_led_entry = NULL;
#endif

/* added by zjg, 12Apr10 */
static struct proc_dir_entry *multi_use_reset_button_entry = NULL;

#ifdef WIFI_RADIO_SW_GPIO
static struct proc_dir_entry *multi_use_wifi_button_entry = NULL;
static struct proc_dir_entry *wifi_button_entry = NULL;
#endif

#ifdef AP_USB_LED_GPIO
/* ZJin 100317: for 3g usb led blink feature, use procfs simple config. */
static struct proc_dir_entry *usb_led_blink_entry = NULL;

/*added by  ZQQ<10.06.02 for usb power*/
static struct proc_dir_entry *usb_power_entry = NULL;
#endif

#ifdef WORKMODE_SWITCH_1ST_GPIO
static struct proc_dir_entry *workmode_entry = NULL;
#endif

#ifndef SYS_LED_GPIO
static struct proc_dir_entry *blockWps_entry = NULL;
#endif

/*  by huangwenzhong, 06Mar13 */
/* 根据GPIO的编号获得对应的FUNCTION寄存器地址 */
static u32 ath_get_gpio_function(int gpio)
{
    int i = (gpio / 4);
    u32 functionx = -1;
    switch (i)
    {
        case 0:
            functionx = ATH_GPIO_OUT_FUNCTION0;
            break;
        case 1:
            functionx = ATH_GPIO_OUT_FUNCTION1;
            break;
        case 2:
            functionx = ATH_GPIO_OUT_FUNCTION2;
            break;
        case 3:
            functionx = ATH_GPIO_OUT_FUNCTION3;
            break;
        case 4:
            functionx = ATH_GPIO_OUT_FUNCTION4;
            break;
        case 5:
            functionx = ATH_GPIO_OUT_FUNCTION5;
            break;
        default:
            functionx = -1;
    }
    return functionx;
}
/* 根据GPIO编号获取对应的8位设置位在寄存器中的偏移量 */
#define ATH_GET_GPIO_SHIFT_BIT(x)           (((x) % 4) * 8)



void ath_gpio_config_int(int gpio,
             ath_gpio_int_type_t type,
             ath_gpio_int_pol_t polarity)
{
    u32 val;
    //printk("gpio = %d, type = %d, polarity = %d\n", gpio, type, polarity);
    /*
     * allow edge sensitive/rising edge too
     */
    if (type == INT_TYPE_LEVEL) {
        /* level sensitive */
        ath_reg_rmw_set(ATH_GPIO_INT_TYPE, (1 << gpio));
    } else {
        /* edge triggered */
        val = ath_reg_rd(ATH_GPIO_INT_TYPE);
        val &= ~(1 << gpio);
        ath_reg_wr(ATH_GPIO_INT_TYPE, val);
    }

    if (polarity == INT_POL_ACTIVE_HIGH) {
        ath_reg_rmw_set(ATH_GPIO_INT_POLARITY, (1 << gpio));
    } else {
        val = ath_reg_rd(ATH_GPIO_INT_POLARITY);
        val &= ~(1 << gpio);
        ath_reg_wr(ATH_GPIO_INT_POLARITY, val);
    }

    ath_reg_rmw_set(ATH_GPIO_INT_ENABLE, (1 << gpio));
}

void ath_gpio_config_output(int gpio)
{
#if defined(CONFIG_MACH_AR934x) || \
    defined(CONFIG_MACH_QCA955x) || \
    defined(CONFIG_MACH_QCA953x) || \
    defined(CONFIG_MACH_QCA956x)
    ath_reg_rmw_clear(ATH_GPIO_OE, (1 << gpio));
#else
    ath_reg_rmw_set(ATH_GPIO_OE, (1 << gpio));
#endif
}
EXPORT_SYMBOL(ath_gpio_config_output);

void ath_gpio_config_input(int gpio)
{
#if defined(CONFIG_MACH_AR934x) || \
    defined(CONFIG_MACH_QCA955x) || \
    defined(CONFIG_MACH_QCA953x) || \
    defined(CONFIG_MACH_QCA956x)
    ath_reg_rmw_set(ATH_GPIO_OE, (1 << gpio));
#else
    ath_reg_rmw_clear(ATH_GPIO_OE, (1 << gpio));
#endif
}
EXPORT_SYMBOL(ath_gpio_config_input);

void ath_gpio_out_val(int gpio, int val)
{
    if (val & 0x1) {
        ath_reg_rmw_set(ATH_GPIO_OUT, (1 << gpio));
    } else {
        ath_reg_rmw_clear(ATH_GPIO_OUT, (1 << gpio));
    }
}
EXPORT_SYMBOL(ath_gpio_out_val);

int ath_gpio_get_upgrade_flags(void)
{
	return bUpgradeFlags;
}
EXPORT_SYMBOL(ath_gpio_get_upgrade_flags);

int ath_gpio_in_val(int gpio)
{
    return ((1 << gpio) & (ath_reg_rd(ATH_GPIO_IN)));
}

static void
ath_gpio_intr_enable(unsigned int irq)
{
    ath_reg_rmw_set(ATH_GPIO_INT_MASK,
                (1 << (irq - ATH_GPIO_IRQ_BASE)));
}

static void
ath_gpio_intr_disable(unsigned int irq)
{
    ath_reg_rmw_clear(ATH_GPIO_INT_MASK,
                (1 << (irq - ATH_GPIO_IRQ_BASE)));
}

static unsigned int
ath_gpio_intr_startup(unsigned int irq)
{
    ath_gpio_intr_enable(irq);
    return 0;
}

static void
ath_gpio_intr_shutdown(unsigned int irq)
{
    ath_gpio_intr_disable(irq);
}

static void
ath_gpio_intr_ack(unsigned int irq)
{
    ath_gpio_intr_disable(irq);
}

static void
ath_gpio_intr_end(unsigned int irq)
{
    if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
        ath_gpio_intr_enable(irq);
}

static int
ath_gpio_intr_set_affinity(unsigned int irq, const struct cpumask *dest)
{
    /*
     * Only 1 CPU; ignore affinity request
     */
    return 0;
}

struct irq_chip /* hw_interrupt_type */ ath_gpio_intr_controller = {
    .name = "ATH GPIO",
    .startup = ath_gpio_intr_startup,
    .shutdown = ath_gpio_intr_shutdown,
    .enable = ath_gpio_intr_enable,
    .disable = ath_gpio_intr_disable,
    .ack = ath_gpio_intr_ack,
    .end = ath_gpio_intr_end,
    .eoi = ath_gpio_intr_end,
    .set_affinity = ath_gpio_intr_set_affinity,
};

void ath_gpio_irq_init(int irq_base)
{
    int i;

    for (i = irq_base; i < irq_base + ATH_GPIO_IRQ_COUNT; i++) {
        irq_desc[i].status = IRQ_DISABLED;
        irq_desc[i].action = NULL;
        irq_desc[i].depth = 1;
        //irq_desc[i].chip = &ath_gpio_intr_controller;
        set_irq_chip_and_handler(i, &ath_gpio_intr_controller,
                     handle_percpu_irq);
    }
}

void register_simple_config_callback(char *cbname, void *callback, void *arg1, void *arg2)
{
    cb_name = cbname;
    registered_cb = (sc_callback_t) callback;
    cb_arg = arg1;
    /* add by hujia.*/
    cb_pushtime=arg2;
    /*end add.*/
}
EXPORT_SYMBOL(register_simple_config_callback);

void unregister_simple_config_callback (void)
{
    registered_cb = NULL;
    cb_arg = NULL;
    cb_pushtime = NULL;
    cb_name = NULL;
}
EXPORT_SYMBOL(unregister_simple_config_callback);


/******* begin ioctl stuff **********/
#undef CONFIG_GPIO_DEBUG
#ifdef CONFIG_GPIO_DEBUG
void print_gpio_regs(char* prefix)
{
    printk("\n-------------------------%s---------------------------\n", prefix);
    printk("ATH_GPIO_OE:%#X\n", ath_reg_rd(ATH_GPIO_OE));
    printk("ATH_GPIO_IN:%#X\n", ath_reg_rd(ATH_GPIO_IN));
    printk("ATH_GPIO_OUT:%#X\n", ath_reg_rd(ATH_GPIO_OUT));
    //printk("ATH_GPIO_SET:%#X\n", ath_reg_rd(ATH_GPIO_SET));
    //printk("ATH_GPIO_CLEAR:%#X\n", ath_reg_rd(ATH_GPIO_CLEAR));
    printk("ATH_GPIO_INT_ENABLE:%#X\n", ath_reg_rd(ATH_GPIO_INT_ENABLE));
    printk("ATH_GPIO_INT_TYPE:%#X\n", ath_reg_rd(ATH_GPIO_INT_TYPE));
    printk("ATH_GPIO_INT_POLARITY:%#X\n", ath_reg_rd(ATH_GPIO_INT_POLARITY));
    printk("ATH_GPIO_INT_PENDING:%#X\n", ath_reg_rd(ATH_GPIO_INT_PENDING));
    printk("ATH_GPIO_INT_MASK:%#X\n", ath_reg_rd(ATH_GPIO_INT_MASK));
    printk("\n-------------------------------------------------------\n");
    }
#endif



/*
 *  USB GPIO control
 */

void ap_usb_led_on(void)
{
#ifdef AP_USB_LED_GPIO
    if (AP_USB_LED_GPIO >= ATH_GPIO_MIN)
    {
        ath_gpio_out_val(AP_USB_LED_GPIO, USB_LED_ON);
    }
#endif
}

EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
#ifdef AP_USB_LED_GPIO
    if (AP_USB_LED_GPIO >= ATH_GPIO_MIN)
    {
        ath_gpio_out_val(AP_USB_LED_GPIO, USB_LED_OFF);
    }
#endif
}

EXPORT_SYMBOL(ap_usb_led_off);


/* for reset button push by user, >=5s */
static int ignore_rstbutton = 1;

#ifdef CONFIG_GPIO_OPTION66	
void check_rst_blink()
{
	static int val = 0x1;
	int gpioIndex = 0;
	val = val ^ 0x1;

	if ((rstPressTimeCounter >= 1) && (rstPressTimeCounter < OPTION66_RST_HOLD_TIME / RST_HOLD_TIME))
	{
		/*all led blink quickly in 5s~30s*/
		ath_gpio_out_val(GPIO_LIST_940[WAN_GPIO_INDEX], 0);
		for (gpioIndex = 0; gpioIndex < sizeof(GPIO_LIST_940)/sizeof(int); gpioIndex++)
		{
			if (WAN_GPIO_INDEX == gpioIndex)
				continue;			
			ath_gpio_out_val(GPIO_LIST_940[gpioIndex], val);
		}
		mod_timer(&rst_timer_opt66, jiffies + HZ / 5);
	}
	else if (rstPressTimeCounter >= OPTION66_RST_HOLD_TIME / RST_HOLD_TIME)
	{
		/*all led off*/
		printk("tp-link or ISP factory! all led off!\n");
		for (gpioIndex = 0; gpioIndex < sizeof(GPIO_LIST_940)/sizeof(int); gpioIndex++)
		{
			ath_gpio_out_val(GPIO_LIST_940[gpioIndex], 1);
		}
		ath_gpio_out_val(GPIO_LIST_940[WAN_GPIO_INDEX], 0);
	}		
		
}
#endif


/* irq handler for reset button */
irqreturn_t rst_irq(int cpl, void *dev_id)
{
    //printk("ignore_rstbutton = %d\n", ignore_rstbutton);
    //print_gpio_regs(irq_count++);
    if (ignore_rstbutton &&
        (ath_gpio_in_val(RST_DFT_GPIO) >> RST_DFT_GPIO) == ATH_GPIO_LEVEL_LOW)
    {
    		bRstBtnDown = 1;
        printk("Reset button down.\n");
        ath_gpio_config_int(RST_DFT_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_HIGH);
        ignore_rstbutton = 0;
        
        mod_timer(&rst_timer, jiffies + RST_HOLD_TIME * HZ);

        return IRQ_HANDLED;
    }

    ignore_rstbutton = 1;
    btnPushCnt++;
    ath_gpio_config_int (RST_DFT_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

	bRstBtnDown = 0;
    printk("Reset button up.\n");
#ifdef CONFIG_GPIO_OPTION66
	/* button is released here.
	 * if rst button is hold for more than 5s but less than 30s and released,
	 * reset to ISP config */
	//if ((rstPressTimeCounter >= 1) && (rstPressTimeCounter < OPTION66_RST_HOLD_TIME / RST_HOLD_TIME))
	if (rstPressTimeCounter >= 1) 
	{
		counter = rstPressTimeCounter;
		rstPressTimeCounter = 0;
		printk("reset to ISP config.\n");
	}
#endif



    /* mark reset status, by zjg, 12Apr10 */
    if ((registered_cb != NULL) && (!bBlockWps) && (l_bMultiUseResetButton) && (l_bWaitForQss)) 
    {
        /*return registered_cb (cpl, cb_arg, regs);*/
        return registered_cb(cpl, cb_arg, NULL, cb_pushtime);
    }

    return IRQ_HANDLED;
}


void check_rst(unsigned long nothing)
{
    if (!ignore_rstbutton)
    {
        printk ("restoring factory default...\n");
        /* check the level first, make sure it is still low level */
        if ((ath_gpio_in_val(RST_DFT_GPIO) >> RST_DFT_GPIO) == ATH_GPIO_LEVEL_LOW)
        {
#ifdef CONFIG_GPIO_OPTION66	
			rstPressTimeCounter ++;
			/* 
			 * under CONFIG_GPIO_OPTION66, if pressed for more than 30s,
			 * execute factory reset; if press for only more than 5s,
			 * execute ISP reset;
			 */
			int gpioIndex = 0;
			if (1 == rstPressTimeCounter)
			{
				for (gpioIndex = 0; gpioIndex < sizeof(GPIO_LIST_940)/sizeof(int); gpioIndex++)
				{
					ath_reg_rmw_clear(ath_get_gpio_function(GPIO_LIST_940[gpioIndex]),
									  0xff << ATH_GET_GPIO_SHIFT_BIT(GPIO_LIST_940[gpioIndex]));
					ath_gpio_config_output(GPIO_LIST_940[gpioIndex]);
				}
				check_rst_blink();
			}
			if ( rstPressTimeCounter < OPTION66_RST_HOLD_TIME / RST_HOLD_TIME)
			{
				/* reset timer */
				printk("press rst button for %d seconds\n", ( rstPressTimeCounter ) * RST_HOLD_TIME);
				mod_timer(&rst_timer, jiffies + RST_HOLD_TIME * HZ);
			}
			else
			{
				counter = rstPressTimeCounter;
				printk ("restoring tp-link factory default...\n");
			}
#else			
            //printk("check_rst low.\r\n");
            counter ++;
#endif
        }

        /* to mark reset status, forbid QSS, added by zjg, 12Apr10 */
        if (l_bMultiUseResetButton)
        {
            l_bWaitForQss   = 0;
        }
    }
}

static int multi_use_reset_button_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", l_bMultiUseResetButton);
}

static int multi_use_reset_button_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    /* only admit "0" or "1" */
    if ((val < 0) || (val > 1))
        return -EINVAL; 

    l_bMultiUseResetButton = val;
    
    return count;

}

#ifdef WIFI_RADIO_SW_GPIO
/* irq handler for wifi switch */
static irqreturn_t wifi_sw_irq(int cpl, void *dev_id)
{
    if (ignore_wifibutton &&
        (ath_gpio_in_val(WIFI_RADIO_SW_GPIO) >> WIFI_RADIO_SW_GPIO) == ATH_GPIO_LEVEL_LOW)
    {
        printk("WIFI button down.\n");
        ath_gpio_config_int (WIFI_RADIO_SW_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_HIGH);
        ignore_wifibutton = 0;

        mod_timer(&wifi_button_timer, jiffies + WIFI_BUTTON_HOLD_TIME * HZ);    /* 2sec */

        if (l_bMultiUseWifiButton)
        {
            l_bWaitForQss = 1;
        }
        
        return IRQ_HANDLED;
    }

    ignore_wifibutton = 1;
    ath_gpio_config_int (WIFI_RADIO_SW_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    printk("WIFI button up.\n");

    if ((registered_cb != NULL) && (!bBlockWps) && (l_bMultiUseWifiButton) && (l_bWaitForQss)) 
    {
        return registered_cb(cpl, cb_arg, NULL, cb_pushtime);
    }

    return IRQ_HANDLED;
}

static void wifi_sw_check(unsigned long nothing)
{
    /* if user keep push button more than 2s, 
    ignore_wifibutton will keep 0, 
    or ignore_wifibutton will be 1 */
    if (!ignore_wifibutton)
    {
        printk ("Switch Wi-Fi...\n");

        /* check the level first, make sure it is still low level */
        if ((ath_gpio_in_val(WIFI_RADIO_SW_GPIO) >> WIFI_RADIO_SW_GPIO) == ATH_GPIO_LEVEL_LOW)
        {
            wifi_button_flag ++;
             printk("Switch Wi-Fi  wifi_sw_check check low\n");
        }
        
        if (l_bMultiUseWifiButton)
        {
            l_bWaitForQss = 0;
        }
    }
}

static int multi_use_wifi_button_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", l_bMultiUseWifiButton);
}

static int multi_use_wifi_button_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    /* only admit "0" or "1" */
    if ((val < 0) || (val > 1))
        return -EINVAL; 

    l_bMultiUseWifiButton = val;
    
    return count;

}

static int wifi_button_proc_read(char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    u_int32_t ret = 0;

    u_int32_t reg_value1 = 0;

    reg_value1 = ath_gpio_in_val(WIFI_RADIO_SW_GPIO);

    reg_value1 = reg_value1 >> WIFI_RADIO_SW_GPIO;

    ret = reg_value1;
    
    return sprintf (page, "%d\n", ret);
}

static int wifi_button_proc_write(struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if ((val < 0) || (val > 1))
        return -EINVAL;

    printk("%s %d: write value = %d\r\n",__FUNCTION__,__LINE__,val);
    
    return count;
}
#endif
#ifdef WPS_BTN_GPIO
irqreturn_t wps_btn_irq(int cpl, void *dev_id)

{
	printk("wps button down.\n");
    if ((registered_cb != NULL) && (!bBlockWps) && (l_bWaitForQss)) 
    {
        /*return registered_cb (cpl, cb_arg, regs);*/
        return registered_cb(cpl, cb_arg, NULL, cb_pushtime);
    }

    return IRQ_HANDLED;
}
#endif
#ifdef RE_BTN_GPIO
/* irq handler for re button */
irqreturn_t re_btn_irq(int cpl, void *dev_id)

{
    //printk("ignore_rstbutton = %d\n", ignore_rstbutton);
    //print_gpio_regs(irq_count++);
    if (ignore_rebutton &&
        (ath_gpio_in_val(RE_BTN_GPIO) >> RE_BTN_GPIO) == ATH_GPIO_LEVEL_LOW)
    {
        printk("Re button down.\n");
        ath_gpio_config_int(RE_BTN_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_HIGH);
        ignore_rebutton = 0;
        
        mod_timer(&re_button_timer, jiffies + RE_BTN_HOLD_TIME * HZ);

        return IRQ_HANDLED;
    }

    ignore_rebutton = 1;
    ath_gpio_config_int (RE_BTN_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    printk("Re button up.\n");

    return IRQ_HANDLED;
}
static void re_check(unsigned long nothing)
{
    /* if user keep push button more than 2s, 
    ignore_wifibutton will keep 0, 
    or ignore_wifibutton will be 1 */
    if (!ignore_rebutton)
    {
        printk ("RE processing...\n");

        /* check the level first, make sure it is still low level */
        if ((ath_gpio_in_val(RE_BTN_GPIO) >> RE_BTN_GPIO) == ATH_GPIO_LEVEL_LOW)
        {
            re_button_flag ++;
             printk("re check low\n");
        }
    }
}
#endif

static int push_button_read(char *page, char **start, off_t off,
                int count, int *eof, void *data)
{
    return 0;
}

static int push_button_write(struct file *file, const char *buf,
                unsigned long count, void *data)
{
    if (registered_cb) {
        /*changed by hujia.*/
        /* registered_cb (0, cb_arg, 0);*/
        registered_cb (0, cb_arg, NULL, NULL);
    }
    return count;
}

/* for USB */
static int usb_led_blink_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
#ifdef AP_USB_LED_GPIO
    return sprintf (page, "%d\n", g_usbLedBlinkCountDown);
#else
    return sprintf (page, "%d\n", 0);
#endif
}

static int usb_led_blink_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
#ifdef AP_USB_LED_GPIO
    u_int32_t val;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    /* modified by hejian for 3G led logic, by 120607 */
    /* added 2 cases, case 2 means turn led off, case 3 mean turn led on */
    if ((val < 0) || (val > 3))
        return -EINVAL;

    if (val == 2)
    {
        ap_usb_led_off();
    }
    else if (val == 3)
    {
        ap_usb_led_on();
    }
    else
    {
        g_usbLedBlinkCountDown = val;
    }
#endif
    return count;
}

/* For 3G switch */
/*added by ZQQ,10.06.02*/
static int usb_power_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    return 0;
}

static int usb_power_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
#ifdef AP_USB_LED_GPIO
    u_int32_t val = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if ((val < 0) || (val > 1))
        return -EINVAL;

    printk("%s %d: write gpio:value = %d\r\n",__FUNCTION__,__LINE__,val);
    
    if (USB_POWER_ON == val)
    {
        ath_gpio_out_val(USB_POWER_SW_GPIO, USB_POWER_ON);
    }
    else
    {
        ath_gpio_out_val(USB_POWER_SW_GPIO, USB_POWER_OFF);
    }
#endif  
    return count;
}


#ifdef WORKMODE_SWITCH_1ST_GPIO
static int workmode_proc_read(char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    u_int32_t ret = 0;
    
    u_int32_t reg_value1 = 0;
    u_int32_t reg_value2 = 0;

    reg_value1 = ath_gpio_in_val(WORKMODE_SWITCH_1ST_GPIO);
    reg_value2 = ath_gpio_in_val(WORKMODE_SWITCH_2ND_GPIO);

    reg_value1 = reg_value1 >> WORKMODE_SWITCH_1ST_GPIO;
    reg_value2 = reg_value2 >> WORKMODE_SWITCH_2ND_GPIO;

    ret = (reg_value2 << 1) | reg_value1;
    
    return sprintf (page, "%d\n", ret);
}

static int workmode_proc_write(struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if ((val < 0) || (val > 1))
        return -EINVAL;

    printk("%s %d: write value = %d\r\n",__FUNCTION__,__LINE__,val);
    
    return count;
}
#endif

#ifndef SYS_LED_GPIO
static int blockWps_proc_read(char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", bBlockWps);
}

static int blockWps_proc_write(struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val = 0;

    if (sscanf(buf, "%d", &val) != 1)
    {
        return -EINVAL;
    }

    if ((val < 0) || (val > 1))
    {
        return -EINVAL;
    }

    printk("%s %d: write value = %d\r\n",__FUNCTION__,__LINE__,val);
    bBlockWps = val;

    return count;
}
#endif


typedef enum 
{
    LED_STATE_OFF       = 0,
    LED_STATE_GREEN     = 1, 
    LED_STATE_YELLOW    = 2,
    LED_STATE_ORANGE    = 3,
    LED_STATE_MAX       = 4,
} led_state_e;

static led_state_e gpio_tricolorled = LED_STATE_OFF;

static int gpio_tricolor_led_read (char *page, char **start, off_t off,
               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", gpio_tricolorled);
}

static int gpio_tricolor_led_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if (val >= LED_STATE_MAX)
        return -EINVAL;

    if (val == gpio_tricolorled)
    return count;

    switch (val) {
        case LED_STATE_OFF :
            /* both LEDs OFF */
            green_led_onoff = JUMPSTART_LED_OFF;
            yellow_led_onoff = JUMPSTART_LED_OFF;
            break;

        case LED_STATE_GREEN:
             /* green ON, Yellow OFF */
            green_led_onoff = JUMPSTART_LED_ON;
            yellow_led_onoff = JUMPSTART_LED_OFF;
            break;

        case LED_STATE_YELLOW:
             /* green OFF, Yellow ON */
            green_led_onoff = JUMPSTART_LED_OFF;
            yellow_led_onoff = JUMPSTART_LED_ON;
            break;

        case LED_STATE_ORANGE:
            /* both LEDs ON */
            green_led_onoff = JUMPSTART_LED_ON;
            yellow_led_onoff = JUMPSTART_LED_ON;
            break;
    }

//    ath_reg_rmw_clear(ath_get_gpio_function(TRICOLOR_LED_GREEN_PIN),
//                        0xff << ATH_GET_GPIO_SHIFT_BIT(TRICOLOR_LED_GREEN_PIN));

#ifdef CONFIG_GPIO_OPTION66
	if (rstPressTimeCounter < 1)
		ath_gpio_out_val (TRICOLOR_LED_GREEN_PIN, green_led_onoff);
#else
	ath_gpio_out_val (TRICOLOR_LED_GREEN_PIN, green_led_onoff);
#endif

    gpio_tricolorled = val;

    return count;
}

#ifdef RE_BTN_GPIO
static led_state_e gpio_re_led = LED_STATE_OFF;
static int re_led_proc_read (char *page, char **start, off_t off,
               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", gpio_re_led);
}

static int re_led_proc_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if (val >= LED_STATE_MAX)
        return -EINVAL;

    if (val == gpio_re_led)
    return count;

    switch (val) {
        case LED_STATE_OFF :
            /* both LEDs OFF */
            green_led_onoff = RE_WPS_LED_OFF;
            yellow_led_onoff = RE_WPS_LED_OFF;
            break;

        case LED_STATE_GREEN:
             /* green ON, Yellow OFF */
            green_led_onoff = RE_WPS_LED_ON;
            yellow_led_onoff = RE_WPS_LED_OFF;
            break;

        case LED_STATE_YELLOW:
             /* green OFF, Yellow ON */
            green_led_onoff = RE_WPS_LED_OFF;
            yellow_led_onoff = RE_WPS_LED_ON;
            break;

        case LED_STATE_ORANGE:
            /* both LEDs ON */
            green_led_onoff = RE_WPS_LED_OFF;
            yellow_led_onoff = RE_WPS_LED_OFF;
            break;
    }

//    ath_reg_rmw_clear(ath_get_gpio_function(TRICOLOR_LED_GREEN_PIN),
//                        0xff << ATH_GET_GPIO_SHIFT_BIT(TRICOLOR_LED_GREEN_PIN));
	
    ath_gpio_out_val (RE_WPS_LED_GPIO, green_led_onoff);
    gpio_re_led = val;

    return count;
}

#endif

static int create_simple_config_led_proc_entry(void)
{
    if (simple_config_entry != NULL) 
    {
        printk ("Already have a proc entry for /proc/simple_config!\n");
        return -ENOENT;
    }

    simple_config_entry = proc_mkdir("simple_config", NULL);
    if (!simple_config_entry)
    {
        return -ENOENT;
    }

    simulate_push_button_entry 
        = create_proc_entry ("push_button", 0644, simple_config_entry);
    if (!simulate_push_button_entry)
    {
        return -ENOENT;
    }

    simulate_push_button_entry->write_proc = push_button_write;
    simulate_push_button_entry->read_proc = push_button_read;

#ifndef SYS_LED_GPIO/*  by huangwenzhong, 21May13 */
    blockWps_entry 
        = create_proc_entry ("block_wps", 0644, simple_config_entry);
    if (!blockWps_entry)
    {
        return -ENOENT;
    }

    blockWps_entry->write_proc = blockWps_proc_write;
    blockWps_entry->read_proc = blockWps_proc_read;
#endif

    /* added by zjg, 12Apr10 */
    multi_use_reset_button_entry = create_proc_entry ("multi_use_reset_button", 0644,
                                                      simple_config_entry);
    if (!multi_use_reset_button_entry)
    {
        return -ENOENT;
    }

    multi_use_reset_button_entry->write_proc    = multi_use_reset_button_write;
    multi_use_reset_button_entry->read_proc     = multi_use_reset_button_read;
    /* end added */

#ifdef WIFI_RADIO_SW_GPIO
    /* added by zjin for multi-use wifi button, 27Apr2012 */
    multi_use_wifi_button_entry = 
        create_proc_entry ("multi_use_wifi_button", 0644, simple_config_entry);
    if (!multi_use_wifi_button_entry)
    {
        return -ENOENT;
    }

    multi_use_wifi_button_entry->write_proc = multi_use_wifi_button_write;
    multi_use_wifi_button_entry->read_proc  = multi_use_wifi_button_read;
    /* end added */

    
    /* wifi switch entry */
    wifi_button_entry = 
            create_proc_entry("wifi_button", 0666, simple_config_entry);
    if(!wifi_button_entry)
    {
        return -ENOENT;
    }
    wifi_button_entry->write_proc = wifi_button_proc_write;
    wifi_button_entry->read_proc = wifi_button_proc_read;
#endif

    tricolor_led_entry = 
            create_proc_entry ("tricolor_led", 0644, simple_config_entry);
    if (!tricolor_led_entry)
    {
        return -ENOENT;
    }

    tricolor_led_entry->write_proc = gpio_tricolor_led_write;
    tricolor_led_entry->read_proc = gpio_tricolor_led_read;

#ifdef AP_USB_LED_GPIO
    /* for usb led blink */
    usb_led_blink_entry = 
            create_proc_entry ("usb_blink", 0666, simple_config_entry);
    if (!usb_led_blink_entry)
    {
        return -ENOENT;
    }
    
    usb_led_blink_entry->write_proc = usb_led_blink_write;
    usb_led_blink_entry->read_proc = usb_led_blink_read;
    
    /*added by ZQQ, 10.06.02 for usb power*/
    usb_power_entry = 
            create_proc_entry("usb_power", 0666, simple_config_entry);
    if(!usb_power_entry)
    {
        return -ENOENT;
    }

    usb_power_entry->write_proc = usb_power_write;
    usb_power_entry->read_proc = usb_power_read;
    /*end added*/
#endif

#ifdef WORKMODE_SWITCH_1ST_GPIO
    /* workmode switch entry */
    workmode_entry = 
            create_proc_entry("workmode", 0666, simple_config_entry);
    if(!workmode_entry)
    {
        return -ENOENT;
    }
    workmode_entry->write_proc = workmode_proc_write;
    workmode_entry->read_proc = workmode_proc_read;
#endif
#ifdef RE_BTN_GPIO
	re_led_entry =
            create_proc_entry("re_led", 0666, simple_config_entry);
    if(!re_led_entry)
    {
        return -ENOENT;
    }
    re_led_entry->write_proc = re_led_proc_write;
    re_led_entry->read_proc = re_led_proc_read;
#endif
    return 0;
}


/* ioctl for reset default detection and system led switch*/
int ath_gpio_ioctl(struct inode *inode, struct file *file,  unsigned int cmd, unsigned long arg)
{
    int* argp = (int *)arg;

    if (_IOC_TYPE(cmd) != ATH_GPIO_MAGIC ||
        _IOC_NR(cmd) < ATH_GPIO_IOCTL_BASE ||
        _IOC_NR(cmd) > ATH_GPIO_IOCTL_MAX)
    {
        printk("type:%d nr:%d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
        printk("ath_gpio_ioctl:unknown command\n");
        return -1;
    }

    switch (cmd)
    {
    case ATH_GPIO_BTN_READ:/* 读取恢复出厂设置按钮的值*/
        *argp = counter;
        counter = 0;
        break;
	case ATH_GPIO_RESET_BTN_STATUS_READ: /* 读取恢复出厂设置按钮的状态*/
		*argp = bRstBtnDown;
		break;
    case ATH_GPIO_UPGRADE_FLAG_WRITE:
		bUpgradeFlags = *argp;
		printk("bUpgradeFlags:%d\n",bUpgradeFlags);
		break;
    case ATH_GPIO_BTN_PUSH_CNT_READ:/*读取恢复出厂设置按钮按下次数*/
        *argp = btnPushCnt;
        btnPushCnt = 0;
        break;

#ifdef SYS_LED_GPIO
    case ATH_GPIO_LED_WRITE:/* 系统灯点灯 */
        if (unlikely(bBlockWps))
            bBlockWps = 0;

        if (SYS_LED_GPIO >= ATH_GPIO_MIN)
        {
            ath_gpio_out_val(SYS_LED_GPIO, *argp);
        }
        break;
#endif

#ifdef  WIFI_RADIO_SW_GPIO/* for wifi switch(button) iotcl */   
    case ATH_GPIO_WIFI_SW_READ:/* 读取WIFI开关的值 */
        *argp = (ath_gpio_in_val(WIFI_RADIO_SW_GPIO)) >> WIFI_RADIO_SW_GPIO;
        
#ifdef CONFIG_GPIO_DEBUG
        print_gpio_regs("");
#endif          
        break;

    case ATH_GPIO_WIFI_BTN_READ:/* 读取WIFI按钮的值 */
        *argp = wifi_button_flag;
        wifi_button_flag = 0;
        break;
#endif

#ifdef AP_USB_LED_GPIO/* for usb led ioctl */
    case ATH_GPIO_USB_LED1_WRITE:/* USB灯点灯 */
        ath_gpio_out_val(AP_USB_LED_GPIO, *argp);
        break;
#endif

#ifdef INET_LED_GPIO/* for internet led ioctl */
    case ATH_GPIO_INET_LED_WRITE:/* 因特网灯点灯 */
		//printk("INET_LED_GPIO:%d rstPressTimeCounter=%d counter=%d\n",*argp, rstPressTimeCounter, counter);
#ifdef CONFIG_GPIO_OPTION66		
		if (rstPressTimeCounter >= 1)
			break;		
#endif
        ath_gpio_out_val(INET_LED_GPIO, *argp);
        break;
#endif

#ifdef WAN_LED_GPIO
    case ATH_GPIO_WAN_LED_WRITE:
		//printk("WAN_LED_GPIO:%d rstPressTimeCounter=%d  counter=%d\n",*argp, rstPressTimeCounter, counter);
#ifdef CONFIG_GPIO_OPTION66		
		if (rstPressTimeCounter >= 1)
			break;
#endif		
        ath_gpio_out_val(WAN_LED_GPIO,*argp);
        break;
#endif

#ifdef WIFI_LED_GPIO
    case ATH_GPIO_WIFI_LED_WRITE:
        ath_gpio_out_val(WIFI_LED_GPIO, *argp);
        break;
#endif

#ifdef POWER_LED_GPIO_BIT
    case ATH_GPIO_POWER_LED_WRITE:
        ath_gpio_out_val(POWER_LED_GPIO_BIT, *argp);
        break;
#endif

#ifdef LAN_LED_GPIO
    case ATH_GPIO_LAN_LED_WRITE:
		//wangyouwang wyw LAN IOCTL ENTYRY
	    /*printk("===>LAN_LED_GPIO:%d,argv:%d",LAN_LED_GPIO,(int)*argp);*/
        ath_gpio_out_val(LAN_LED_GPIO, *argp);
        break;
#endif

#ifdef RE_BTN_GPIO
	case ATH_GPIO_RE_BTN_READ:
		*argp = re_button_flag;
		re_button_flag = 0;
		break;
#endif

    default:
	/**
	 * Some mach may not support all GPIO functions
	 */
	#if 0
        printk("command not supported %x\n", cmd);
	#endif
        return -1;
    }


    return 0;
}


int ath_gpio_open (struct inode *inode, struct file *filp)
{
    int minor = iminor(inode);
    int devnum = minor; //>> 1;
    struct mtd_info *mtd;

    if ((filp->f_mode & 2) && (minor & 1))
    {
        printk("You can't open the RO devices RW!\n");
        return -EACCES;
    }

    mtd = get_mtd_device(NULL, devnum);   
    if (!mtd)
    {
        printk("Can not open mtd!\n");
        return -ENODEV; 
    }
    filp->private_data = mtd;
    return 0;

}

/* struct for cdev */
struct file_operations gpio_device_op =
{
    .owner = THIS_MODULE,
    .ioctl = ath_gpio_ioctl,
    .open = ath_gpio_open,
};

/* struct for ioctl */
static struct cdev gpio_device_cdev =
{
    .owner  = THIS_MODULE,
    .ops    = &gpio_device_op,
};
/******* end  ioctl stuff **********/



int __init ath_simple_config_init(void)
{
    /* restore factory default and system led */
    dev_t dev;
    int rt;
    int ath_gpio_major = gpio_major;
    int ath_gpio_minor = gpio_minor;


    init_timer(&rst_timer);
    rst_timer.function = check_rst;
	
#ifdef CONFIG_GPIO_OPTION66	
	init_timer(&rst_timer_opt66);
    rst_timer_opt66.function = check_rst_blink;
#endif	

#ifdef WIFI_RADIO_SW_GPIO
    init_timer(&wifi_button_timer);
    wifi_button_timer.function = wifi_sw_check;
#endif
#ifdef RE_BTN_GPIO
    init_timer(&re_button_timer);
    re_button_timer.function = re_check;
#endif

#if 1
/* Disable JTAG. because we will use GPIO14/GPIO15 */
    ath_reg_rmw_set(ATH_GPIO_FUNCTIONS, ATH_GPIO_FUNCTION_JTAG_DISABLE);
    /* config gpio 18, 3, 8, 14/15, 4, 5, 6, 7 as normal gpio function */
    /* gpio4, gpio5, gpio6, gpio7 */
#ifdef CONFIG_GPIO_941HP
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION1, 0xffff<<16);
#else
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION1, 0xffffffff);
#endif
    /* gpio3 */
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION0, 0xff<<24);
    /* gpio8 */
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION2, 0xff<<0);
    /* gpio14, gpio15 */
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION3, 0xffff<<16);
    /* gpio18 */
    ath_reg_rmw_clear(ATH_GPIO_OUT_FUNCTION4, 0xff<<16);
#ifdef CONFIG_GPIO_941HP
	/*GPIO 4\5 of 941HP is not led */
#else
        ath_gpio_config_output(4);
    ath_gpio_out_val(4, 1);
        ath_gpio_config_output(5);
    ath_gpio_out_val(5, 1);
#endif
        ath_gpio_config_output(6);
    ath_gpio_out_val(6, 1);
        ath_gpio_config_output(7);
    ath_gpio_out_val(7, 1);

        ath_gpio_config_output(3);
    ath_gpio_out_val(3, 1);
        ath_gpio_config_output(8);
    ath_gpio_out_val(8, 1);
        ath_gpio_config_output(14);
#if defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
	ath_gpio_out_val(14, 0);
#else
    ath_gpio_out_val(14, 1);
#endif
    ath_gpio_config_output(15);
#if defined(CONFIG_GPIO_941HP) || defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV6)
    ath_gpio_out_val(15, 1);
#else
	ath_gpio_out_val(15, 0);
#endif

#endif

    /* config gpio3, 12, 13, 17 as normal gpio function */

    /* because we use GPIO4 as WAN LED, so we need clean bit6 */
    /* we do it in ethernet driver instead */
    //ath_reg_rmw_clear(ATH_GPIO_FUNCTIONS, 1 << 6);

    /* disable JTAG before use GPIO3 */
    ath_reg_rmw_set(ATH_GPIO_FUNCTIONS, 1 << 1);

#ifdef AP_USB_LED_GPIO
    /* gpio X usb 使能开关*/
    ath_reg_rmw_clear(ath_get_gpio_function(USB_POWER_SW_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(USB_POWER_SW_GPIO));
    /* gpio Y usb 灯*/
    ath_reg_rmw_clear(ath_get_gpio_function(AP_USB_LED_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(AP_USB_LED_GPIO));
#endif

#ifdef SYS_LED_GPIO 
    /* gpio13 系统灯*/
    ath_reg_rmw_clear(ath_get_gpio_function(SYS_LED_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(SYS_LED_GPIO));
#endif

    /* gpio3 QSS灯*/


#ifdef WIFI_RADIO_SW_GPIO
    /* gpio17 WIFI按钮/开关*/
    ath_reg_rmw_clear(ath_get_gpio_function(WIFI_RADIO_SW_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(WIFI_RADIO_SW_GPIO));
#endif

    /* gpio12 恢复出厂设置键*/
    ath_reg_rmw_clear(ath_get_gpio_function(RST_DFT_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(RST_DFT_GPIO));

#ifdef INET_LED_GPIO
    /*  by huangwenzhong, 06Mar13 */
    /* gpio X 因特网灯，目前只有WR803N才有*/
#if defined(CONFIG_GPIO_940NV6)

#else
    ath_reg_rmw_clear(ath_get_gpio_function(INET_LED_GPIO),
                    0xff << ATH_GET_GPIO_SHIFT_BIT(INET_LED_GPIO));
#endif
#endif

#ifdef RE_BTN_GPIO
    /* RE键*/
    ath_reg_rmw_clear(ath_get_gpio_function(RE_BTN_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(RE_BTN_GPIO));
#endif
#ifdef WPS_BTN_GPIO
/*wps button*/
	ath_reg_rmw_clear(ath_get_gpio_function(WPS_BTN_GPIO),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(WPS_BTN_GPIO));
#endif

        /* GPIO3 WPS LED reconfigure, configure wps gpio as outputs */
    ath_reg_rmw_clear(ath_get_gpio_function(TRICOLOR_LED_GREEN_PIN),
                        0xff << ATH_GET_GPIO_SHIFT_BIT(TRICOLOR_LED_GREEN_PIN));
    ath_gpio_config_output(TRICOLOR_LED_GREEN_PIN);
    ath_gpio_out_val (TRICOLOR_LED_GREEN_PIN, JUMPSTART_LED_OFF);
    create_simple_config_led_proc_entry();

    ath_gpio_config_input(RST_DFT_GPIO);

    /* configure GPIO RST_DFT_GPIO as level triggered interrupt */
    ath_gpio_config_int (RST_DFT_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    rt = request_irq (ATH_GPIO_IRQn(RST_DFT_GPIO), rst_irq, 0,
                       "RESTORE_FACTORY_DEFAULT", NULL);
    if (rt != 0)
    {
        printk (KERN_ERR "unable to request IRQ for RESTORE_FACTORY_DEFAULT GPIO (error %d)\n", rt);
    }

#ifdef WIFI_RADIO_SW_GPIO
    /* wifi switch! */
    ath_gpio_config_input(WIFI_RADIO_SW_GPIO);

    /* configure GPIO WIFI_RADIO_SW_GPIO as level triggered interrupt */
    ath_gpio_config_int (WIFI_RADIO_SW_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    rt = request_irq (ATH_GPIO_IRQn(WIFI_RADIO_SW_GPIO), 
                    wifi_sw_irq, 0, "WIFI_RADIO_SWITCH", NULL);
    if (rt != 0)
    {
        printk (KERN_ERR "unable to request IRQ for WIFI_RADIO_SWITCH GPIO (error %d)\n", rt);
    }
#endif
#ifdef RE_BTN_GPIO
    /*re button! */
    ath_gpio_config_input(RE_BTN_GPIO);

    /* configure GPIO WIFI_RADIO_SW_GPIO as level triggered interrupt */
    ath_gpio_config_int (RE_BTN_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    rt = request_irq (ATH_GPIO_IRQn(RE_BTN_GPIO), 
                    re_btn_irq, 0, "RE_BUTTON", NULL);
    if (rt != 0)
    {
        printk (KERN_ERR "unable to request IRQ for RE_BUTTON (error %d)\n", rt);
    }
#endif
#ifdef WPS_BTN_GPIO
    /*wps button! */
    ath_gpio_config_input(WPS_BTN_GPIO);

    /* configure GPIO WIFI_RADIO_SW_GPIO as level triggered interrupt */
    ath_gpio_config_int (WPS_BTN_GPIO, INT_TYPE_EDGE, INT_POL_ACTIVE_LOW);

    rt = request_irq (ATH_GPIO_IRQn(WPS_BTN_GPIO), 
                    wps_btn_irq, 0, "WPS_BTN_GPIO", NULL);
    if (rt != 0)
    {
        printk (KERN_ERR "unable to request IRQ for WPS_BTN_GPIO (error %d)\n", rt);
    }
#endif
    /* Work mode switchs!
     * 14   16
        L   H   3G
        H   L   Router/APC Router
        H   H   AP
     */
#ifdef WORKMODE_SWITCH_1ST_GPIO
    ath_gpio_config_input(WORKMODE_SWITCH_1ST_GPIO);
    ath_gpio_config_input(WORKMODE_SWITCH_2ND_GPIO);
#endif

#ifdef AP_USB_LED_GPIO
    /* Configure GPIOs default status */
    ath_gpio_config_output(AP_USB_LED_GPIO);
    ath_gpio_out_val(AP_USB_LED_GPIO, USB_LED_OFF);
    
    ath_gpio_config_output(USB_POWER_SW_GPIO);
    ath_gpio_out_val(USB_POWER_SW_GPIO, 1);
#endif

#ifdef SYS_LED_GPIO 
    if (SYS_LED_GPIO >= ATH_GPIO_MIN)
    {
        ath_gpio_config_output(SYS_LED_GPIO);
        ath_gpio_out_val(SYS_LED_GPIO, SYS_LED_OFF);
    }
#endif
    
#ifdef INET_LED_GPIO
#if defined(CONFIG_GPIO_945NV1) || defined(CONFIG_GPIO_940NV7)
	//Inet Led ON
#else
    ath_gpio_config_output(INET_LED_GPIO);
    ath_gpio_out_val(INET_LED_GPIO, INET_LED_OFF);
#endif	
#endif

#if defined(CONFIG_GPIO_940NV6) && defined(WAN_LED_GPIO)
	//printk("......................................\n\n\n\n\n\n\n\n\n940nv6 WAN LED ON\n\n\n\n\n\n\n\n\n........................................\n");
	ath_gpio_config_output(WAN_LED_GPIO);
	ath_gpio_out_val(WAN_LED_GPIO, WAN_LED_ON);
#endif



#ifdef POWER_LED_GPIO_BIT
    ath_gpio_config_output(POWER_LED_GPIO_BIT);
    ath_gpio_out_val(POWER_LED_GPIO_BIT, POWER_LED_GPIO_ON);
#endif


    /* Create char device for gpio */
    if (ath_gpio_major)
    {
        dev = MKDEV(ath_gpio_major, ath_gpio_minor);
        rt = register_chrdev_region(dev, 1, "ar7240_gpio_chrdev");
    }
    else
    {
        rt = alloc_chrdev_region(&dev, ath_gpio_minor, 1, "ar7240_gpio_chrdev");
        ath_gpio_major = MAJOR(dev);
    }

    if (rt < 0)
    {
        printk(KERN_WARNING "ar7240_gpio_chrdev : can`t get major %d\n", ath_gpio_major);
        return rt;
    }

    cdev_init (&gpio_device_cdev, &gpio_device_op);
    rt = cdev_add(&gpio_device_cdev, dev, 1);
    
    if (rt < 0) 
    {
        printk(KERN_NOTICE "Error %d adding ar7240_gpio_chrdev ", rt);
    }
    //print_gpio_regs(-1);
    return 0;
}

//subsys_initcall(ath_simple_config_init);

/*
 * used late_initcall so that misc_register will succeed
 * otherwise, misc driver won't be in a initializated state
 * thereby resulting in misc_register api to fail.
 */
#if !defined(CONFIG_ATH_EMULATION)
late_initcall(ath_simple_config_init);
#endif

