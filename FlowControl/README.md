Flow Control implementation using IPC, THREADS, BinSEMAPHORE.

To build
```bash
cd build
rm *
mkdir fifos
cmake ../
make
```

To run the programs.
Open three seperate terminal windows.
```bash
#receivercontroller should be run first. Rest two any ordering.
./receivercontroller
./sendercontroller
./channel
```

Thanks.
