# msPTI Feature Design Specifications

<table>
    <tr>
        <td>SIG group:</td>
        <td>mstt-sig</td>
    </tr>
    <tr>
        <td>Target Version:</td>
        <td>MindStudio 26.0.0</td>
    </tr>
    <tr>
        <td>Designer:</td>
        <td>chenhao</td>
    </tr>
    <tr>
        <td>Date:</td>
        <td>2026.01.21</td>
    </tr>
</table>

**Copyright © 2022 openGauss Community**

Your replication, use, modification, and distribution of this document are governed by the Creative Commons Attribution-ShareAlike 4.0 International Public License (CC BY-SA 4.0).
You can visit [https://creativecommons.org/licenses/by-sa/4.0/](https://creativecommons.org/licenses/by-sa/4.0/) to view a summary of (not a substitute for) CC BY-SA 4.0.
For the complete CC BY-SA 4.0, visit [https://creativecommons.org/licenses/by-sa/4.0/legalcode](https://creativecommons.org/licenses/by-sa/4.0/legalcode).

**Change History**

<table>
    <tr>
        <th>Date</th>
        <th>Version</th>
        <th>Description</th>
        <th>Author</th>
        <th>Reviewer</th>
    </tr>
    <tr>
        <td>2026.01.21</td>
        <td>1.0</td>
        <td>Initial draft.</td>
        <td>chenhao</td>
        <td></td>
    </tr>
</table>

<!-- TOC -->
* [1 Overview](#1-Overview)
  * [1.1 Scope](#11-Scope)
  * [1.2 Feature Requirements](#12-Feature-Requirements)
* [2 Requirement Scenario Analysis](#2-Requirement-Scenario-Analysis)
  * [2.1 Requirement Origin and Benefits](#21-Requirement-Origin-and-Benefits)
  * [2.2 Feature Scenario Analysis](#22-Feature-Scenario-Analysis)
  * [2.3 Feature Impact Analysis](#23-Feature-Impact-Analysis)
    * [2.3.1 Hardware Restrictions](#231-Hardware-Restrictions)
    * [2.3.2 Technical Restrictions](#232-Technical-Restrictions)
* [3 Feature/Function Implementation Principles (Decomposable into Multiple Use Cases)](#3-Feature/Function-Implementation-Principles-(Decomposable-into-Multiple-Use-Cases))
  * [3.1 Objectives](#31-Objectives)
  * [3.2 Overall Solution](#32-Overall-Solution)
* [4. Supporting the Collection Capability of the CANN Runtime API](#4 Supporting the Collection Capability of the CANN Runtime API)
  * [4.1 Design Rationale](#41-Design-Rationale)
  * [4.2 Constraints](#42-Constraints)
  * [4.3 Detailed Implementation (Module- or Process-Level Message Sequence Diagrams from the User Entry)](#43 Detailed Implementation (Module--or-Process-Level-Message-Sequence-Diagrams-from-the-User-Entry))
  * [4.4 Inter-Subsystem Interfaces (Mainly Module Interface Definitions)](#44-Inter-Subsystem-Interfaces-(Mainly-Module-Interface-Definitions))
  * [4.5 Detailed Subsystem Design](#45-Detailed-Subsystem-Design)
  * [4.6 DFX Attribute Design](#46-DFX-Attribute-Design)
    * [4.6.1 Performance Design](#461-Performance-Design)
    * [4.6.2 Upgrade and Capacity Expansion Design](#462-Upgrade-and-Capacity-Expansion-Design)
    * [4.6.3 Exception Handling Design](#463-Exception-Handling-Design)
    * [4.6.4 Resource Management Design](#464-Resource-Management-Design)
    * [ 4.6.5 Compact Design](#465-Compact-Design)
    * [4.6.7 Security Design](#467-Security-Design)
      * [4.6.7.1 Security Design Confirmation](#4671-Security-Design-Confirmation)
      * [4.6.7.2 Sensitive Data Analysis](#4672-Sensitive-Data-Analysis)
        * [1. Sensitive Data List](#1-Sensitive-Data-List)
        * [2. Sensitive Operation Check](#2-Sensitive-Operation-Check)
      * [4.6.7.3 Design Implementation](#4673-Design-Implementation)
  * [4.7 External Interfaces](#47-External-Interfaces)
* [5. (Optional) Data Structure Design](#5 Data Structure Design)
<!-- TOC -->

# 1. Feature Overview

The MindStudio Profiling Tools Interface (msPTI) is a set of profiling APIs provided by MindStudio for Ascend devices. You can use msPTI to build tools for NPU applications to analyze the performance of the applications.
msPTI is a universal API. Profiling tools developed using msPTI APIs can be used in inference and training scenarios of various frameworks.

## 1.1 Scope

msPTI provides the following functions:

- Tracing: In msPTI, tracing refers to the collection of timestamps and additional information about the execution and startup of CANN activities in CANN applications, such as CANN APIs, kernels, and memory copy. Identify performance issues of CANN by understanding the program running duration. You can use the Activity APIs and Callback APIs to collect tracing information.

- Profiling: In msPTI, profiling refers to the collection of NPU performance metrics of one or a group of kernels.

## 1.2 Feature Requirement List

Table 1 List of feature requirements

<table>
    <tr>
        <th>Requirement No.</th>
        <th>Requirement</th>
        <th>Feature Description</th>
        <th>Remarks</th>
    </tr>
    <tr>
        <td>1</td>
        <td>Supports the collection capability of CANN Runtime APIs.</td>
        <td>Collects statistics on API calls and time consumption at the runtime level.</td>
        <td></td>  
    </tr>
</table>

# 2. Requirement Scenario Analysis

## 2.1 Requirement Origin and Benefits

The external API collection capability of msPTI is extended to support the collection capability of CANN Runtime APIs.

## 2.2 Feature Scenario Analysis

Scenario triggering condition: For host-bound service processes, the factors that affect the CANN API performance analysis need to be analyzed.

The user needs to analyze the API performance on the CANN Runtime side, and obtain and analyze the API time consumption.

## 2.3 Feature Impact Analysis

### 2.3.1 Hardware Restrictions

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training series products                         |    ×     |

### 2.3.2 Technical Restrictions

Operating system: Linux

Programming language: C/Python

# 3. Feature/Function Implementation Principles (Decomposable into Multiple Use Cases)

## 3.1 Objectives

_Describe the specifications and objectives of the feature in different scenarios._

## 3.2 Overall Solution

Profiling scenario: The system measures the time consumed by key functions, key computing, and communication, collects data, flushes the data to disks, and performs offline parsing and visualized display.

![image_1](image_1.png)

Figure 1: Overall implementation principle of the msPTI solution

# 4. Supports the collection capability of CANN Runtime APIs.

## 4.1 Design Rationale

The msPTI enables the runtime API data collection capability through the msptiActivityEnable(msptiActivityKind kind) API and obtains the runtime API data through the callback API.

## 4.2 Constraints

No constraint

## 4.3 Detailed Implementation (Module- or Process-Level Message Sequence Diagrams from the User Entry)

_Describe the implementation process of the use case in detail. Use timing diagrams and flowcharts to illustrate the interaction between modules._

_Use brief words to describe the changes of module allocation requirements in the sequence diagrams and flowcharts. Structured language is preferred._

## 4.4 Inter-Subsystem Interfaces (Mainly Module Interface Definitions)

Extended interface capability: msptiActivityEnable(msptiActivityKind kind)

## 4.5 Detailed Subsystem Design

The msPTI enables the runtime API data collection capability through the msptiActivityEnable(msptiActivityKind kind) API and obtains the runtime API data through the callback API.

## 4.6 DFX Attribute Design

### 4.6.1 Performance Design

The impact on performance is limited. Only the enabling and obtaining of the API type are performed. The number of runtime APIs is controllable.

### 4.6.2 Upgrade and Capacity Expansion Design

Upgrade and capacity expansion scenarios are not involved.

### 4.6.3 Exception Handling Design

_Describe the potential exception scenarios, whether workarounds exist, how users are notified, and how to minimize impact on user services._

### 4.6.4 Resource Management Design

The enabled runtime API uses the memory transferred by the user. The resource specifications can be maintained by the user's normal consumption. When the memory is used in the critical condition, the user can control the size of the requested memory.

The msPTI data is managed in the memory in a unified manner. The disk I/O is a post-processing procedure after the user consumes the data. Therefore, the consumption frequency can be considered.

### 4.6.5 Compact Design

There is no version design related to miniaturization. There is no impact.

### 4.6.7 Security Design

#### 4.6.7.1 Security Design Confirmation

*Confirm the security design by referring to the security design checklist.*

| Security Attribute    | Check Item                                                      | Description                                              | Involved or Not| Satisfied or Not|
| ------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |------|-----|
| Access channel control| Whether any listening port is added                                            | If any listening port is added, update the communication matrix.                                  | No   |     |
| Access channel control| Whether any process or inter-component communication method is added                                    | If any process or inter-component communication method is added, update the communication matrix.                            | No   |     |
| Access channel control| Whether any authentication mode is added                                            | If any authentication mode is added, update the communication matrix and product documentation.                        | No   |     |
| Permission control    | Whether any file or directory needs to be created                                      | If any file or directory needs to be created, explicitly specify the access permission of the file or directory.                | No   |     |
| Permission control    | Whether the account permission meets the principle of least privilege                            | Assign the minimum permissions to each account in the system.                                  | No   |     |
| Permission control    | Whether user privilege escalation exists                                        | User privilege escalation is prohibited.                                    | No   |     |
| Undisclosed interface  | Whether any grand unified configuration (GUC) parameter is added                                             | If any GUC parameter is added, update the product documentation.                                   | No   |     |
| Undisclosed interface  | Whether any function, view, or system table is added or modified                            | If any function, view, or system table is added or modified, update the product documentation and consider updating the permission control policy.    | No   |     |
| Undisclosed interface  | Whether any SQL syntax is added                                             | If any SQL syntax is added, update the product documentation and add support for recording audit logs.                 | No   |     |
| Undisclosed interface  | Whether any internal tool is added                                            | If any internal tool is added, update the product documentation.                                  | No   |     |
| Undisclosed interface  | Whether the script contains commented-out code                                      | Commented-out code is prohibited in interpreted languages such as Shell and Python and must be deleted.      | No   |     |
| Undisclosed interface  | Whether there are hidden access methods such as commands, parameters, and ports                      | Access methods such as commands, parameters, and ports (including but not limited to those for product production, commissioning, and maintenance) that are not used during live network maintenance must be deleted (for example, by using macros).| No   |     |
| Undisclosed interface  | Whether the system has hidden backdoors                                        | No undocumented accounts may be reserved in the system. All accounts must be manageable by the system and documented accordingly.| No   |     |
| Undisclosed interface  | Whether any cracking or network sniffing tool is provided in the software (including software packages and patch packages) released to external users| 1. Do not provide any function or tool that can change user passwords, perform exhaustive password search (including malicious cracking of passwords by exploiting system or algorithm vulnerabilities), or decrypt files containing sensitive data (such as configuration files and databases containing keys) in the software (including software packages and patch packages) released to external users. 2. Third-party network sniffing tools (such as tcpdump, gdb, strace, and readelf), debugging tools (such as cpp, gcc, dexdump, and mirror), JDK development or compilation tools, and proprietary debugging tools and scripts (such as encryption/decryption scripts, debugging functions, and privilege escalation commands) used only for commissioning must not be retained in the system. If retention is required for service needs, strict access control must be enforced. The scenarios, risks, and reasons for keeping them must also be explained in the product documentation.| No   |     |
| Sensitive data protection| Whether authentication credentials are encrypted before being stored in the system              | Authentication credentials (such as passwords and private keys) must be encrypted for storage in the system.| No   |     |
| Sensitive data protection| Whether keys for encrypting sensitive data during transmission are hard-coded            | Hard-coded passwords and keys are prohibited.                                      | No   |     |
| Sensitive data protection| Whether sensitive information such as passwords or keys is recorded in plaintext                            | It is prohibited to display sensitive information (including passwords, private keys, and pre-shared keys) in plaintext in logs stored in the system, debugging information, error messages, and ps commands.| No   |     |
| Sensitive data protection| Whether passwords are displayed in plaintext in command output                                            | It is prohibited to display passwords in plaintext in command output.                                          | No   |     |
| Sensitive data protection| Whether the default passwords of third-party and open source software are used                          | It is prohibited to use the default passwords of third-party and open source software. For details, see section 1.5 in the *Security Design Guide*.| No   |     |
| Sensitive data protection| Whether passwords are stored in plaintext in configuration files                              | Passwords must not be written into configuration files in plaintext (except for scenarios where passwords must be specified during the installation, deployment, and use of the CLI tool).| No   |     |
| Sensitive data protection| Whether any insecure encryption algorithm is used                                    | It is prohibited to use proprietary or insecure encryption algorithms known in the industry. For recommended encryption algorithms, see section 6.2 in *Security Design Guide*.| No   |     |
| Sensitive data protection| Whether sensitive information such as passwords is transmitted through secure channels                        | Sensitive information must be transmitted through secure channels or encrypted before transmission on untrusted networks. For details, see chapter 10 in *Security Design Guide*.| No   |     |
| Sensitive data protection| Whether sensitive information such as passwords and keys in the memory is destroyed after use                    | Sensitive information such as passwords and keys in the memory must be cleared immediately after use                 | No   |     |
| Sensitive data protection| Whether random numbers used by cryptographic algorithms are cryptographically secure    | Random numbers used by cryptographic algorithms must be cryptographically secure. For details, see section 6.3 in *Security Design Guide*.| No   |     |
| Sensitive data protection| Whether there are insecure examples in the documentation                                  | Examples in the documentation must be secure and provide correct guidance for users. If there are potential risks in the examples, describe the risks in the documentation.| No   |     |
| Authentication        | Whether an authentication mechanism is provided                                            | An authentication mechanism must be provided and enabled by default for the new system.                          | No   |     |
| Authentication        | Whether authentication is performed on the server                                        | Authentication must be performed on the server.                              | No   |     |
| Authentication        | Whether the server returns valid information upon an authentication failure                            | Upon an authentication failure, the server must return detailed information of the failure to help the user locate the cause of the failure.| No   |     |
| External parameter verification| Whether the validity of external input is verified                                | 1. The use of external input data as parameters such as loop termination conditions, array indices, and memory allocation sizes may lead to system behaviors such as infinite loops, buffer overflows, memory out-of-bounds access, and denial of service. 2. The validity of external input such as file paths must be verified to prevent injection risks.| Yes   | Yes  |
| Third-party component introduction  | Whether any new third-party component is introduced                                          | 1. New third-party components must pass security compilation options, virus and vulnerability scanning, open source snippet detection, license compliance checks, and open source component scanning. For details, see the network security requirements for version releases. 2. Ensure new third-party components originate from trusted sources.| No   |     |

#### 4.6.7.2 Sensitive Data Analysis

##### 1. Sensitive data list

No sensitive data is involved.

##### 2. Sensitive operation check

There are no sensitive operations and no sensitive data is involved.

#### 4.6.7.3 Design Implementation

The msPTI enables the runtime API data collection capability through the msptiActivityEnable(msptiActivityKind kind) API and obtains the runtime API data through the callback API.

The enabled runtime API uses the memory transferred by the user. The user can maintain the resource specifications by consuming the memory in a normal way. When the memory is used in the critical condition, the user can control the size of the requested memory.

The msPTI data is managed in the memory in a unified manner. The disk I/O is the post-processing procedure after the user consumes the data. The consumption frequency can be considered.

## 4.7 External Interfaces

The API data is obtained by reusing the API capability: msptiActivityEnable(msptiActivityKind kind)

# 5. (Optional) Data structure design

No structure change is involved. The runtime API data and other APIs at the CANN layer use the same data format and APIs for reporting.
