#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__
#include <linux/miscdevice.h>
#include <linux/fs.h>

static struct miscdevice llkd_miscdev = {
    .minor = MISC_DYNAMIC_MINOR, /* ядро динамически выделяет минорный номер нашему драйверу */
    .name = "llkd_miscdrv", /*ядро автоматически создает файл устройства в dev */


    .mode = 0666, /* эта строчка задает права доступа к файлу твего драйвера в папке  dev*/
    .fops = &llkd_misc_fops /*передаем ядру ссылку на список своих Си функций */
};

static  int __init miscdrv_init (void):
{
    int ret; 
    struct device *dev;
    ret = misc_register(&llkd_miscdev);
    if (ret!=0)
    {
        pr_notice("misc device registration failed\n");
    }
    return ret;
}
