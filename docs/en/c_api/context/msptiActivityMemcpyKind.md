# msptiActivityMemcpyKind<a name="ZH-CN_TOPIC_0000002120344714"></a>

Memory copy type. msptiActivityMemcpyKind is an enumeration class called by [msptiActivityMemcpy](msptiActivityMemcpy.md), defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMCPY_KIND_UNKNOWN = 0,    // Reserved internally, undefined
    MSPTI_ACTIVITY_MEMCPY_KIND_HOST = 1,    // Host-to-Host memory copy type
    MSPTI_ACTIVITY_MEMCPY_KIND_HTOD = 2,    // Host-to-Device memory copy type
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOH = 3,    // Memory copy type from Device to Host
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOD = 4,    // Memory copy type from Device to Device
    MSPTI_ACTIVITY_MEMCPY_KIND_DEFAULT = 5    // Memory copy type from device memory to device memory on the same Device
} msptiActivityMemcpyKind;
```
