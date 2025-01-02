#include <linux/init.h>       
#include <linux/kernel.h>       // For kernel-level macros and functions (e.g., printk, pr_info)
#include <linux/module.h>       // For module macros (module_init, module_exit)
#include <linux/fs.h>           // For file operations structure
#include <linux/cdev.h>         // For character device structure
#include <linux/uaccess.h>      // For copy_to_user and copy_from_user

#include <linux/of.h>               // For device tree (used for devices in embedded systems)
#include <linux/platform_device.h>  // For platform device support (devices not on a standard bus like PCI)

#define CLASS_NAME          "manhtd24_device_class"   // Name of the device class
#define DEVICE_NAME_PREFIX  "manhtd24_"               // Prefix for device names (used for device naming)
#define BUF_LEN             128                       // Buffer length for state information

static int i = 0;                              
static struct class* manhtd24_class = NULL;    // Struct device class
static dev_t devt;                             // Device number for the character device

// Structure representing an manhtd24 device
typedef struct {
  struct device *device;              // Device structure for sysfs
  struct cdev cdev;                   // Character device structure
  dev_t dev;                          // Device number
  char name[64];                      // Device name including prefix and serial number
  char state[BUF_LEN];                // State of the device, stored as a string (max length BUF_LEN)
} manhtd24_dev;

// Structure to store device data
typedef struct  {
  char label[64];       // Device label
  char serial[64];      // Device serial number
  u64 reg_base;         // Register base
  u64 reg_size;         // Register base
  u32 size;             // Device size
  char permission[64];  // Device permission
} manhtd24_device_data;

// Function prototypes for file operations
static int manhtd24dev_open(struct inode *inode, struct file *filep);
static int manhtd24dev_release(struct inode *inode, struct file *filep);
static ssize_t manhtd24dev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset);
static ssize_t manhtd24dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset);

// File operations for the manhtd24 device
static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = manhtd24dev_read,
  .write = manhtd24dev_write,
  .open = manhtd24dev_open,
  .release = manhtd24dev_release,
  .llseek = no_llseek,
};

// Function to open the device file
static int manhtd24dev_open(struct inode *inode, struct file *filep) {
  // Get the LED device structure associated with this inode
  manhtd24_dev *mdevice = container_of(inode->i_cdev, manhtd24_dev, cdev);
  filep->private_data = mdevice; // Store the LED device in file's private data
  pr_info("Opened device %s\n", mdevice->name);
  return 0;
}

// Function to read the device file
static ssize_t manhtd24dev_read(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
  manhtd24_dev *mdevice = (manhtd24_dev *)filep->private_data;
  manhtd24_device_data *data = mdevice->device->platform_data;
  char kernel_buffer[1024]; // Kernel buffer to hold formatted data
  size_t format_len;

  if (*offset != 0) {
    return 0; // EOF
  }   

  if (len < sizeof(kernel_buffer)) {
    return -EINVAL; // User buffer is too small
  }

  // Format the device data into the kernel buffer
  format_len = snprintf(kernel_buffer, sizeof(kernel_buffer),
                        "Device Information:\n"
                        "Label       : %s\n"
                        "Serial      : %s\n"
                        "Reg Base    : 0x%016llx\n"
                        "Reg Size    : 0x%016llx\n"
                        "Size        : %u bytes\n"
                        "Permission  : %s\n\n"
                        "State       : %s\n",
                        data->label,
                        data->serial,
                        data->reg_base,
                        data->reg_size,
                        data->size,
                        data->permission,
                        mdevice->state);
  
  // If copying to user space fails, return error
  if (copy_to_user(buf, kernel_buffer, format_len + 1)) {
    return -EFAULT;
  }

  pr_info("Device %s done read\n", mdevice->name);  
  *offset += format_len + 1;
  return format_len + 1;
}

// Function to write to device file
static ssize_t manhtd24dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
  manhtd24_dev *mdevice = (manhtd24_dev *)filep->private_data;
  manhtd24_device_data *device_data = mdevice->device->platform_data;
  if (strcmp(device_data->permission, "ro") == 0) {
    pr_info("Device is read-only!\n");
    return 1;
  }

  char user_buf[BUF_LEN];

  if (len >= sizeof(user_buf)) {
    return -EINVAL; // Input is too large
  }
    
  if (copy_from_user(user_buf, buf, len)) {
    return -EFAULT;
  }

  user_buf[len] = '\0'; // Null-terminate the string
  
  if (strncmp(user_buf, "idle", 2) == 0) {
    strcpy(mdevice->state, user_buf);
    // Do somthings
    pr_info("switch to idle\n");
  } else if (strncmp(user_buf, "running", 3) == 0) {
    strcpy(mdevice->state, user_buf);
    // Do somthings
    pr_info("switch to running\n");
  } else {
    pr_info("Invalid command: %s\n", user_buf);
    return -EINVAL; // Invalid command
  }

  pr_info("Device %s done write\n", mdevice->name);  
  return len;
}

// Function to release the device file
static int manhtd24dev_release(struct inode *inode, struct file *filep) {
  manhtd24_dev *mdevice = (manhtd24_dev *)filep->private_data;
  pr_info("Device %s closed\n", mdevice->name);
  return 0;
}

// Device tree matching table for the manhtd24 devices
static const struct of_device_id manhtd24_match[] = {
  { .compatible = "manhtd24,device1" },
  { .compatible = "manhtd24,device2" },
  { .compatible = "manhtd24,device3" },
  { .compatible = "manhtd24,device4" },
  { .compatible = "manhtd24,device5" },
  {},
};

// Register the device table with the kernel to indicate supported devices
MODULE_DEVICE_TABLE(of, manhtd24_match);

// Probe function to read and display device properties and create device
static int my_device_probe(struct platform_device *pdev) {
  struct device *dev = &pdev->dev;               // Pointer to the device structure
  struct device_node *node = dev->of_node;       // Pointer to the device node (representing the hardware device)

  const char *label, *serial, *permission;       // Variables to store device properties
  u64 reg_base, reg_size;                        // Variables for base address and size of registers
  u32 size;                                      // Variable for device size
  manhtd24_dev *mdevice;                         // Pointer to the device structure for driver
  manhtd24_device_data *mdata;                   // Pointer to device-specific data structure
  int ret;

  if (!node) {
    dev_err(dev, "No device node found\n");
    return -EINVAL;
  }

  // Allocate memory for the device structure
  mdevice = kzalloc(sizeof(manhtd24_dev), GFP_KERNEL);
  if (!mdevice) {
    dev_err(dev, "Failed to allocate memory for device structure\n");
    return -ENOMEM;
  }

  // Allocate memory for the device data structure
  mdata = kzalloc(sizeof(manhtd24_device_data), GFP_KERNEL);
  if (!mdata) {
    dev_err(dev, "Failed to allocate memory for device data\n");
    kfree(mdevice);
    return -ENOMEM;
  }

  // Read the "reg" property (base address and size)
  if (of_property_read_u64_index(node, "reg", 0, &reg_base) == 0 &&
    of_property_read_u64_index(node, "reg", 1, &reg_size) == 0) {
    mdata->reg_base = reg_base;
    mdata->reg_size = reg_size;
    dev_info(dev, "Register base: 0x%llx, size: 0x%llx\n", reg_base, reg_size);
  } 

  // Read the "label" property
  if (of_property_read_string(node, "label", &label) == 0) {
    strcpy(mdata->label, label);
    dev_info(dev, "Label: %s\n", label);
  }

  // Read the "size" property
  if (of_property_read_u32(node, "size", &size) == 0) {
    mdata->size = size;
    dev_info(dev, "Device size: %u bytes\n", size);
  }

  // Read the "serial" property
  if (of_property_read_string(node, "serial", &serial) == 0) {
    strcpy(mdata->serial, serial);
    snprintf(mdevice->name, sizeof(mdevice->name), "manhtd24_%s", mdata->serial);
    dev_info(dev, "Device serial number: %s\n", serial);
  }

  // Read the "permission" property
  if (of_property_read_string(node, "permission", &permission) == 0) {
    strcpy(mdata->permission, permission);
    dev_info(dev, "Permission: %s\n", permission);
  }

  // Initialize device
  mdevice->dev = MKDEV(MAJOR(devt), MINOR(devt) + i++);
  // Initialize cdev
  cdev_init(&mdevice->cdev, &fops);
  mdevice->cdev.owner = THIS_MODULE;
  ret = cdev_add(&mdevice->cdev, mdevice->dev, 1);
  if (ret) {
    pr_err("Failed to add cdev for Device %s\n", mdevice->name);
    kfree(mdata);
    kfree(mdevice);
    return ret;
  }

  // Set initial state to idle
  strcpy(mdevice->state, "idle");

  // Create device file
  mdevice->device = device_create(manhtd24_class, NULL, mdevice->dev, NULL, mdevice->name);
  if (IS_ERR(mdevice->device)) {
    pr_err("Failed to create device file for %s\n", mdevice->name);
    kfree(mdata);
    kfree(mdevice);
    return IS_ERR(mdevice->device);
  }
  mdevice->device->platform_data = mdata;  // Attach the platform data to the device
  dev->platform_data = mdevice;            
  dev_info(dev, "Device successfully matched and initialized\n");
  
  return 0;
}

// Remove function
static int my_device_remove(struct platform_device *pdev)
{
  manhtd24_dev *mdevice = dev_get_platdata(&pdev->dev);  // Retrieve the device structure
  if (mdevice) {
    pr_info("Resources for device %s cleaning up . . .\n", mdevice->name);
    // Remove the character device
    cdev_del(&mdevice->cdev);

    // Destroy the device file
    device_destroy(manhtd24_class, mdevice->dev);

    // Free the allocated memory for device data and the device structure
    kfree(mdevice->device->platform_data);  // Free device data
    kfree(mdevice);                         // Free device structure
  }
  dev_info(&pdev->dev, "Device removed\n");
  return 0;
}

// Platform driver structure
static struct platform_driver manhtd24_driver = {
  .driver = {
    .name = "manhtd24_device_driver",
    .of_match_table = manhtd24_match,
  },
  .probe = my_device_probe,
  .remove = my_device_remove,
};

// Module initialization
static int __init manhtd24_driver_init(void) {
  int ret;

  // Allocate a major number for the character device
  ret = alloc_chrdev_region(&devt, 0, 1, "manhtd24_device");
  if (ret) {
    pr_info("Can not register major number.\n");
    return ret;
  }
  pr_info("Register successfully.\n");

  // Create device class
  manhtd24_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(manhtd24_class)) {
    unregister_chrdev_region(devt, 1);
    pr_err("Failed to create class\n");
    return PTR_ERR(manhtd24_class);
  }

  // Register the platform driver
  ret = platform_driver_register(&manhtd24_driver);
  if (ret) {
    class_destroy(manhtd24_class);
    unregister_chrdev_region(devt, 1);
    pr_err("Failed to register driver\n");
    return ret;
  }

  pr_info("manhtd24 driver initialized\n");
  return 0;
}

// Module exit
static void __exit manhtd24_driver_exit(void) {
  platform_driver_unregister(&manhtd24_driver);
  class_destroy(manhtd24_class);
  unregister_chrdev_region(devt, i);
  pr_info("manhtd24 driver exited\n");
}

module_init(manhtd24_driver_init);
module_exit(manhtd24_driver_exit);

MODULE_DESCRIPTION("ManhTD24 device driver");
MODULE_AUTHOR("ManhTD24");
MODULE_LICENSE("GPL");
