# Usage
* Load module:
  ```
  insmod test_module.ko
  ```

* Check current parmeters:
  ```
  sudo cat /sys/module/test_module/parameters/int_param
  sudo cat /sys/module/test_module/parameters/string_param
  ```

* Change parameters:
  ```
  echo "Updated string" | sudo tee /sys/module/test_module/parameters/string_param

  echo 150 | sudo tee /sys/module/test_module/parameters/int_param
  ```

# Explain
* 0660 allows user to read/ write parameter in /sys/module/`module_name`/parameters/.
  ```C
  module_param(int_param, int, 0660);    
  module_param(string_param, charp, 0660);
  ```