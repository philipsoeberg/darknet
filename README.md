Note: This is Philip Soeberg changes:

This branch is for NVIDIA ION (which is my ASROCK 330 box..)

ION need CUDA6.5 which requires GCC 4 series.
Install CentOS 6.8 with devel tools, then download cuda6.5 from

nvidia.com https://developer.nvidia.com/cuda-toolkit-65
THe cuda6.5 .run installer includes the 340 series nvidia driver.

No need for X or other graphical stuff.. It's just consuming vmem.

ION can only run yolov2-tiny.cfg as it only has 512MB vmem.
Get the wheights from: wget https://pjreddie.com/media/files/yolov2-tiny.weights

I use this project for my CCTV/ip-cams to identify people at my property through tight integration with Node-red.


-----------


![Darknet Logo](http://pjreddie.com/media/files/darknet-black-small.png)

# Darknet #
Darknet is an open source neural network framework written in C and CUDA. It is fast, easy to install, and supports CPU and GPU computation.

For more information see the [Darknet project website](http://pjreddie.com/darknet).

For questions or issues please use the [Google Group](https://groups.google.com/forum/#!forum/darknet).
