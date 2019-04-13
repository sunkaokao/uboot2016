#include "common.h"
#include "command.h"


void nandread(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int i = 0;
    unsigned char buf[512] = {0};

    int start_addr = simple_strtoul(argv[1], NULL, 16);
    int size = simple_strtoul(argv[2], NULL, 16);

    if(3 < argc)
    {
        printf("usage error!\n");
    }

    printf("start_addr: %d\n size: %d\n", start_addr, size);

    if(512 < size)
    {
        printf("nand read < 512 bytes\n");
        return;
    }

    nand_read_ll(buf, start_addr, size);

    printf("nand data: \n");
    
    for(i = 0; i < size; i++)
    {
        printf("%x ", buf[i]);
    }
    printf("\n");
}

#if 0
void nandwrite(cmd_table_t *cmd, int flag, int argc, char * const argv[])
{
    
}
#endif



U_BOOT_CMD(readflash, 3, 0, nandread,
  "test nand data",
  "test read nand cmd\n"
  "usage: nandread dst size");

#if 0
  U_BOOT_CMD(writeflash, 3, 0, nandwrite,
    "test nand data",
    "test write nand cmd\n"
    "usage: nandaddr dst size");
#endif




