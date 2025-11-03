/*
 * #include "memfault/core/platform/device_info.h"

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  // !FIXME: Populate with platform device information
  //
  // *NOTE* All fields must be populated, and the values assigned to the fields
  // must have static lifetime: the data is accessed when this function returns.
  // In this example, the fields are string literals, which are placed either
  // inline into .text data tables, or in .rodata, and the pointers are valid
  // for the lifetime of the program
  //
  // See https://mflt.io/version-nomenclature for more context
  *info = (sMemfaultDeviceInfo) {
    // Set the device serial to a unique value.
    // It is typically set to a unique identifier like a serial number
    // or MAC address.
    // This is used to de-deduplicate data in Memfault cloud
    .device_serial = "DEMOSERIAL",
    // Set the device software type.
    // It can be simply "app" for a single-chip device, otherwise it
    // should match the component name, eg "ble", "sensor" etc.
    // This is used to filter devices in the Memfault UI
    .software_type = "app-main",
    // Set the device software version.
    // If using Memfault OTA, this should exactly match the OTA Release
    // Version name for the installed image
    .software_version = "1.0.0-dev",
    // Set the device hardware revision.
    // This is used to filter/group devices in the Memfault UI
    .hardware_version = "evt",
  };
}
*/
