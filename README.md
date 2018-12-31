This is working proof of concept for the distributed convolutional neural network. The project based on [YOLO v2](https://pjreddie.com/darknet/yolov2/). 

You need Windows and Visual Studio to compile it.

# How run nNet

1. Clone this repo.
2. Download Yolo2 weights from [pjreddie.com](https://pjreddie.com/media/files/yolov2.weights) to `data` folder.
3. Build `nnet/nnet.sln` and `nnet.mjpeg/nNet.mjpeg.sln` in **release mode**.
4. Run two workers: 
`run_distributed_worker_1_yolov2.cmd` 
`run_distributed_worker_2_yolov2.cmd`
5. Run a detector: 
`run_distributed_detector_yolov2.cmd`
6. Run a client: 
`run_mjpge.cmd`
7. Put a URL to working MJPG stream (e.g. `http://50.73.9.194:80/mjpg/video.mjpg`) and press **Start**.

# How it works?

Soon.

## License

nNet is open-sourced software licensed under the [MIT license](http://opensource.org/licenses/MIT).