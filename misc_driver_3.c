#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

/*контекст драйверов - одна большая структура , в которую мы складываем все переменные,*/
/*которые нужны вашему драйверу для работы*/
struct my_driver_context
{
    struct device *dev;
    int open_count;
    char secret[128];
    int tx, rx;
};

static struct my_driver_context *ctx;

/*обновленная функция open_miscdrv*/
static int open_miscdrv(struct inode *inode, struct file *file)
{
    /*Теперь наш драййвер хранит состояние, он запоминает сколько раз мы использовали функцию open*/
    if (ctx)
    {
        ctx->open_count++;
        pr_info("Driver opened %d times\n", ctx->open_count);
        
    }
    return 0;
}

/*обновленная функция read */
static ssize_t read_miscdrv (struct file *file, char __user *ubuf, size_t count, loff_t *off )
{
    pr_info("read !");
    char *kbuf = kzalloc(count, GFP_KERNEL);
    /*поменяли PATH_MAX на count чтобы выделить определенное колличество байт, а не максимальное */

    /*добавилась проверка на секретное слово*/
    if (ctx && ctx->secret[0]!=0)
    {
        strncpy(kbuf, ctx->secret, count);
    } else {
        strncpy(kbuf, "Hello from driver!\n", count);
    }

    /*копируем из буфера дайвера в буфер программы*/
    if (copy_to_user(ubuf, kbuf, count))
    {
        pr_err("copy to user failes\n");
        goto failed_s;
    }
    /*функция copy_to_user берет данные с kbuf и безопасно перекладывает их в ubuf*/
    /* Если она не смогла это сделать возвращает не нулевое значение*/
    return  count;   
  
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

/*обновленная функция выгрузки драйвера */
static void __exit miscdrv_exit(void)
{
    misc_deregister(&llkd_miscdev);
    
    /*добавленна очистка контекста драйвера*/
    if (ctx)
    {
        kfree(ctx);
    }
    pr_info("LLKD misc driver unregistered");
}

/*модуль загрузки*/
static  int __init miscdrv_init (void)
{
    int ret; 
    ctx = kzalloc(sizeof(struct my_driver_context), GFP_KERNEL);
    
    if (!ctx) {
        pr_err("Failed to allocate context\n");
        return -ENOMEM;
    }
    
    ret = misc_register(&llkd_miscdev);
    if (ret!=0)
    {
        pr_notice("misc device registration failed\n");
        return ret;
    }
    ctx->dev = llkd_miscdev.this_device;
    ctx->open_count = 0;
   strncpy(ctx->secret, "This is a secret message!\n", sizeof(ctx->secret));

    // 4. Выводим информацию
    pr_info("LLKD misc driver minor = %d, dev name = /dev/%s\n", 
            llkd_miscdev.minor, llkd_miscdev.name);
    dev_info(ctx->dev, "dev_info minor - %d\n", llkd_miscdev.minor);
    return 0;
};

module_init(miscdrv_init);
module_exit(miscdrv_exit);
MODULE_LICENSE("GPL");



