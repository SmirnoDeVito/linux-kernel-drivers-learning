#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
#include <linux/miscdevice.h>
#include <linux/fs.h>

/*временная заглушка функции open_miscdrv*/
static int open_miscdrv(struct inode *inode, struct file *file)
{
    return 0;
}

/*список функций*/
static const struct file_operations llkd_misc_fops = { 
    /*создаем и заполняем структуру file_operations*/
    /*file_operations - структура в которой записано, какие функции должен вызвать драйвер ,*/
    /* когда пользователь делает то или иное действеи с файлом в DEV*/
    .open = open_miscdrv,
    /*.read = open_miscdrv,
    .write =open_miscdrv,
    .release = open_miscdrv,*/
};

/*бланк misc*/
static struct miscdevice llkd_miscdev = {
    .minor = MISC_DYNAMIC_MINOR, /* ядро динамически выделяет минорный номер нашему драйверу */
    .name = "llkd_miscdrv", /*ядро автоматически создает файл устройства в dev */


    .mode = 0666, /* эта строчка задает права доступа к файлу твего драйвера в папке  dev*/
    .fops = &llkd_misc_fops, /*передаем ядру ссылку на список своих Си функций */
    /*связывание со списком функций*/
};

/**/
static void __exit miscdrv_exit(void)
{
    misc_deregister(&llkd_miscdev);
    pr_info("LLKD misc driver unregistered");
}

/*модуль загрузки*/
static  int __init miscdrv_init (void)
{
    int ret; 
    struct device *dev;
    ret = misc_register(&llkd_miscdev);
    if (ret!=0)
    {
        pr_notice("misc device registration failed\n");
        return ret;
    }
    dev = llkd_miscdev.this_device;

    pr_info ("LLKD misc driver minor = %d, dev name = /dev/%s \n", llkd_miscdev.minor, llkd_miscdev.name);
    /*простой вывод пишет текст в лог*/


    dev_info (dev, "dev_info  minor -  %d \n", llkd_miscdev.minor);
    /*умный вывод просить в начале указатель на структуру miscdevice*/
    return ret;
};

module_init(miscdrv_init);
module_exit(miscdrv_exit);
MODULE_LICENSE("GPL");

