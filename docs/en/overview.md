# Overview

MindStudio Profiler Tools Interface (msPTI) is a set of profiling APIs provided by MindStudio for Ascend devices. You can use msPTI to build tools for NPU applications to analyze the performance of the applications.

msPTI is a universal API. Profiling tools developed using msPTI APIs can be used in inference and training scenarios of various frameworks.

msPTI provides the following functions:

- Tracing: In msPTI, tracing refers to the collection of timestamps and additional information about the execution and startup of CANN activities in CANN applications, such as CANN APIs, kernels, and memory copy. Identify performance issues of CANN by understanding the program running duration. You can use the Activity APIs and Callback APIs to collect tracing information.
- Profiling: In msPTI, profiling refers to the collection of NPU performance metrics of one or a group of kernels.
