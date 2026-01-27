# Cross-Compile Client Scripts
This is meant as a temporary method to build and run the client on a Linux machine.  This is accomplished by:

1. Using a pre-configured Docker image with Mingw and all required tools to compile `KnightOnLine.exe`.  Mingw is the
only compiler with Windows Build Tools support (Windows C/C++ Headers). The `build_client.sh` script in this directory 
will handle building the image, creating and destroying a temporary container that will build and export `KnightOnline.exe` 
to `cmake-build-debug/bin`.
2. `run_client.sh` will setup and run `KnightOnLine.exe` using `umu-launcher`. `umu-launcher` allows for the use of Steam's
Proton (an enhanced fork of wine) without actually needing Steam installed or running.

This temporary tooling will be removed when we achieve all goals required for the client to natively build cross-platform.

# Dependencies

The following must be installed on your Linux installation to leverage this utility:
* Docker
* Umu-launcher

These can be installed with the following commands, based on popular distros.

**apt (Debian, Ubuntu, others)**<br/>
> sudo apt install docker umu-launcher<br/>

**pacman (Arch)**<br/>
> sudo pacman -S docker umu-launcher<br/>

**yay (Arch AUR)**<br/>
> sudo yay -S docker umu-launcher<br/>

# Limitations / Drawbacks
This is cross-compiling the client inside of a docker container, copying the executable out to the `cmake-build-debug/bin`
directory in `build_client.sh`, then running the executable using an enhanced version of wine through `run_client.sh`. 

The biggest drawback: As of this moment I have not looked into how to attach a debugger to this process.  The debugger would likely need to be run 
within the same wine prefix to have access to the running/compiled executable.

While the image should only need to be created on the first run, the container built from the image to compile the client 
is destroyed with every run of the `build_client.sh` script.  This ensures a clean build every time, but it also means that 
every dependency must be fetched and every file must be compiled every run.  It takes about a minute to execute a build on 
my machine like this.

For these reasons, you still may want to bite the bullet on using a Windows VM for Client development.  If you're more focused 
on Server development and don't want to deal with Windows, this should get you by.