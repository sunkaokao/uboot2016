#define GPFCON        (*((volatile unsigned long *)0x56000050))
#define GPFDAT        (*((volatile unsigned char *)0x56000054))
#define LED1_MASK      (1 << 4)
#define LED2_MASK      (1 << 5)
#define LED3_MASK      (1 << 6)    
#define LED_MASK(n)    (LED##n##_MASK)


#define NFCONF        (*((volatile unsigned long *)0x4E000000))
#define NFCONT        (*((volatile unsigned long *)0x4E000004))
#define NFCMMD        (*((volatile unsigned char *)0x4E000008))
#define NFADDR        (*((volatile unsigned char *)0x4E00000C))
#define NFDATA        (*((volatile unsigned long *)0x4E000010))
#define NFSTAT        (*((volatile unsigned long *)0x4E000020))

#define TACLS_VAL     0
#define TWRPH0_VAL    1
#define TWRPH1_VAL    0

#define NAND_BLOCK_SIZE     2048
#define NAND_BLOCK_MASK    (NAND_BLOCK_SIZE - 1)




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

static void nand_select(void)
{
    int i = 0;
    
    NFCONT &= ~(1 << 1);
    for(i = 0; i < 10; i++);
}

static void nand_deselect(void)
{    
    int i = 0;
    
    NFCONT |= (1 << 1);
    for(i = 0; i < 10; i++); 
}

static void nand_write_cmd(unsigned char cmd)
{
    NFCMMD = cmd;
}

static void nand_write_addr(unsigned int addr)
{
    int col = 0;
    int page = 0;
    int i = 0;

    page = addr / NAND_BLOCK_SIZE;
    col &= NAND_BLOCK_MASK;

    NFADDR = (col & 0xFF);
    for(i = 0; i < 10; i++);
    NFADDR = ((col >> 8) & 0x0F);
    for(i = 0; i < 10; i++);
    
    NFADDR = (page & 0xFF);
    for(i = 0; i < 10; i++); 
    NFADDR = ((page >> 8) & 0xFF);
    for(i = 0; i < 10; i++);
    NFADDR = ((page >> 16) & 0x01);
    for(i = 0; i < 10; i++);


}

static void nand_wait_idle(void)
{
    int i = 0;
    
    while(!(NFSTAT & 0x01))
    {
        for(i = 0; i < 10; i++);
    }
}

static unsigned char nand_read_byte(void)
{
    return NFDATA;
}

static void nand_reset(void)
{
    nand_select();

    nand_write_cmd(0xFF);

    nand_wait_idle();

    nand_deselect();
}

int nand_read_ll(unsigned char *buf, unsigned int start_addr, int len)
{
    int i = 0, j = 0;

    if((start_addr & NAND_BLOCK_MASK) || (len & NAND_BLOCK_MASK))
    {
        return -1;
    }

    nand_select();

    for(i = 0; i < (start_addr + len); )
    {
        nand_write_cmd(0x00);
        
        nand_write_addr(start_addr);
        
        nand_write_cmd(0x30);
        
        nand_wait_idle();

        for(j = 0; j < NAND_BLOCK_SIZE; i++, j++)
        {
            *buf = nand_read_byte();
            buf++;
        }
    }

    nand_deselect();

    return 0;
}


void nand_init_ll(void)
{
    NFCONF = (TACLS_VAL << 12) | (TWRPH0_VAL << 8) | (TWRPH1_VAL << 4);
    NFCONT = (1<<4)|(1<<1)|(1<<0);

//    nand_reset();
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

    if(isBootFromNorFlash)
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


