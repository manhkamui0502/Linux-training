#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // for copy_to_user and copy_from_user
#include <linux/gpio.h>

#define LED_CONTROL_DEV_NAME      "multiple_led"
#define LED_CLASS_NAME            "multiple_led_class"

#define RED_LED_GPIO_PIN          272
#define YELLOW_LED_GPIO_PIN       258
#define GREEN_LED_GPIO_PIN        268

#define BUFF_LEN                  5
#define NUM_LEDS                  3

static struct class *led_class;

// Structure representing an LED controller device
typedef struct {
  struct device *device_name;   // Device structure for sysfs
  struct cdev cdev;             // Character device structure
  dev_t dev;                    // Device number
  int gpio_pin;                 // GPIO pin associated with the LED
  char state[BUFF_LEN];         // Buffer to store the current LED state ("on"/"off")
} led_controller_dev;

// Array of LED devices for red, yellow, and green LEDs
static led_controller_dev led_cdev[NUM_LEDS] = {
  {.gpio_pin = RED_LED_GPIO_PIN, .state = "off"}, 
  {.gpio_pin = YELLOW_LED_GPIO_PIN, .state = "off"}, 
  {.gpio_pin = GREEN_LED_GPIO_PIN, .state = "off"}, 
};

// Function prototypes for file operations
static int chardev_open(struct inode *inode, struct file *filep);
static int chardev_release(struct inode *inode, struct file *filep);
static ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset);
static ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset);

// File operations for the LED controller
static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = chardev_read,
  .write = chardev_write,
  .open = chardev_open,
  .release = chardev_release,
  .llseek = no_llseek,
};

// Function to open the device file
static int chardev_open(struct inode *inode, struct file *filep) {
  // Get the LED device structure associated with this inode
  led_controller_dev *led = container_of(inode->i_cdev, led_controller_dev, cdev);
  filep->private_data = led; // Store the LED device in file's private data
  pr_info("Opened device for GPIO %d\n", led->gpio_pin);
  return 0;
}

// Function to release the device file
static int chardev_release(struct inode *inode, struct file *filep) {
  pr_info("LED controller closed\n");
  return 0;
}

// Function to read the state of the LED
static ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
  pr_info("Led controller being read\n");
  led_controller_dev *led = (led_controller_dev *)filep->private_data;
  size_t state_len = strlen(led->state);

  if (*offset != 0) {
    return 0; // EOF
  }   

  if (len < state_len + 1) {
    return -EINVAL; // User buffer is too small
  }
    
  if (copy_to_user(buf, led->state, state_len + 1)) {
    return -EFAULT;
  }

  pr_info("Led controller done read\n");  
  *offset += state_len + 1;
  return state_len + 1;
}

// Function to write to the LED controller (turn on/off)
static ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
  pr_info("Led controller being written\n");
  led_controller_dev *led = (led_controller_dev *)filep->private_data;
  char user_buf[BUFF_LEN];

  if (len >= sizeof(user_buf)) {
    return -EINVAL; // Input is too large
  }
    
  if (copy_from_user(user_buf, buf, len)) {
    return -EFAULT;
  }

  user_buf[len] = '\0'; // Null-terminate the string

  if (strncmp(user_buf, "on", 2) == 0) {
    snprintf(led->state, sizeof(led->state), "on");
    gpio_set_value(led->gpio_pin, 1); // Turn LED on
    pr_info("LED turned on\n");
  } else if (strncmp(user_buf, "off", 3) == 0) {
    snprintf(led->state, sizeof(led->state), "off");
    gpio_set_value(led->gpio_pin, 0); // Turn LED off
    pr_info("LED turned off\n");
  } else {
    pr_info("Invalid command: %s\n", user_buf);
    return -EINVAL; // Invalid command
  }

  pr_info("Led controller done write\n");
  return len;
}

// Module initialization function
static int __init led_control_init(void) {
  int ret, i;
  dev_t dev;

  // Allocate major, minor number
  ret = alloc_chrdev_region(&dev, 0, 1, "leds controller");
  if (ret) {
    pr_info("Can not register major number.\n");
    return ret;
  }
  pr_info("Register successfully.\n");

  // Create class
  led_class = class_create(THIS_MODULE, LED_CLASS_NAME);

  if (IS_ERR(led_class)) {
    unregister_chrdev_region(dev, 1);
    pr_info("class_create failed!\n");
    return PTR_ERR(led_class);
  }
  pr_info("class_create successfully!\n");
  
  // Initialize devices
  for (i = 0; i < NUM_LEDS; i++) {
    led_cdev[i].dev = MKDEV(MAJOR(dev), MINOR(dev) + i);
    // Initialize GPIO
    ret = gpio_request(led_cdev[i].gpio_pin, "LED_GPIO");
    if (ret) {
      pr_err("Failed to request GPIO %d\n", led_cdev[i].gpio_pin);
      goto fail_gpio_request;
    }
    gpio_direction_output(led_cdev[i].gpio_pin, 0);

    // Initialize cdev
    cdev_init(&led_cdev[i].cdev, &fops);
    led_cdev[i].cdev.owner = THIS_MODULE;
    ret = cdev_add(&led_cdev[i].cdev, led_cdev[i].dev, 1);
    if (ret) {
      pr_err("Failed to add cdev for GPIO %d\n", led_cdev[i].gpio_pin);
      goto failed_cdev_add;
    }
    // Create device file
    led_cdev[i].device_name = device_create(led_class, NULL, led_cdev[i].dev, NULL, "led%d", i);
  }
  return 0;
  
failed_cdev_add:
  gpio_free(led_cdev[i].gpio_pin);
fail_gpio_request:
  while (--i >= 0) {
    device_destroy(led_class, led_cdev[i].dev);
    cdev_del(&led_cdev[i].cdev);
    gpio_free(led_cdev[i].gpio_pin);
  }
  class_destroy(led_class);
  unregister_chrdev_region(dev, NUM_LEDS);
  return ret;
}

static void __exit led_control_exit(void) {
  int i;
  for (i = 0; i < NUM_LEDS; i++) {
    gpio_set_value(led_cdev[i].gpio_pin, 0);
    gpio_free(led_cdev[i].gpio_pin);
    device_destroy(led_class, led_cdev[i].dev);
    cdev_del(&led_cdev[i].cdev);
  }
  class_destroy(led_class);
  unregister_chrdev_region(led_cdev[0].dev, NUM_LEDS);

  pr_info("Led controller driver unloaded\n");
}

module_init(led_control_init);
module_exit(led_control_exit);

MODULE_DESCRIPTION("LEDs Control Kernel Module");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");
