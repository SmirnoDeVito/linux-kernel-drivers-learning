#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

/*контекст драйверов - одна большая структура , в которую мы складываем все переменные,*/
/*которые нужны вашему драйверу для работы*/
struct my_driver_context
{
    int open_count;
    char secret[128];
    int tx, rx;
};


/*временная заглушка функции open_miscdrv*/
static int open_miscdrv(struct inode *inode, struct file *file)
{
    /*внутренняя структура данных в ядре, которая описывает сам файл*/
    char *buf = kzalloc(PATH_MAX, GFP_KERNEL);
    /* kzalloc - calloc , так как в ядре нет библиотеки libc у нее свои функции для расширения кучи*/
    /*PATH_MAX - константа, означает всю возможную длинну пути к файлу 4096 байт*/
    /* GFP_KERNEL - флга, который говорит менеджеру памяти , как себя вести при выделении*/
    if(unlikely(!buf))
    { 
        /*unlikely - подсказка для процессора , оптимизация , подсказывают компилятору какой исход более вероятен*/
        return -ENOMEM;
    }

    pr_info(" opening!\n");
    kfree(buf);
    /*освобождение памяти*/
    return 0;
}

/*заглушка для чтения */
static ssize_t read_miscdrv (struct file *file, char __user *ubuf, size_t count, loff_t *off )
{
    pr_info("read !");
    char *kbuf = kzalloc(PATH_MAX, GFP_KERNEL);
    strncpy(kbuf, "Hello from driver!\n", count);
    /*Теперь у нас данные лежат в kbuf, программа ждет их в  ubuf ( в своей памяти )*/
    /*функция copy_to_user берет данные с kbuf и безопасно перекладывает их в ubuf*/
    /* Если она не смогла это сделать возвращает не нулевое значение*/
    if (copy_to_user(ubuf, kbuf, count))
    {
        pr_err("copy to user failes\n");
        goto failed_s;
    }
    return count;
    failed_s:
    kfree(kbuf);

    return -EIO;
}

/*список функций*/
static const struct file_operations llkd_misc_fops = { 
    /*создаем и заполняем структуру file_operations*/
    /*file_operations - структура в которой записано, какие функции должен вызвать драйвер ,*/
    /* когда пользователь делает то или иное действеи с файлом в DEV*/
    .open = open_miscdrv,
    .read = read_miscdrv,
    /*.write =open_miscdrv,
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



