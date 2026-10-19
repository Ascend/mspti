# `msptiActivityObjectKind`<a name="ZH-CN_TOPIC_0000002155584861"></a>

Activity object type. msptiActivityObjectKind is an enumeration class invoked by [msptiActivityOverhead](./msptiActivityOverhead.md). It identifies the type of object associated with an Activity record and indicates which fields in `msptiObjectId` are valid. The definition is as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_OBJECT_UNKNOWN = 0,    // Unknown object type
    MSPTI_ACTIVITY_OBJECT_PROCESS = 1,    // The object is a process, and the pt member of msptiObjectId is valid
    MSPTI_ACTIVITY_OBJECT_THREAD = 2,    // The object is a thread, and the pt member of msptiObjectId is valid
    MSPTI_ACTIVITY_OBJECT_DEVICE = 3,    // The object is a device, and the ds member of msptiObjectId is valid
    MSPTI_ACTIVITY_OBJECT_FORCE_INT = 0x7fffffff
} msptiActivityObjectKind;
```
