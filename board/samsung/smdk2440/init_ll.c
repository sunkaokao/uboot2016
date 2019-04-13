#define GPFCON        (*((volatile unsigned long *)0x56000050))
#define GPFDAT        (*((volatile unsigned char *)0x56000054))
#define LED1_MASK      (1 << 4)
#define LED2_MASK      (1 << 5)
#define LED3_MASK      (1 << 6)    
#define LED_MASK(n)    (LED##n##_MASK)


#define NFCONF     (*(volatile unsigned long *)0x4E000000)
#define NFCONT     (*(volatile unsigned long *)0x4E000004)

#define NFCMMD     (*(volatile unsigned char *)0x4E000008)
#define NFADDR     (*(volatile unsigned char *)0x4E00000C)
#define NFDATA     (*(volatile unsigned char *)0x4E000010)
#define NFSECCD    (*(volatile unsigned long *)0x4E00001C)
#define NFSTAT     (*(volatile unsigned long *)0x4E000020)

#define TACLS      0
#define TWRPH0     1
#define TWRPH1     0

#define PAGE_SIZE   2048
#define PAGE_ALIGN  (PAGE_SIZE - 1)


void led_init_ll(void)
{
    GPFCON = (1 << 12) | (1 << 10) | (1 << 8);
    GPFDAT = (1 << 6) | (1 << 5) | (1 << 4);
}

void led_on_ll(int n)
{
    switch(n)
    {
    case 1:
        GPFDAT &= 0xEF;
        /*GPFDAT &= ~(LED_MASK(1)); */
        break;
    case 2:
        GPFDAT &= 0xDF;
        /*GPFDAT &= ~(LED_MASK(2)); */
        break;
    case 3:
        GPFDAT &= 0xBF;
        /*GPFDAT &= ~(LED_MASK(3)); */
        break;
    default:
        break;
    }
    
}

void led_off_ll(int n)
{
    switch(n)
    {
    case 1:
        GPFDAT |= 0x10;
        /*GPFDAT |= LED_MASK(1);*/
        break;
    case 2:
        GPFDAT |= 0x20;
        /*GPFDAT |= LED_MASK(2); */
        break;
    case 3:
        GPFDAT |= 0x40;
        /*GPFDAT |= LED_MASK(3); */
        break;
    default:
        break;
    }

}

/*----------- --- for nand flash ---------------*/

static void nand_wait_idle(void)
{
    while(!(NFSTAT & 0x01));
}

static void nand_select(void)
{
    int i = 0;
    
    NFCONT &= ~(1 << 1);
    for(i=0; i<10; i++);    // 延时一小会儿
}

static void nand_deselect(void)
{
    NFCONT |= (1 << 1);
}

static void nand_write_cmd(int cmd)
{
    NFCMMD = cmd;
}

static void nand_write_addr(unsigned int addr)
{
    int i = 0;
    unsigned col = 0, raw = 0;
    
    col = addr & PAGE_ALIGN;    // һҳ2Kb
    raw = addr / PAGE_SIZE;
    
    NFADDR = col & 0xff;
    for(i=0; i<10; i++);
    NFADDR = (col >> 8) & 0x0f;
    for(i=0; i<10; i++);
    NFADDR = raw & 0xff;
    for(i=0; i<10; i++);
    NFADDR = (raw >> 8) & 0xff;
    for(i=0; i<10; i++);
    NFADDR = (raw >> 16) & 0x03;
    for(i=0; i<10; i++);
}

static unsigned char nand_read_byte(void)
{
    return NFDATA;
}

static void nand_reset(void)
{
    nand_select();
    nand_write_cmd(0xff);
    nand_wait_idle();
    nand_deselect();
}

void nand_init_ll(void)
{
    NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
    NFCONT = (1<<4)|(1<<1)|(1<<0);
    
    nand_reset();
}

int nand_read_ll(unsigned char *buf, unsigned int start_addr, int len)
{
    int col = start_addr % 2048;
    int i = 0;
   
    nand_select();
    
    while(i < len)
    {
        nand_write_cmd(0x00);
        nand_write_addr(start_addr);
        nand_write_cmd(0x30);
        nand_wait_idle();
        
        for(; (i < len) && (col < 2048); col++)
        {
            buf[i] = nand_read_byte();
            i++;
            start_addr++;
        }
        
        col = 0;
    }
    
    nand_deselect();

    return 0;
}

int isBootFromNorFlash(void)
{
    volatile unsigned int *p = (volatile unsigned int *)0;
    unsigned int val = 0;

    val = *p;
    *p = 0x12345678;
    if(0x12345678 == *p)
    {
        *p = val;
        return 0;
    }
    else
    {
        return 1;
    }
}

void copy_code_to_sdram(unsigned char *dest, unsigned char *src, int len)
{
    int i = 0;

    if(isBootFromNorFlash())
    {
        for(i = 0; i < len; i++)
        {
            dest[i] = src[i];
        }
    }
    else
    {
        nand_read_ll(dest, (unsigned int)src, len);
    }
}

void clear_bss(unsigned int start, unsigned int end)
{
    unsigned int *p = (unsigned int *)start;

    for(; p < end; p++)
    {
        *p = 0;
    }
}


