#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#define LED_CONTROL_DEV_NAME      "sample_led"
#define LED_CLASS_NAME            "sample_led_class"
#define CHARDEV_BASE_MAJOR        0 // Dynamic major number allocation

#define RED_LED_GPIO_PIN          272
#define YELLOW_LED_GPIO_PIN       258
#define GREEN_LED_GPIO_PIN        268

#define BUFF_LEN                  5
#define NUM_LEDS                  3

typedef struct {
    struct cdev cdev;
    dev_t dev;
    char state[BUFF_LEN];
    int gpio_pin;
} led_controller_dev;

static led_controller_dev led_devs[NUM_LEDS] = {
    { .gpio_pin = RED_LED_GPIO_PIN, .state = "off" },
    { .gpio_pin = YELLOW_LED_GPIO_PIN, .state = "off" },
    { .gpio_pin = GREEN_LED_GPIO_PIN, .state = "off" },
};

static struct class *led_class;

int chardev_open(struct inode *inode, struct file *filep);
int chardev_release(struct inode *inode, struct file *filep);
ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset);
ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset);

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = chardev_read,
    .write = chardev_write,
    .open = chardev_open,
    .release = chardev_release,
};

int chardev_open(struct inode *inode, struct file *filep) {
    led_controller_dev *led = container_of(inode->i_cdev, led_controller_dev, cdev);
    filep->private_data = led;
    pr_info("Opened device for GPIO %d\n", led->gpio_pin);
    return 0;
}

int chardev_release(struct inode *inode, struct file *filep) {
    pr_info("Device closed\n");
    return 0;
}

ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
    led_controller_dev *led = filep->private_data;
    size_t state_len = strlen(led->state);

    if (*offset != 0) return 0; // EOF

    if (len < state_len + 1) return -EINVAL;

    if (copy_to_user(buf, led->state, state_len + 1)) return -EFAULT;

    *offset += state_len + 1;
    return state_len + 1;
}

ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
    led_controller_dev *led = filep->private_data;
    char user_buf[BUFF_LEN];

    if (len >= sizeof(user_buf)) return -EINVAL;

    if (copy_from_user(user_buf, buf, len)) return -EFAULT;

    user_buf[len] = '\0';

    if (strncmp(user_buf, "on", 2) == 0) {
        snprintf(led->state, sizeof(led->state), "on");
        gpio_set_value(led->gpio_pin, 1);
        pr_info("LED on GPIO %d turned ON\n", led->gpio_pin);
    } else if (strncmp(user_buf, "off", 3) == 0) {
        snprintf(led->state, sizeof(led->state), "off");
        gpio_set_value(led->gpio_pin, 0);
        pr_info("LED on GPIO %d turned OFF\n", led->gpio_pin);
    } else {
        pr_info("Invalid command: %s\n", user_buf);
        return -EINVAL;
    }

    return len;
}

static int __init led_control_init(void) {
    int ret, i;
    dev_t dev;

    // Allocate major number
    ret = alloc_chrdev_region(&dev, 0, NUM_LEDS, LED_CONTROL_DEV_NAME);
    if (ret) {
        pr_err("Failed to allocate major number\n");
        return ret;
    }

    // Create class
    led_class = class_create(THIS_MODULE, LED_CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev_region(dev, NUM_LEDS);
        pr_err("Failed to create class\n");
        return PTR_ERR(led_class);
    }

    // Initialize devices
    for (i = 0; i < NUM_LEDS; i++) {
        led_devs[i].dev = MKDEV(MAJOR(dev), MINOR(dev) + i);

        // Initialize GPIO
        ret = gpio_request(led_devs[i].gpio_pin, "LED_GPIO");
        if (ret) {
            pr_err("Failed to request GPIO %d\n", led_devs[i].gpio_pin);
            goto fail_gpio;
        }
        gpio_direction_output(led_devs[i].gpio_pin, 0);

        // Initialize cdev
        cdev_init(&led_devs[i].cdev, &fops);
        led_devs[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&led_devs[i].cdev, led_devs[i].dev, 1);
        if (ret) {
            pr_err("Failed to add cdev for GPIO %d\n", led_devs[i].gpio_pin);
            goto fail_cdev;
        }

        // Create device file
        device_create(led_class, NULL, led_devs[i].dev, NULL, "led%d", i);
    }

    pr_info("LED controller initialized\n");
    return 0;

fail_cdev:
    gpio_free(led_devs[i].gpio_pin);
fail_gpio:
    while (--i >= 0) {
        device_destroy(led_class, led_devs[i].dev);
        cdev_del(&led_devs[i].cdev);
        gpio_free(led_devs[i].gpio_pin);
    }
    class_destroy(led_class);
    unregister_chrdev_region(dev, NUM_LEDS);
    return ret;
}

static void __exit led_control_exit(void) {
    int i;

    for (i = 0; i < NUM_LEDS; i++) {
        gpio_set_value(led_devs[i].gpio_pin, 0);
        gpio_free(led_devs[i].gpio_pin);
        device_destroy(led_class, led_devs[i].dev);
        cdev_del(&led_devs[i].cdev);
    }

    class_destroy(led_class);
    unregister_chrdev_region(led_devs[0].dev, NUM_LEDS);

    pr_info("LED controller exited\n");
}

module_init(led_control_init);
module_exit(led_control_exit);

MODULE_DESCRIPTION("LED Control Kernel Module for multiple LEDs");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");
