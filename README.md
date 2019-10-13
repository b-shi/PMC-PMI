# PMC-PMI

PMC-PMI is a linux kernel module which uses performance monitoring interrupts(PMI) to record performance counter events each cpu cycle. This is one method to observe some of the microarchitectural behaviours of the underlying hardware. The idea was inspired by the wonderfully written blog post https://gamozolabs.github.io/metrology/2019/08/19/sushi_roll.html. For a summary of the main ideas please read the the blog post.  

**DISCLAIMER**: This kernel module has had very limited testing and may contain bugs the author is not aware of. You are proceeding at your own risk when running it.

## Initial Configuration
Before running the kernel it is recommended to do the following:
* Disable Hyper-Threading in BIOS
* Enable X2APIC in BIOS
* Using the kernel boot parameter "isolcpus" to isolate cpus you plan to run module on

## Installing / Building / Getting started

To build the kernel module run

```shell
git clone https://github.com/bsghost/PMC-PMI.git
cd PMC-PMI && make
```


Here you should say what actually happens when you execute the code above.

## Features

What's all the bells and whistles this project can perform?
* What's the main functionality
* You can also do another thing
* If you get really randy, you can even do this

## Configuration

Here you should write what are all of the configurations a user can enter when
using the project.

## Links

## Licensing
