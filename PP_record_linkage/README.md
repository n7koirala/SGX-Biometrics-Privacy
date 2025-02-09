# **Privacy-Preserving Record Linkage with Intel SGX via Gramine**

This repository demonstrates a simple **record linkage** application that can run either:
- **Inside an Intel SGX Enclave** (using [Gramine](https://gramineproject.io/) in SGX mode), or
- **Outside the Enclave** in a normal Linux environment (using Gramine Direct).

---

## **Overview**

1. **`create_sender_db.cpp`**: Creates simulated “sender” databases. Each sender has a set of random integer records.  
2. **`main.cpp`**: Loads one record from the “receiver” file and checks if this record exists in any of the “sender” databases.
3. **`ReceiverSender.manifest.template`**: A template used by Gramine to build the final manifest files.  
4. **`Makefile`**: Automates compilation and signing tasks for both SGX and non-SGX modes.

## **Prerequisites**

1. **Intel SGX Driver/Platform Software (PSW)** installed on your system.  
2. **Gramine** installed on your system. ([Installation Guide](https://gramine.readthedocs.io/en/latest/))
3. **GCC / G++** (or a similar C++ compiler) for building the code.  

**Optional**: If you do not have SGX hardware, you can still run in non-SGX mode (`gramine-direct`).

---

## **Step-by-Step Instructions**

### **1. Clone and Navigate to the Repository**

```
git clone https://github.com/your-username/PP_record_linkage.git
cd PP_record_linkage
```


## 2. Create Sender Databases

Compile and run the script that generates the sender sets:

```
g++ create_sender_db.cpp -o create_sender_db
```

# Run to generate sender databases, e.g., 8 senders
```
./create_sender_db 8
```


## 3. Build and Run the Record Linkage Application
## 3.1. Non-SGX (Gramine Direct)

Build the application (this compiles main.cpp and generates a manifest):

```
make NUM_SENDERS=8
```

Run the application in non-SGX mode:

```
gramine-direct ./build/ReceiverSender
```

Output: The application will print whether the “receiver” value is found in any of the sender sets:

## 3.2. SGX Mode (Gramine SGX)
Clean the previous build:

```
make clean
```

Build for SGX:
```
make SGX=1 NUM_SENDERS=8
```

This creates:

```build/ReceiverSender```
```build/ReceiverSender.manifest``` (non-SGX manifest)
```build/ReceiverSender.manifest.sgx``` (SGX-specific manifest)
```build/ReceiverSender.sig``` (signature file)

Run in SGX mode:

```
gramine-sgx ./build/ReceiverSender
```

Output: Similar to the non-SGX mode, it will print whether the receiver’s record exists in any sender database.


# Manifest File Explanation

```ReceiverSender.manifest.template```: A template that Gramine uses to generate the final ```.manifest``` and ```.manifest.sgx```.

It includes paths to the application binary and libraries it depends on.
The Makefile automatically replaces variables like ```{{ log_level }}``` and ```{{ num_senders }}``` with values provided during build.



# Setting Number of Senders

Use ```NUM_SENDERS=<N>``` with make to specify how many sender sets the application should look for:

```
make NUM_SENDERS=8
```

## Debug Logging
If you need more verbose logs, build with:

```
make DEBUG=1
```

Then, in SGX mode:

```
make SGX=1 DEBUG=1
```

## Clean Build

```
make clean
```
This removes all generated files, including manifests, signature files, and the build directory.

# Manifest Checks

Verify non-SGX manifest:

```
gramine-manifest-check build/ReceiverSender.manifest
```
## Verify SGX manifest:

```
gramine-manifest-check build/ReceiverSender.manifest.sgx
```
