sycl-info(1) -- Using sycl-info
===============================================================================

## SYNOPSIS

`sycl-info` [--help] [--verbose] [--all] [--device-cflags] [--impl <impl>]
  [--config <platform>:<device>] [--target <backend>] [--hint <additional_dir>]

## DESCRIPTION

sycl-info is a tool that can print metadata about available SYCL implementations
and provide the SYCL device compiler flags required to build a valid SYCL
executable.

It works by parsing a set of vendor supplied `.syclinfo` files and matching them
with the user-supplied query.

## OPTIONS

  * `-h`, `--help`:
    Displays all options that sycl-info supports.

  * `-v`, `--verbose`:
    Displays additional details for other options.
    
  * `--all`:
    Displays all platform/device configurations, even those unsupported.
    Requires `--impl` to be specified.

  * `--device-cflags`:
    Displays the recommended SYCL device compiler flags for a specific
    configuration.
    Requires `--impl` and `--config` to be specified.
    Optionally `--target` can also be specified.

  * `--impl <impl>`: 
    Selects a SYCL implementations and displays the supported platform/device
    configurations.
    
  * `--config <platform>:<device>`:
    Selects a SYCL platform/device configuration.
    Requires `--impl` to be specified.

  * `--target <backend>`:
    Selects a SYCL backend.
    Requires `--impl` to be specified.

  * `--hint <additional_dir>`:
    Provides an additional path to look for .syclinfo files.

## ENVIRONMENT

  * SYCL_VENDOR_PATHS:
    sycl-info will search the directory of all paths in `$SYCL_VENDOR_PATHS`
    looking for a `.syclinfo` file and display an indexed list (starting at 1)
    of SYCL implementations, one for each `.syclinfo` file it finds specifying
    the name, vendor and version of each. This will be the default behavior
    when no options are passed to sycl-info.
    
## EXAMPLES

Calling sycl-info will display the available SYCL implementations. It will do
this by looking for .syclinfo files located in $ENV{SYCL_VENDOR_PATHS} and the
path provided by `--hint` if specified.

```
$ sycl-info
SYCL implementation(s) available:
---------------------------------
1. <impl_1_name> | <impl_1_version> | <impl_1_vendor>
...
N. <impl_N_name> | <impl_N_version> | <impl_N_vendor>
```

Once you know which implementations are available you can query which
device/platform configurations are available and supported by a specific SYCL
implementation using `--impl`. Note that this will only display device/platform
configurations that are stated as supported in a .syclinfo file, you can display
all available device/platform configurations, regrardless of whether the SYCL
implementation supports them with `--all`.

```
$ sycl-info --impl 1
Supported platform/device configuration(s) available:
---------------------------------
1. <platform_1_vendor> : <platform_1_name>:
  1. (<device_N_type>) <device_1_vendor> : <device_1_name> (<device_1_supported_back_end_targets>) [device_1_support_status]
  ...
  N. (<device_N_type>) <device_N_vendor> : <device_N_name> (<device_N_supported_back_end_targets>) [device_N_support_status]
...
N. <platform_N_vendor> : <platform_N_name>:
  1. (<device_N_type>) <device_1_vendor> : <device_1_name> (<device_1_supported_back_end_targets>) [device_1_support_status]
  ...
  N. (<device_N_type>) <device_N_vendor> : <device_N_name> (<device_N_supported_back_end_targets>) [device_N_support_status]
```

Once you know which device/platform configurations are supported by a SYCL
implementation, sycl-info can provide further information about them.

One such query that is currently available is printing the recommended SYCL
device compiler flags for a SYCL implementation and device/platform
configuration using `--device-cflags`. When using `--device-cflags` you can
specify the configuration you are interested in using `--config`. You can also
optionally specify the back-end target using `--target`.

```
$ sycl-info --impl 1 --config 1:1 --target SPIRV --device-cflags
Supported platform/device configuration(s) available:
---------------------------------
Backend: <device_1_supported_back_end_target>
Device flags: <device_1_device_flags_spir>
```

## EXIT STATUS

  * 0:
    If run successfully

  * 1:
    If no implementations could be found

## TROUBLESHOOTING

### sycl-info is not displaying any device/platform configurations

When you call sycl-info with `--impl` it will filter the available
device/platform configurations against those listed as supported configurations
in the .syclinfo file. If you don't see the platform(s) or device(s) you expect
to it may be due to variance in platform or device names, you can use the
`--all` option, to list every platform and device regardless of whether they
match the configurations in the .syclinfo file.

## SYCLINFO FILE JSON SCHEMA

SYCL info uses the folling JSON schema for interpreting .syclinfo files.

### Version 0.1

```
{
  "version" : "<sycl_info_version>",
  “implementation”: {
    “name” : “<sycl_implementation_name”,
    “vendor” : “sycl_implementation_vendor”,
    “version” : “sycl_implementation_version”,
    “supported_configurations”: [
      {
        “sycl_version” : “configuration_1_sycl_version”,
        “os”: “configuration_1_os”,
        "backend" : "configuration_1_backend",
        “platform_name”: “configuration_1_platform_name”,
        “device_type” : “configuration_1_device_type”,
        “device_name” : “configuration_1_device_name”,
        "supported_backend_targets" : [ { "backend_target" : "<configuration_1_backend_target_1>", "device_flags" : "<configuration_1_backend_target_1_device_flags>" }, ...,
                                        { "backend_target" : "<configuration_1_backend_target_N>", "device_flags" : "<configuration_1_backend_target_N_device_flags>" }  {} ],
        “supported_driver_versions”: [ "configuration_1_version_1", ..., "configuration_1_version_N" ]
      },
      ...,      {
        “sycl_version” : “configuration_N_sycl_version”,
        “os”: “configuration_N_os”,
        "backend" : "configuration_N_backend",
        “platform_name”: “configuration_N_platform_name”,
        “device_type” : “configuration_N_device_type”,
        “device_name” : “configuration_N_device_name”,
        "supported_backend_targets" : [ { "backend_target" : "<configuration_N_backend_target_1>", "device_flags" : "<configuration_N_backend_target_1_device_flags>" }, ...,
                                        { "backend_target" : "<configuration_N_backend_target_N>", "device_flags" : "<configuration_N_backend_target_N_device_flags>" }  {} ],
        “supported_driver_versions”: [ "configuration_N_version_1", ..., "configuration_N_version_N" ]
      }
    ]
  }
}
```
See [here](../../samples/example.syclinfo) for an example .syclinfo file.


## SUPPORTING

SYCL 1.2.1.

## BUGS

Please report any issues on the GitHub project page
(https://github.com/codeplaysoftware/sycl-info)

## HISTORY

 * 2019-11-18 v0.1:
    Initial Release.

## AUTHOR

  * Codeplay Software Ltd. <sycl@codeplay.com>

## COPYRIGHT

Copyright Codeplay Software, Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## SEE ALSO

clinfo(1), vulkaninfo(1)
