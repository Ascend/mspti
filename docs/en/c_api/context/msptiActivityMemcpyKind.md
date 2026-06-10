# msptiActivityMemcpyKind<a name="ZH-CN_TOPIC_0000002120344714"></a>

Memory copy type. msptiActivityMemcpyKind is an enumeration class invoked by [msptiActivityMemcpy](msptiActivityMemcpy.md). It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMCPY_KIND_UNKNOWN = 0, // Reserved for internal use. Undefined.
    MSPTI_ACTIVITY_MEMCPY_KIND_HOST = 1, // Memory copy from host to host.
    MSPTI_ACTIVITY_MEMCPY_KIND_HTOD = 2, // Memory copy from host to device.
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOH = 3, // Memory copy from device to host.
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOD = 4, // Memory copy from device to device.
    MSPTI_ACTIVITY_MEMCPY_KIND_DEFAULT = 5 // Memory copy from device memory to device memory on the same device.
} msptiActivityMemcpyKind;
```
