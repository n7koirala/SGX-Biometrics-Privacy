# **Privacy-Preserving Record Linkage with Intel SGX via Gramine**

## Project Structure: 
```
│── /src/                # Source files
│   ├── main.cpp         # Main program 
│   ├── aes_crypt.cpp    # AES encryption & decryption implementation
│   ├── encrypt_receiver.cpp  # Encrypts receiver query before main runs
│   ├── create_sender_db.cpp  # Creates sender dataset
│── /include/            # Header files
│   ├── aes_crypt.h      # AES function declarations
│   ├── file_paths.h 
│── /aes_keys/           # Folder to store AES key & IV files
│── /scripts/            # Scripts for running experiments
│── /data/               # Results of experimental runs and charts
│── /enc_receiver_query/ # Stores encrypted receiver query file
│── /sender_db/          # Stores sender dataset          
```

This repository demonstrates a simple **record linkage** application that can run either:
- **Inside an Intel SGX Enclave** (using [Gramine](https://gramineproject.io/) in SGX mode), or
- **Outside the Enclave** in a normal Linux environment (using Gramine Direct).

---

## **Overview**

1. **`create_sender_db.cpp`**: Creates simulated “sender” database. Sender has a set of random 32-bit integer records.  
2. **`encrypt_receiver.cpp`**: Generates the AES keys and the encrypted receiver query ciphertext.  
3. **`main.cpp`**: Loads one record from the “receiver” file and checks if this record exists in the “sender” database.
4. **`ReceiverSender.manifest.template`**: A template used by Gramine to build the final manifest files.  
5. **`Makefile`**: Automates compilation and signing tasks for both SGX and non-SGX modes.
6. **`record_linkage_charts.ipynb`**: Generates charts for the data under `/data` folder.

## **Prerequisites**

1. **Intel SGX Driver/Platform Software (PSW)** installed on your system.  
2. **Gramine** installed on your system. ([Installation Guide](https://gramine.readthedocs.io/en/latest/))
3. **GCC / G++** (or a similar C++ compiler) for building the code.  
4. **AES Encryption Support*** OpenSSL and crypto libraries required for AES-based encryption.

**Optional**: If you do not have SGX hardware, you can still run in non-SGX mode (`gramine-direct`).

---

## **Step-by-Step Instructions**

### **1. Clone and Navigate to the Repository**

```
git clone https://github.com/your-username/PP_record_linkage.git
cd PP_record_linkage
```


## 2. Create Sender Database

Compile and run the program that generates the sender set and use ```-Iinclude``` flag to include the necessary files under ```include```  :

```
g++ -o ./bin/create_sender_db ./src/create_sender_db.cpp -Iinclude -std=c++17
```

### Run to generate sender databases, e.g., 1024 records
```
./bin/create_sender_db 1024
```

## 3. Generate AES keys and Run Receiver Program

Compile and run the program to generate encrypted receiver query ciphertext and AES-CTR keys:

```
g++ -o ./bin/encrypt_receiver ./src/encrypt_receiver.cpp ./src/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17
```

### Run to generate the query ciphertext
```
./bin/encrypt_receiver
```

## 4. Build and Run the Record Linkage Application

To simply make and run the plain application:

```
g++ -o ./bin/main ./src/main.cpp ./src/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17
./bin/main
```


## 4.1. Non-SGX (Gramine Direct)

Build the application (this compiles main.cpp and generates a manifest):

```
make
```

Run the application in non-SGX mode:

```
gramine-direct ./build/ReceiverSender
```

## 3.2. SGX Mode (Gramine SGX)
Clean the previous build:

```
make clean
```

Build for SGX:
```
make SGX=1
```

This creates:

```build/ReceiverSender```
```build/ReceiverSender.manifest``` (non-SGX manifest)
```build/ReceiverSender.manifest.sgx``` (SGX-specific manifest)
```build/ReceiverSender.sig``` (signature file)

Run in SGX mode:

```
gramine-sgx ./ReceiverSender
```


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


# Analysis using ```perf```

1. **Install perf**: ([Installation Guide](https://gramine.readthedocs.io/en/stable/performance.html#perf))
2. **Set flags**: Add the following in the manifest.template file: ```sgx.debug = true```
3. **Clean and make**: Clean and make again with ```DEBUG=1``` flag.

```
make clean
make SGX=1 DEBUG=1 NUM_SENDERS=8
```

4. **Run the app**

```
sudo perf record gramine-sgx ./build/ReceiverSender
```

5. **Analyze the report**

```
sudo perf report
```

