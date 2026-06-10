# msPTI Security Statement

## System Security Hardening

You are advised to enable the `address space layout randomization` (ASLR) (level 2) in the system. Run the following command to enable it:

    echo 2 > /proc/sys/kernel/randomize_va_space

## User Account Recommendations

 1. Use the principle of least privilege. For example, prevent other users from writing data by disabling permissions like `666` and `777`.

 2. Before installing or using the tools, ensure the user's `umask` is set to `0027` or a more restrictive value. Otherwise, issues may occur, such as source code compilation failures, configuration file read failures, or excessive permissions for generated directories and files.

 3. The tools in this code repository are designed to be installed and used with low permissions. For security and least privilege purposes, all tools should not be operated by high-privilege accounts such as the root account. You are advised to install and execute the tools as a common user.

 4. If a tool depends on CANN, install the CANN package under the same non-privileged user. After running the `source` command, do not modify the environment variables in `set_env.sh`.

## File Permission Control

 1. Ensure that the directory permission is `750`.

 2. When providing an input file to the tool, you are advised to ensure that the owner of the file is the same as the owner of the tool process, and the file permission cannot be modified by others (including the group and others). By default, tool files written to the drive are not writable by others. You can manually control the permissions for the generated files as needed.

 3. Proper permission control is essential during installation and use. For details, see the following table.

**File Permission Reference**

| Type                              | Maximum Linux Permission|
| ---------------------------------- | ------------------- |
| Home directory                        | 750 (rwxr-x---)   |
| Program files (including scripts and library files)    | 550 (r-xr-x---)   |
| Program file directory                      | 550 (r-xr-x---)   |
| Configuration files                          | 640 (rw-r-----)   |
| Configuration file directory                      | 750 (rwxr-x---)   |
| Log files (recorded or archived)    | 440 (r--r-----)   |
| Log files (being recorded)                | 640 (rw-r-----)   |
| Log file directory                      | 750 (rwxr-x---)   |
| Debug files                         | 640 (rw-r-----)   |
| Debug file directory                     | 750 (rwxr-x---)   |
| Temporary file directory                      | 750 (rwxr-x---)   |
| Maintenance and upgrade file directory                  | 770 (rwxrwx---)   |
| Service data files                      | 640 (rw-r-----)   |
| Service data file directory                  | 750 (rwxr-x---)   |
| Key components, private keys, certificates, and ciphertext file directory| 700 (rwx------)   |
| Key components, private keys, certificates, and ciphertext files    | 600 (rw-------)   |
| APIs and scripts for encryption and decryption            | 500 (r-x------)   |

## Vulnerability Security Statement

The MindStudio community attaches great importance to the security of the community edition and has appointed a vulnerability management specialist to handle vulnerability-related issues. In addition, to build a more secure AI full-process toolchain, we welcome your participation.

### Vulnerability Handling Process

For each security vulnerability, the MindStudio community will assign personnel to track and handle the vulnerability. The following figure shows the end-to-end process of vulnerability handling.

   ![Vulnerability Handling Process](./figures/vulnerability_handling_process.png)

The following sections explain the vulnerability reporting, assessment, and disclosure processes.

### Vulnerability Reporting

You can contact the MindStudio community team by submitting an issue. We will immediately assign a dedicated security vulnerability specialist to contact you.
Note that to ensure security, do not include specific information about security privacy in the issue.

**Response to Reports**

1. The MindStudio community will confirm, analyze, and report the security vulnerability within three working days, and start the security handling process.
2. After confirming the security vulnerability, the MindStudio security team will distribute and follow up the issue.
3. During the process of classifying, confirming, and fixing security vulnerabilities, as well as releasing patches, we will provide timely updates on the report.

### Vulnerability Assessment

The CVSS standard is widely used in the industry to assess the severity of vulnerabilities. When using CVSS v3.1 to assess vulnerabilities, MindStudio needs to set the vulnerability attack scenario and assess the vulnerability based on the actual impact in the attack scenario. Vulnerability severity assessment involves assessing the difficulty of exploitation as well as the impact on confidentiality, integrity, and availability after exploitation, resulting in a numerical score.

#### Vulnerability Assessment Metrics

MindStudio uses the following vectors to assess the severity of a vulnerability:

- Attack vector (AV): indicates the "remoteness" of an attack and how a vulnerability can be exploited.
- Attack complexity (AC): describes the difficulty of executing an attack and the factors required for a successful attack.
- User interaction (UI): determines whether the attack requires user participation.
- Privileges required (PR): records the level of user authentication required for a successful attack.
- Scope (S): determines whether an attack can affect components with different permission levels.
- Confidentiality (C): measures the impact resulting from information disclosure to unauthorized parties.
- Integrity (I): measures the impact resulting from information tampering.
- Availability (A): measures the impact on users' access to data or services when needed.

#### Assessment Principles

- Assess the severity level of a vulnerability, not the risk.
- The assessment must be based on an attack scenario where a successful attack can compromise the confidentiality, integrity, and availability of the system.
- When a security vulnerability has multiple attack scenarios, the attack scenario with the highest CVSS score (that is, with the greatest impact) shall prevail in the assessment.
- If a vulnerability exists in an embedded or invoked library, perform the assessment after determining the attack scenario based on how the library is used in the product.
- The CVSS score is 0 if the security defect cannot be triggered or does not affect CIA (confidentiality, integrity, and availability).

#### Assessment Procedure

To assess the severity level of a vulnerability, perform the following steps:

1. Set a possible attack scenario and score based on this attack scenario.
2. Identify the vulnerable component and affected components.
3. Select values for base metrics.

   - Select values for the exploitability metrics (attack vector, attack complexity, privileges required, user interaction, and scope) based on the vulnerable component.

   - Ensure impact metrics (confidentiality, integrity, and availability) reflect the impact on either the vulnerable component or the affected components, whichever is more severe.

#### Severity Rating

| **Severity Rating** | **CVSS Score** | **Vulnerability Fix Time**|
| ------------------------------- | --------------------- | ---------------- |
| Critical               | 9.0~10.0              | 7 days             |
| High                     | 7.0~8.9               | 14 days            |
| Medium                   | 4.0~6.9               | 30 days            |
| Low                      | 0.1~3.9               | 30 days            |

### Vulnerability Disclosure

After a security vulnerability is fixed, the MindStudio community will release a security advisory (SA) and security notice (SN). The security advisory includes the technical details, type, reporter, CVE ID, affected versions, and fixed versions of the vulnerability.
To protect the security of MindStudio users, the MindStudio community will not disclose, discuss, or confirm the security issues of MindStudio products before the investigation, fixing, and release of the security advisory.

### Appendixes

#### MindStudio Security Advisory (SA)

Currently maintained versions have no security vulnerabilities.

#### MindStudio Security Notice (SN)

Vulnerability descriptions for third-party open-source components:

| CVE ID| Third-Party Component Name| Affected MindStudio Tool/Plugin| Status| Description|
| ------- | ------------ | --------------------------- | ---- | ---- |
|         |              |                             |      |      |

## Data Security Statement

Loading and saving data during the use of the tools may involve data risks.

## Build Security Statement

msPTI supports source code compilation and installation. During compilation, the third-party library is downloaded and the build shell script is executed. Temporary program files and compilation directories are generated during the compilation. To reduce security risks, you can perform permission control on files within the source code directory. During the build process, you can modify build scripts as needed to avoid security risks and ensure the security of the build results.

## Running Security Statement

If an exception occurs during operation, the tool will exit the process and print error messages. This is expected behavior. You are advised to locate the specific cause of the error based on the error prompts, such as by viewing log files or result files generated during the collection and parsing process.

## Public API Statement

The msPTI project is developed using C++ and Python. All the provided external interfaces are disclosed in the documentation. Only the Python interface is provided as the formal interface. The dynamic library does not directly provide services. The exposed interfaces are for internal use only. You are advised not to use them.

For scripting languages such as Python where source code is released, use the public APIs specified in the documentation. Do not directly call source code APIs that are not explicitly disclosed.

## Usage of Safe Functions

While unsafe functions are not forcibly disabled, you are advised to use their safe variants that explicitly take a buffer length parameter, for example, `memset_s` and `memcpy_s`.
