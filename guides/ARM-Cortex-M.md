# Stratify OS on the ARM Cortex M

Stratify OS was designed specifically to take advantage of the powerful hardware features introduced with the ARM Cortex M architecture.

- Memory Protection Unit (MPU)
- Thread Mode/Handler Mode
- Service Call (SVC instruction)
- SysTick
- PendSV
- Nested Vectored Interrupt Controller (NVIC)
- Floating Point Unit (FPU)
- Cortex M7 Cache

## Thread and Handler Mode with the MPU

The Cortex M has two execution modes: thread and handler. ISR's alway execute in handler mode while non-ISR's can be configured for thread or handler mode. The MPU allows different permissions to the same memory based on the execution mode. In Stratify OS, applications--[which are built independent of the kernel](Overview.md)--run in thread mode while specified kernel code and device drivers run in handler mode. This prevents applications from accessing sensitive code or data.

The general MPU access for applications is organized as follows:

- Application Code: read and execute
- Application Data: read and write
- Thread Stack Guard: no access
- Kernel User Code: read and execute
- Kernel Root Code: no direct access
- Kernel System Data: read-only access
- Kernel Shared Data: read-write access

**The Best Part.** There is no special action to take to use the MPU in Stratify OS. You just write your applications and the MPU and memory management is all handled by Stratify OS.

## SVC

The SVC instruction is the application's gateway to handler mode.  Stratify OS creates an API where the application can pass a function pointer and an argument.

```c
void cortexm_svcall(void (*function)(void*), void* argument);
```

The SVC handler will execute the function with its argument if it passes security checks. If the application is running in thread mode, the function must be a part of the kernel root code and cortexm_svcall() must have been called from kernel user code. If the application is in root mode, the security checks are bypassed.

## PendSV and SysTick

The PendSV and SysTick interrupts are used for context switching in Stratify OS. When PendSV is triggered, it causes an immediate context switch. This is used in FIFO-style scheduling or with `sched_yield()`. The SysTick interrupt triggers a timeout when the timer expires and is used for round robin scheduling.

## NVIC

The NVIC allows for nested interrupts to execute without the programmer having to deal with saving the state. The state is saved and restored by the hardware.

Stratify OS uses the NVIC for prioritized and nested exception handling. It sets all the interrupt priorities automatically and provides a mechanism to device driver developers for elevating the priority of certain interrupts in order to meet hard real-time requirements.

The WDT and fault handlers always have the highest interrupt priority in Stratify OS. This is so that the WDT and fault handlers can reset the system if any buggy device drivers cause problems.

## FPU

The FPU is part of the Cortex M4F (single precision) and Cortex M7 (single or single/double precision) architectures. Stratify OS integrates all FPU register handling into the context switching mechanisms. Developers can simply use floating point values as they normally would in any program.

To take full advantage of the FPU, keep in mind to use `float` types on the M4F and M7's with only single precision.  For example:

```c
float x = 1.0; //Bad: 1.0 is a double that needs to be converted to float
float y = 2.0f; //Good: 2.0 is a float that can directly use the single precision FPU

x = sin(0.5); //Bad: this will use double
y = sinf(0.5f); //Good: this will use float
```

## Cortex M7 Cache (the Best for Last)

Because Stratify OS builds the kernel independent of the applications, it enables developers to take full advantage of the L1 cache introduced as part of the Cortex M7 architecture. The cache enables the chip to execute code from external memories at performance levels that are close to internal flash and SRAM speeds.

With Stratify OS, you can install applications in external memory and execute them at near optimal performance. The cache combined with Stratify OS's application based approach allows your firmware to easily scale with the complexity of both the hardware and product requirements.


