# springbok-cli

The `springbok-cli` application provides a command-line option for accessing SPRINGBOK Core RPCs.

## Usage

> 🚧 Multiple wallet support
>
> Since you may have multiple wallets loaded at the same time, wallet-related RPCs require using the `-rpcwallet` option any time more than one wallet file is loaded. This is to ensure the RPC command is executed using the correct wallet. The syntax is:  
> `springbok-cli -rpcwallet=<wallet-name> <command>`

```bash Send command
springbok-cli [options] <command> [params]
```

```bash Send command using wallet
springbok-cli [options] -rpcwallet=<wallet-name> <command> [params]
```

```bash Send command (with named arguments)
  springbok-cli [options] -named <command> [name=value] ... 
```

```bash List commands
springbok-cli [options] help
```

```bash Get help for command
springbok-cli [options] help <command>
```

> 📘 RPC Details
>
> View [the list of RPCs](../api/remote-procedure-call-quick-reference.md) for more detailed information. Using springbok-cli, this information is available using the `springbok-cli [options] help` and `springbok-cli [options] help <command>` commands.

### Options

```text
  -?
       Print this help message and exit

  -conf=<file>
       Specify configuration file. Relative paths will be prefixed by datadir
       location. (default: SPRINGBOK.conf)

  -datadir=<dir>
       Specify data directory

  -generate
       Generate blocks immediately, equivalent to RPC getnewaddress followed by
       RPC generatetoaddress. Optional positional integer arguments are
       number of blocks to generate (default: 1) and maximum iterations
       to try (default: 1000000), equivalent to RPC generatetoaddress
       nblocks and maxtries arguments. Example: springbok-cli -generate 4
       1000

  -getinfo
       Get general information from the remote server. Note that unlike
       server-side RPC calls, the results of -getinfo is the result of
       multiple non-atomic requests. Some entries in the result may
       represent results from different states (e.g. wallet balance may
       be as of a different block from the chain state reported)

  -named
       Pass named instead of positional arguments (default: false)

  -netinfo
       Get network peer connection information from the remote server. An
       optional integer argument from 0 to 4 can be passed for different
       peers listings (default: 0). Pass "help" for detailed help
       documentation.

  -rpcclienttimeout=<n>
       Timeout in seconds during HTTP requests, or 0 for no timeout. (default:
       900)

  -rpcconnect=<ip>
       Send commands to node running on <ip> (default: 127.0.0.1)

  -rpccookiefile=<loc>
       Location of the auth cookie. Relative paths will be prefixed by a
       net-specific datadir location. (default: data dir)

  -rpcpassword=<pw>
       Password for JSON-RPC connections

  -rpcport=<port>
       Connect to JSON-RPC on <port> (default: 9998, testnet: 19998, regtest:
       19898)

  -rpcuser=<user>
       Username for JSON-RPC connections

  -rpcwait
       Wait for RPC server to start

  -rpcwallet=<walletname>
       Send RPC for non-default wallet on RPC server (needs to exactly match
       corresponding -wallet option passed to SPRINGBOKd). This changes the
       RPC endpoint used, e.g. http://127.0.0.1:9998/wallet/<walletname>

  -stdin
       Read extra arguments from standard input, one per line until EOF/Ctrl-D
       (recommended for sensitive information such as passphrases). When
       combined with -stdinrpcpass, the first line from standard input
       is used for the RPC password.

  -stdinrpcpass
       Read RPC password from standard input as a single line. When combined
       with -stdin, the first line from standard input is used for the
       RPC password. When combined with -stdinwalletpassphrase,
       -stdinrpcpass consumes the first line, and -stdinwalletpassphrase
       consumes the second.

  -stdinwalletpassphrase
       Read wallet passphrase from standard input as a single line. When
       combined with -stdin, the first line from standard input is used
       for the wallet passphrase.

  -version
       Print version and exit
```

### Chain selection options

```text
  -chain=<chain>
       Use the chain <chain> (default: main). Allowed values: main, test,
       regtest

  -devnet=<name>
       Use devnet chain with provided name

  -highsubsidyblocks=<n>
       The number of blocks with a higher than normal subsidy to mine at the
       start of a chain. Block after that height will have fixed subsidy
       base. (default: 0, devnet-only)

  -highsubsidyfactor=<n>
       The factor to multiply the normal block subsidy by while in the
       highsubsidyblocks window of a chain (default: 1, devnet-only)

  -llmqchainlocks=<quorum name>
       Override the default LLMQ type used for ChainLocks. Allows using
       ChainLocks with smaller LLMQs. (default: llmq_devnet,
       devnet-only)

  -llmqdevnetparams=<size>:<threshold>
       Override the default LLMQ size for the LLMQ_DEVNET quorum (default: 3:2,
       devnet-only)

  -llmqinstantsenddip0024=<quorum name>
       Override the default LLMQ type used for InstantSendDIP0024. (default:
       llmq_devnet_dip0024, devnet-only)

  -llmqmnhf=<quorum name>
       Override the default LLMQ type used for EHF. (default: llmq_devnet,
       devnet-only)

  -llmqplatform=<quorum name>
       Override the default LLMQ type used for Platform. (default:
       llmq_devnet_platform, devnet-only)

  -minimumdifficultyblocks=<n>
       The number of blocks that can be mined with the minimum difficulty at
       the start of a chain (default: 0, devnet-only)

  -powtargetspacing=<n>
       Override the default PowTargetSpacing value in seconds (default: 2.5
       minutes, devnet-only)

  -testnet
       Use the test chain. Equivalent to -chain=test
```
