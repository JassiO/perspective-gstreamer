# perspective-gstreamer

- opencv_calib_tool

	g++ -ggdb `pkg-config --cflags opencv` -o `basename opencv_calib_tool.cpp .cpp` opencv_calib_tool.cpp `pkg-config --libs opencv`
