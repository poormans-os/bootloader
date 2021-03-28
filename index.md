<h1 align="center"> PMOS </h1>

PMOS (poor mans operating-system) Is A Simple UEFI Based Operating System, Using [EDK2](https://github.com/tianocore/edk2), Written In C By Two Students.

## What Can It Do?

- Basic Input/Output
- Multi-Threading Using SMP
- Run [BrainF\*ck](https://en.wikipedia.org/wiki/Brainfuck) Code

## How To Run

**Note**: You Need To Have At Least Dual Core CPU

1. First You Need To Clone The Code

   ```shell
   $ git clone https://github.com/poormans-os/bootloader --recurse-submodules
   ```

2. Configure Your System

   ### windows

   - You need to have at least build version [_19645.1_](https://www.cnet.com/how-to/change-to-the-fast-ring-for-more-frequent-windows-10-preview-updates/)
   - [_Qemu_](https://www.qemu.org/download/#windows)
   - [_Xserver_](https://sourceforge.net/projects/vcxsrv/)
     - Make Sure The Server Is Running
   - [_WSL2_](https://docs.microsoft.com/en-us/windows/wsl/install-win10) (with the requirements below)

   ### wsl

   - [**KVM enabled**](https://boxofcables.dev/accelerated-kvm-guests-on-wsl-2/)
   - Dependencies [clang-9, nasm, lld-link, make]

   ### linux (Not Tested, Should Work)

   - The same requirements as wsl
   - [_Qemu_](https://www.qemu.org/download/#linux)

3. Run Using Make

   Run BrainF\*ck Code From The File `/bin/main.bf`, If Not Present Run As An Interpreter

   ```shell
   $ sudo make run
   ```

   Runs A Simple MultiThreaded Test

   ```shell
   $ sudo make run-smp
   ```

   Runs A Simple Echo-Like Program

   ```shell
   $ sudo make run-io
   ```
