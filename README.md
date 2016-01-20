# DGMA_P2P
Samples showing FirePro DirectGMA features in OpenGL, DX11 and OpenCL

----------------
System Setup
----------------

Your system needs 2 AMD GPUs with the DirectGMA feature, and the latest GPU driver.
Plug at least 1 monitor on each GPU.

-----------------
Enable DirectGMA
-----------------

Before running your DirectGMA samples, you have to make sure that the DirectGMA feature is enabled:

on Windows: 
AMD Firepro Control Center -> AMD Firepro -> SDI/DirectGMA -> Check the checkbox of the 2 GPU -> Apply -> Restart your computer.

on Linux:
in your terminal, enter the commands :
>  aticonfig --initial=dual-head --adapter=all -f
>  aticonfig --set-pcs-val=MCIL,DMAOGLExtensionApertureMB,96
>  aticonfig --set-pcs-u32=KERNEL,InitialPhysicalUswcUsageSize,96
and reboot.


-------------------------
Building the source code
-------------------------

For Visual Studio users, each sample has its Visual Studio 2010 solution file, it will also work on more recent version of visual studio:
- GPUtoGPU_OpenCL\GPUtoGPU_OpenCL.sln
- GPUtoGPU_OpenGL\GPUtoGPU_OpenGL.sln
- GPUtoGPU_DX11\GPUtoGPU_DX11.sln 

For Linux users, each sample has its Makefile:
- GPUtoGPU_OpenCL\GPUtoGPU_OpenCL\Makefile
- GPUtoGPU_OpenGL\GPUtoGPU_OpenGL\Makefile


Before building the GPUtoGPU_OpenCL project, install the AMD APP SDK,
The include and libraries directories point to this SDK using AMDAPPSDKROOT environment variable. This variable will be set when installing the SDK


Before building the GPUtoGPU_DX11, install the Microsoft DirectX SDK (June 2010),
and check that your include&lib directories are correct in your Visual Studio solution.
The ones used in this SDK are: 
C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include  and  C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86
