#include <linux/init.h>       
#include <linux/kernel.h>       // For kernel-level macros and functions (e.g., printk, pr_info)
#include <linux/module.h>       // For module macros (module_init, module_exit)
#include <linux/fs.h>           // For file operations structure
#include <linux/cdev.h>         // For character device structure
#include <linux/uaccess.h>      // For copy_to_user and copy_from_user
#include <linux/gpio.h>         // For GPIO manipulation

// Constants for LED control
#define LED_CONTROL_DEV_NAME      "sample_led"
#define LED_CLASS_NAME            "sample_led_class"

// GPIO pins for LEDs
#define RED_LED_GPIO_PIN          272
#define YELLOW_LED_GPIO_PIN       258
#define GREEN_LED_GPIO_PIN        268

// Buffer length for LED state
#define BUF_LEN                   5

// Structure for the LED controller device
typedef struct {
  struct class *class_name;    // Class for device
  struct device *device_name;  // Device name
  struct cdev my_cdev;         // Character device structure
  dev_t dev;                   // Device number
  char state[BUF_LEN];         // Buffer to store the current LED state
} led_controller_dev;

static led_controller_dev led_cdev = {
  .class_name = NULL,
  .device_name = NULL,
  .state = "off", // Default state
};

// Function prototypes for file operations
int chardev_open(struct inode *inode, struct file *filep);
int chardev_release(struct inode *inode, struct file *filep);
ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset);
ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset);

// File operations structure
static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = chardev_read,
  .write = chardev_write,
  .open = chardev_open,
  .release = chardev_release,
  .llseek = no_llseek,
};

// Open the device
int chardev_open(struct inode *inode, struct file *filep) {
  pr_info("Led controller opened\n");
  return 0;
}

// Close the device
int chardev_release(struct inode *inode, struct file *filep) {
  pr_info("Led controller closed\n");
  return 0;
}

// Read from the device
ssize_t chardev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
  pr_info("Led controller being read\n");
  size_t state_len;

  if (*offset != 0) {
    return 0; // EOF
  }   

  state_len = strlen(led_cdev.state);

  if (len < state_len + 1) {
    return -EINVAL; // Buffer too small
  }
    
  if (copy_to_user(buf, led_cdev.state, state_len + 1)) {
    return -EFAULT; // Error copying to user
  }
  pr_info("Led controller read done\n");
    
  *offset += state_len + 1;
  return state_len + 1;
}

// Write to the device
ssize_t chardev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
  pr_info("Led controller being written\n");
  char user_buf[BUF_LEN];

  if (len >= sizeof(user_buf)) {
    return -EINVAL; // Input is too large
  }
    
  if (copy_from_user(user_buf, buf, len)) {
    return -EFAULT; // Error copying from user
  }

  user_buf[len] = '\0'; // Null-terminate the string

  // Handle commands
  if (strncmp(user_buf, "on", 2) == 0) {
    snprintf(led_cdev.state, sizeof(led_cdev.state), "on");
    gpio_set_value(RED_LED_GPIO_PIN, 1); // Turn LED on
    pr_info("LED turned on\n");
  } else if (strncmp(user_buf, "off", 3) == 0) {
    snprintf(led_cdev.state, sizeof(led_cdev.state), "off");
    gpio_set_value(RED_LED_GPIO_PIN, 0); // Turn LED off
    pr_info("LED turned off\n");
  } else {
    pr_info("Invalid command: %s\n", user_buf);
    return -EINVAL; // Invalid command
  }

  pr_info("Led controller write done\n");
  return len;
}

// Initialize the LED controller
static int __init led_control_init(void) {
  int ret;
  snprintf(led_cdev.state, sizeof(led_cdev.state), "off");

  // Request GPIO pin
  ret = gpio_request(RED_LED_GPIO_PIN, "LED_GPIO");
  if (ret) {
    pr_err("Failed to request GPIO pin\n");
    return ret;
  }

  // Set as output and turn off the LED initially
  gpio_direction_output(RED_LED_GPIO_PIN, 0); 
  pr_info("LED GPIO configured successfully\n");

  // Allocate character device region
  ret = alloc_chrdev_region(&led_cdev.dev, 0, 1, "led controller");
  if (ret) {
    gpio_set_value(RED_LED_GPIO_PIN, 0); // Turn off the LED
    gpio_free(RED_LED_GPIO_PIN);         // Release the GPIO pin
    pr_info("Can not register major number.\n");
    return ret;
  }
  pr_info("Register successfully, major number is %d.\n", MAJOR(led_cdev.dev));
  pr_info("Register successfully, minor number is %d.\n", MINOR(led_cdev.dev));

  // Initialize character device
  cdev_init(&led_cdev.my_cdev, &fops);
  led_cdev.my_cdev.owner = THIS_MODULE;
  led_cdev.my_cdev.dev = led_cdev.dev;

  ret = cdev_add(&led_cdev.my_cdev, led_cdev.dev, 1);
  if (ret < 0) {
    gpio_set_value(RED_LED_GPIO_PIN, 0);  // Turn off the LED
    gpio_free(RED_LED_GPIO_PIN);          // Release the GPIO pin
    unregister_chrdev_region(led_cdev.dev, 1);  // Unregister the character device region
    pr_info("cdev_add error!\n");
    return ret;
  }

  // Create class
  led_cdev.class_name = class_create(THIS_MODULE, LED_CLASS_NAME);
  if (IS_ERR(led_cdev.class_name)) {
    gpio_set_value(RED_LED_GPIO_PIN, 0);  // Turn off the LED
    gpio_free(RED_LED_GPIO_PIN);          // Release the GPIO pin
    cdev_del(&led_cdev.my_cdev);          // Remove the character device
    unregister_chrdev_region(led_cdev.dev, 1);  // Unregister the character device region
    pr_info("class_create failed!\n");
    return PTR_ERR(led_cdev.class_name);
  }
  pr_info("class_create successfully!\n");

  // Create device
  led_cdev.device_name = device_create(led_cdev.class_name, NULL, led_cdev.dev, NULL, LED_CONTROL_DEV_NAME);
  if (IS_ERR(led_cdev.device_name)) {
    gpio_set_value(RED_LED_GPIO_PIN, 0);  // Turn off the LED
    gpio_free(RED_LED_GPIO_PIN);          // Release the GPIO pin
    cdev_del(&led_cdev.my_cdev);          // Remove the character device
    class_destroy(led_cdev.class_name);         // Destroy the device class
    unregister_chrdev_region(led_cdev.dev, 1);  // Unregister the character device region
    pr_info("device_create failed!\n");
    return PTR_ERR(led_cdev.device_name);
  }

  pr_info("Led controller driver loaded\n");

  return 0;
}

static void __exit led_control_exit(void) {
  gpio_set_value(RED_LED_GPIO_PIN, 0);  // Turn off the LED
  gpio_free(RED_LED_GPIO_PIN);          // Release the GPIO pin
  cdev_del(&led_cdev.my_cdev);          // Remove the character device
  device_destroy(led_cdev.class_name, led_cdev.dev);  // Remove the device from the sysfs and free associated memory.
  class_destroy(led_cdev.class_name);         // Destroy the device class
  unregister_chrdev_region(led_cdev.dev, 1);  // Unregister the character device region

  pr_info("Led controller driver unloaded\n");
}

module_init(led_control_init);
module_exit(led_control_exit);

MODULE_DESCRIPTION("LED Control Kernel Module");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");