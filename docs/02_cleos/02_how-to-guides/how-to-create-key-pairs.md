<<<<<<< HEAD
## Goal
Create a keypair consisting of a public and a private key for signing transactions in the EOSIO blockchain.
=======
## Overview

This how-to guide provides instructions on how to create a keypair consisting of a public key and a private key for signing transactions in an EOSIO blockchain.
>>>>>>> 1926c9588e98187e7666c14ab94ac800a6ca84d7

## Before you begin
Before you follow the steps to create a new key pair, make sure the following items are fulfilled:


Make sure you meet the following requirements:
* Install the currently supported version of `cleos`
[[info | Note]]
| The cleos tool is bundled with the EOSIO software. [Installing EOSIO](../../00_install/index.md) will also install the cleos tool. 
* Learn about asymmetric cryptography (public and private keypair) in the context of an EOSIO blockchain.

## Command Reference

See the following reference guide for `cleos` command line usage and related options:
* [`cleos create key`](../03_command-reference/create/key.md) command and its parameters

<<<<<<< HEAD
[[info | Note]]
| The cleos tool is bundled with the EOSIO software. [Installing EOSIO](../../00_install/index.md) will also install the cleos tool. 

* Acquire functional understanding of asymmetric cryptography (public and private keypair) in the context of blockchain
=======
## Procedure
>>>>>>> 1926c9588e98187e7666c14ab94ac800a6ca84d7

The following steps show how to create a public/private keypair, display them on the console, and save them to a file:

<<<<<<< HEAD
To create a keypair and print the result to the console:

```sh
cleos create key --to-console
```

**Example Output**
=======
1. Create a public/private keypair and print them to the console:
>>>>>>> 1926c9588e98187e7666c14ab94ac800a6ca84d7

```sh
cleos create key --to-console
Private key: 5KPzrqNMJdr6AX6abKg*******************************cH
Public key: EOS4wSiQ2jbYGrqiiKCm8oWR88NYoqnmK4nNL1RCtSQeSFkGtqsNc
```

<<<<<<< HEAD

To create a keypair and save it to a file:

```sh
cleos create key --file FILE_TO_SAVEKEY
```
Where: FILE_TO_SAVEKEY = name of the file

**Example Output**
```sh
cleos create key --file pw.txt         
saving keys to pw.txt
```

To view the saved keypair in the file:
```sh
cat pw.txt
Private key: 5K7************************************************
Public key: EOS71k3WdpLDeqeyqVRAAxwpz6TqXwDo9Brik5dQhdvvpeTKdNT59
```
=======
**Where**:

* `--to-console` = The option parameter to print the keypair to the console

**Example Output**

```console
Private key: 5KPzrqNMJdr6AX6abKg*******************************cH
Public key: EOS4wSiQ2jbYGrqiiKCm8oWR88NYoqnmK4nNL1RCtSQeSFkGtqsNc
```

2. Create a public/private keypair and save it to a file:

```sh
cleos create key --file pw.txt
```
**Where**: 

* `--file` = The option parameter to save the keypair to a file
* `FILE_TO_SAVEKEY` = The name of the file to save the keypair

**Example Output**

```console
saving keys to pw.txt
```

To view the saved keypair stored in the file:

```sh
cat pw.txt
```
```console
Private key: 5K7************************************************
Public key: EOS71k3WdpLDeqeyqVRAAxwpz6TqXwDo9Brik5dQhdvvpeTKdNT59
```

## Summary

By following these instructions, you are able to create public/private keypairs, print them to the console, and save them to a file. 
>>>>>>> 1926c9588e98187e7666c14ab94ac800a6ca84d7
