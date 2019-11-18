Target Selector is a tool that enables querying the target's OpenCL platform.
It offers the following functionality: 
* It can query the current system and get the available platforms/devices in
  a vector of pairs `<cl_platform_id, cl_device_id>`

* For each of the available platforms, the tool can query each device and return
  it as a vector of characters 

* It offers a utility function that is able to query a specific device

* It provides a handful of utility error macros
